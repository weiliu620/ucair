#include <common.h>
#include <utility.h>
#include "itkSymmetricSecondRankTensor.h"
#include "itkSymmetricEigenAnalysis.h"

typedef double EigenValueType;
typedef itk::FixedArray< EigenValueType, 3 > EigenValueArrayType;

bool AbsLessEqualCompare(EigenValueType a, EigenValueType b);

int hessian_eigenvector(HessianImageType::Pointer hessianPtr,
			ImageType3D::Pointer vesselnessPtr,
			ImageTypeArray3D::Pointer eigenvectorPtr)
{
     double m_Alpha = 0.5;
     double m_Beta = 0.5;
     double m_Gamma = 5.0;
     unsigned m_ObjectDimension = 1;
     bool m_BrightObject = true;
     unsigned ImageDimension = 3;
     bool m_ScaleObjectnessMeasure = true;
     
     typedef itk::SymmetricEigenAnalysis< HessianImageType::PixelType, EigenValueArrayType > CalculatorType;
     CalculatorType eigenCalculator(3);

     itk::ImageRegionConstIterator< HessianImageType > it(hessianPtr, hessianPtr->GetLargestPossibleRegion());     
     itk::ImageRegionIterator< ImageType3D > oit(vesselnessPtr, vesselnessPtr->GetLargestPossibleRegion());

     oit.GoToBegin();
     it.GoToBegin();

     while ( !it.IsAtEnd() )
     {
     	  // compute eigen values
     	  EigenValueArrayType eigenValues;
     	  eigenCalculator.ComputeEigenValues(it.Get(), eigenValues);

     	  // Sort the eigenvalues by magnitude but retain their sign.
     	  // The eigenvalues are to be sorted |e1|<=|e2|<=...<=|eN|
     	  EigenValueArrayType sortedEigenValues = eigenValues;
     	  std::sort( sortedEigenValues.Begin(), sortedEigenValues.End(), AbsLessEqualCompare );

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
     	       objectnessMeasure *= sortedAbsEigenValues[ImageDimension - 1];
     	  }

     	  oit.Set( static_cast< double >( objectnessMeasure ) );

     	  ++it;
     	  ++oit;
     }

}
			
bool AbsLessEqualCompare(EigenValueType a, EigenValueType b)
{
     return vnl_math_abs(a) <= vnl_math_abs(b);
}

