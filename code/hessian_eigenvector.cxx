#include <common.h>
#include <utility.h>
#include "itkSymmetricSecondRankTensor.h"
#include "itkSymmetricEigenAnalysis.h"
#include <itkHessianRecursiveGaussianImageFilter.h>
#include "itkFixedArray.h"

typedef float EigenValueType;
typedef itk::FixedArray< EigenValueType, 3 > EigenValueArrayType;
typedef itk::Matrix<float, 3,3> EigenMatrixType;

bool AbsLessEqualCompare(EigenValueType a, EigenValueType b);

int hessian_eigenvector(HessianImageType::Pointer hessianPtr,
			ImageType3F::Pointer vesselnessPtr,
			ImageTypeArray3F::Pointer eigenvectorPtr,
			HessianPar par)
{
     double m_Alpha = par.alpha;
     double m_Beta = par.beta;
     double m_Gamma = par.gamma;
     unsigned m_ObjectDimension = 1;
     bool m_BrightObject = par.bright_object;
     unsigned ImageDimension = 3;
     bool m_ScaleObjectnessMeasure = par.scale_objectness;
     
     typedef itk::SymmetricEigenAnalysis< HessianImageType::PixelType, EigenValueArrayType, EigenMatrixType  > CalculatorType;
     CalculatorType eigenCalculator(3);
     eigenCalculator.SetOrderEigenMagnitudes(true);

     itk::ImageRegionConstIterator< HessianImageType > it(hessianPtr, hessianPtr->GetLargestPossibleRegion());     
     itk::ImageRegionIterator< ImageType3F > oit(vesselnessPtr, vesselnessPtr->GetLargestPossibleRegion());
     itk::ImageRegionIterator< ImageTypeArray3F > evit(eigenvectorPtr, eigenvectorPtr->GetLargestPossibleRegion());

     oit.GoToBegin();
     it.GoToBegin();
     evit.GoToBegin();

     itk::FixedArray<float, 3> prin_ev; // eigenvector with smallest eigenvalue magnitude.

     HessianImageType::IndexType hessianIdx;
     while ( !it.IsAtEnd() )
     {
     	  // compute eigen values
     	  EigenValueArrayType eigenValues;
	  EigenMatrixType eigenVectors;
     	  eigenCalculator.ComputeEigenValuesAndVectors(it.Get(), eigenValues, eigenVectors);

     	  // Sort the eigenvalues by magnitude but retain their sign.
     	  // The eigenvalues are to be sorted |e1|<=|e2|<=...<=|eN|
     	  EigenValueArrayType sortedEigenValues = eigenValues;
     	  // std::sort( sortedEigenValues.Begin(), sortedEigenValues.End(), AbsLessEqualCompare );

     	  // check whether eigenvalues have the right sign
     	  bool signConstraintsSatisfied = true;
     	  for ( unsigned int i = m_ObjectDimension; i < ImageDimension; i++ )
     	  {
     	       if ( ( m_BrightObject && sortedEigenValues[i] > 0.0 )
     		    || ( !m_BrightObject && sortedEigenValues[i] < 0.0 ) )
     	       {
     		    signConstraintsSatisfied = false;
     		    break;
     	       }
     	  }

     	  if ( !signConstraintsSatisfied )
     	  {
     	       oit.Set(0);
     	       ++it;
     	       ++oit;
	       ++evit;
     	       continue;
     	  }

     	  EigenValueArrayType sortedAbsEigenValues;
     	  for ( unsigned int i = 0; i < ImageDimension; i++ )
     	  {
     	       sortedAbsEigenValues[i] = vnl_math_abs(sortedEigenValues[i]);
     	  }

     	  // initialize the objectness measure
     	  double objectnessMeasure = 1.0;

     	  // compute objectness from eigenvalue ratios and second-order structureness
     	  if ( m_ObjectDimension < ImageDimension - 1 )
     	  {
     	       double rA = sortedAbsEigenValues[m_ObjectDimension];
     	       double rADenominatorBase = 1.0;
     	       for ( unsigned int j = m_ObjectDimension + 1; j < ImageDimension; j++ )
     	       {
     		    rADenominatorBase *= sortedAbsEigenValues[j];
     	       }
     	       if ( vcl_fabs(rADenominatorBase) > 0.0 )
     	       {
     		    if ( vcl_fabs(m_Alpha) > 0.0 )
     		    {
     			 rA /= vcl_pow( rADenominatorBase, 1.0 / ( ImageDimension - m_ObjectDimension - 1 ) );
     			 objectnessMeasure *= 1.0 - vcl_exp( -0.5 * vnl_math_sqr(rA) / vnl_math_sqr(m_Alpha) );
     		    }
     	       }
     	       else
     	       {
     		    objectnessMeasure = 0.0;
     	       }
     	  }

     	  if ( m_ObjectDimension > 0 )
     	  {
     	       double rB = sortedAbsEigenValues[m_ObjectDimension - 1];
     	       double rBDenominatorBase = 1.0;
     	       for ( unsigned int j = m_ObjectDimension; j < ImageDimension; j++ )
     	       {
     		    rBDenominatorBase *= sortedAbsEigenValues[j];
     	       }
     	       if ( vcl_fabs(rBDenominatorBase) > 0.0 && vcl_fabs(m_Beta) > 0.0 )
     	       {
     		    rB /= vcl_pow( rBDenominatorBase, 1.0 / ( ImageDimension - m_ObjectDimension ) );

     		    objectnessMeasure *= vcl_exp( -0.5 * vnl_math_sqr(rB) / vnl_math_sqr(m_Beta) );
     	       }
     	       else
     	       {
     		    objectnessMeasure = 0.0;
     	       }
     	  }

     	  if ( vcl_fabs(m_Gamma) > 0.0 )
     	  {
     	       double frobeniusNormSquared = 0.0;
     	       for ( unsigned int i = 0; i < ImageDimension; i++ )
     	       {
     		    frobeniusNormSquared += vnl_math_sqr(sortedAbsEigenValues[i]);
     	       }
     	       objectnessMeasure *= 1.0 - vcl_exp( -0.5 * frobeniusNormSquared / vnl_math_sqr(m_Gamma) );
     	  }


     	  // in case, scale by largest absolute eigenvalue
     	  if ( m_ScaleObjectnessMeasure )
     	  {
     	       objectnessMeasure *= log(sortedAbsEigenValues[ImageDimension - 1]);
     	  }

     	  oit.Set( static_cast< float >( objectnessMeasure ) );

	  // set eigen vectors. each row of eigenVectors represents one
	  // eigenvector. We are interested in the first row (after sorting).
	  prin_ev[0] = eigenVectors[0][0];
	  prin_ev[1] = eigenVectors[0][1];
	  prin_ev[2] = eigenVectors[0][2];

	  evit.Set(prin_ev);

     	  ++it;
     	  ++oit;
	  ++evit;
     }

     return 0;
}
			
bool AbsLessEqualCompare(EigenValueType a, EigenValueType b)
{
     return vnl_math_abs(a) <= vnl_math_abs(b);
}

int multiscale_hessian(ImageType3F::Pointer intensityPtr,
		       ImageType3F::Pointer vesselnessPtr,
		       ImageType3F::Pointer scalePtr,
		       ImageTypeArray3F::Pointer eigenvectorPtr,
		       ImageType3UC::Pointer maskPtr,
		       HessianPar par)
{
     bool m_NonNegativeHessianBasedMeasure = true;
     unsigned m_NumberOfSigmaSteps = par.steps;

     // define buffer for eigenvector at a perticular scale.
     ImageTypeArray3F::Pointer evPtr = ImageTypeArray3F::New();
     evPtr->SetRegions(eigenvectorPtr->GetLargestPossibleRegion() );
     evPtr->Allocate();
     evPtr->FillBuffer(itk::NumericTraits< ImageTypeArray3F::PixelType >::Zero);

     // define a vesselness buffer for a particular scale.
     ImageType3F::Pointer vnessPtr = ImageType3F::New();
     vnessPtr->SetRegions(vesselnessPtr->GetLargestPossibleRegion() );
     vnessPtr->Allocate();
     vnessPtr->FillBuffer(0);

     // Define Hessian filter.
     typedef itk::HessianRecursiveGaussianImageFilter<ImageType3F, HessianImageType>  HessianFilterType;
     HessianFilterType::Pointer hessianFilter = HessianFilterType::New();
     hessianFilter->SetNormalizeAcrossScale(true);
     hessianFilter->SetInput(intensityPtr);

     if (par.verbose >= 3) {
	  std::cout << "multiscale_hessian(), hessianFilter:\n" << hessianFilter;
     }

     double sigma = par.sigma_min;
     int scaleLevel = 1;

	  itk::ImageRegionIterator<ImageType3F> vnessIt(vnessPtr, vnessPtr->GetLargestPossibleRegion() );
	  itk::ImageRegionIterator<ImageType3F> vesselnessIt(vesselnessPtr, vesselnessPtr->GetLargestPossibleRegion() );

	  itk::ImageRegionIterator<ImageType3F> scaleIt(scalePtr, scalePtr->GetLargestPossibleRegion() );
	  // itk::ImageRegionIterator<ImageType3F> scIt(scPtr, scPtr->GetLargestPossibleRegion() );

	  itk::ImageRegionIterator<ImageTypeArray3F> eigenvectorIt(eigenvectorPtr, eigenvectorPtr->GetLargestPossibleRegion() );
	  itk::ImageRegionIterator<ImageTypeArray3F> evIt(evPtr, evPtr->GetLargestPossibleRegion() );

	  itk::ImageRegionIterator<ImageType3UC> maskIt(maskPtr, maskPtr->GetLargestPossibleRegion() );

     while ( sigma <= par.sigma_max ) {
	  if (par.verbose >= 1) {
	       std::cout << "multiscale_hessian(), current sigma: " << sigma << std::endl;
	  }

	  if ( par.steps == 0 )
	  {
	       break;
	  }

	  hessianFilter->SetSigma(sigma);
	  hessianFilter->Update();

	  // compute vesselness and eigenvector.
	  vnessPtr->FillBuffer(0);
	  evPtr->FillBuffer(itk::NumericTraits< ImageTypeArray3F::PixelType >::Zero);
	  hessian_eigenvector(hessianFilter->GetOutput(), vnessPtr, evPtr, par);

	  // update max response, scale and eigenvectors.

	  ImageType3UC::IndexType maskIdx;
	  for (vnessIt.GoToBegin(), vesselnessIt.GoToBegin(), 
		    eigenvectorIt.GoToBegin(), evIt.GoToBegin(), 
		    maskIt.GoToBegin(), scaleIt.GoToBegin();
	       !maskIt.IsAtEnd();
	       ++ vnessIt, ++ vesselnessIt, ++ eigenvectorIt, ++ evIt, ++ maskIt, ++ scaleIt) {

	       if (vesselnessIt.Value() < vnessIt.Value() ) {
		    vesselnessIt.Value() = vnessIt.Value();
		    scaleIt.Value() = sigma;
		    eigenvectorIt.Value() = evIt.Value();
		    // std::cout << vnessIt.Value() << " " << evIt.Value() << std::endl;
	       }
	  }

	  // compute sigma at next scale.
	  if (par.steps < 2) return par.sigma_min;

	  // assume the log steps.
	  const double stepSize = vnl_math_max( 1e-10, ( vcl_log(par.sigma_max) - vcl_log(par.sigma_min) ) / ( par.steps - 1 ) );
	  sigma = vcl_exp(vcl_log (par.sigma_min) + stepSize * scaleLevel);
	  scaleLevel++;

	  if ( m_NumberOfSigmaSteps == 1 ) break;

     }

}

