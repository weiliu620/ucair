#include <common.h>
#include <utility.h>
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include <itkImageToListSampleFilter.h>
#include "itkFastMarchingImageFilter.h"
#include "itkMeanSampleFilter.h"
#include "itkCovarianceSampleFilter.h"
#include "itkGaussianMembershipFunction.h"
#include "itkSigmoidImageFilter.h"
#include "itkFastMarchingImageToNodePairContainerAdaptor.h"
#include "itkFastMarchingThresholdStoppingCriterion.h"

namespace po = boost::program_options;
int main(int argc, char* argv[])
{
     std::string in_file, seed_file, out_file, bodymask_file;
     unsigned shortverbose = 0;
     float sigma = 0.01;
     float sigmoid_alpha = -5, sigmoid_beta = 50;
     unsigned short verbose = 0;
     float stop_time = 100;

     po::options_description mydesc("Options can only used at commandline");
     mydesc.add_options()
	  ("help,h", "Canny filter outputs the edge map.")
	  ("int,i", po::value<std::string>(&in_file)->default_value("input.nii.gz"), 
	   "Input intensity image.")

	  ("seed,e", po::value<std::string>(&seed_file)->default_value("seed.nii.gz"), 
	   "A mask file for the seed region.")

	   ("bodymask,m", po::value<std::string>(&bodymask_file)->default_value("mask.nii.gz"), 
	    "Body mask file.")

	  // ("sigma,s", po::value<float>(&sigma)->default_value(1.5), 
	  //  "Gaussian filter sigma.")


	  ("sigmoidalpha,", po::value<float>(&sigmoid_alpha)->default_value(-5), 
	   "Alpha of sigmoid function. Small value generate steep sigmoid function.")

	  ("sigmoidbeta,", po::value<float>(&sigmoid_beta)->default_value(50), 
	   "beta of sigmoid function, indicating the center of the range.")

	  ("stoptime,t", po::value<float>(&stop_time)->default_value(100), 
	   "The stop time for the Fast Marching filter.")

	  ("output,o", po::value<std::string>(&out_file)->default_value("output.nii.gz"), 
	   "Output binary volumge.")

	  ("verbose,v", po::value<unsigned short>(&verbose)->default_value(0), 
	   "verbose level. 0 for minimal output. 3 for most output.");

     po::variables_map vm;        
     po::store(po::parse_command_line(argc, argv, mydesc), vm);
     po::notify(vm);    

     try {
	  if ( (vm.count("help")) | (argc == 1) ) {
	       std::cout << "Usage: closing_filter [options]\n";
	       std::cout << mydesc << "\n";
	       return 0;
	  }
     }
     catch(std::exception& e) {
	  std::cout << e.what() << "\n";
	  return 1;
     }    

     // read in original intensity file
     ReaderType3F::Pointer inReader = ReaderType3F::New();
     inReader->SetFileName(in_file);
     inReader->Update();
     ImageType3F::Pointer inPtr = inReader->GetOutput();

     // read in body mask file.
     ReaderType3UC::Pointer maskReader = ReaderType3UC::New();
     maskReader->SetFileName(bodymask_file);
     maskReader->Update();
     ImageType3UC::Pointer maskPtr = maskReader->GetOutput();

     // read in seed mask file.
     ReaderType3UC::Pointer seedReader = ReaderType3UC::New();
     seedReader->SetFileName(seed_file);
     seedReader->Update();
     ImageType3UC::Pointer seedPtr = seedReader->GetOutput();

     typedef itk::GradientMagnitudeRecursiveGaussianImageFilter<ImageType3F>  DerivativeFilterType;
     DerivativeFilterType::Pointer derivative_filter = DerivativeFilterType::New();

     derivative_filter->SetInput(inPtr);
     derivative_filter->SetSigma(sigma);
     if (verbose >=1 ) {
	  save_volume(derivative_filter->GetOutput(), "gradient_mag.nii.gz");
     }
     
     // convert gradient magnitude to speed map.
     typedef itk::SigmoidImageFilter<ImageType3F, ImageType3F > SigmoidFilterType;
     SigmoidFilterType::Pointer sigmoidFilter = SigmoidFilterType::New();     
     sigmoidFilter->SetInput(derivative_filter->GetOutput());
     sigmoidFilter->SetOutputMinimum( 0 );
     sigmoidFilter->SetOutputMaximum( 1 );
     sigmoidFilter->SetAlpha(sigmoid_alpha);
     sigmoidFilter->SetBeta(sigmoid_beta);
     if (verbose >=1) {
	  save_volume(sigmoidFilter->GetOutput(), "sigmoid_out.nii.gz");
     }

     typedef itk::FastMarchingImageFilter< ImageType3F, ImageType3F > FastMarchingFilterType;
     FastMarchingFilterType::Pointer fastMarching = FastMarchingFilterType::New();
     fastMarching->SetInput( sigmoidFilter->GetOutput() );

     // we don't really need theses.
     fastMarching->SetOverrideOutputInformation(true);
     fastMarching->SetOutputSize(inPtr->GetLargestPossibleRegion().GetSize());
     fastMarching->SetOutputSpacing(inPtr->GetSpacing());
     fastMarching->SetOutputOrigin(inPtr->GetOrigin()); 
     fastMarching->SetOutputDirection(inPtr->GetDirection());

     typedef FastMarchingFilterType::NodeContainer NodeContainer;
     typedef FastMarchingFilterType::NodeType NodeType;
     NodeContainer::Pointer seeds = NodeContainer::New();
     NodeType node;
     const double seedValue = 0.0;
     seeds->Initialize();
     IteratorType3UC seedIt(seedPtr, seedPtr->GetLargestPossibleRegion());

     unsigned nodeIdx = 0;
     for (seedIt.GoToBegin(); !seedIt.IsAtEnd(); ++ seedIt) {
	  if (seedIt.Get() > 0) {
	       node.SetValue(seedValue);
	       node.SetIndex(seedIt.GetIndex());
	       seeds->InsertElement(nodeIdx, node);
	       nodeIdx ++;
	  }
     }

     // nodeIdx ++ then is the number of nodes.
     printf("Total number of voxels in seed region: %i\n", nodeIdx);
     fastMarching->SetTrialPoints( seeds );

     // Set the mask. Outside the mask will not be computed. it seems I have to
     // use GetPointer here.
     fastMarching->SetBinaryMask(maskPtr.GetPointer());
     
     // step stop rule
     // no such function. only has such func in base filter. Double check latest ITK source. 
     // typedef itk::FastMarchingThresholdStoppingCriterion< ImageType3F,  ImageType3F > CriterionType;
     // CriterionType::Pointer criterion = CriterionType::New();
     // criterion->SetThreshold( 100. );
     // fastMarching->SetStoppingCriterion( criterion ) 
     
     // Note: I've changed the filter source code in ITK, such that the voxels
     // not explored has value equal to the stop value, instead of the max float
     // point number
     fastMarching->SetStoppingValue(stop_time);

     // turn on debugging
     fastMarching->DebugOn();

     // update the marcher. 
     fastMarching->Update();
     save_volume(fastMarching->GetOutput(), out_file); 

//      // convert voxels in the seed regions to samples.
//      typedef itk::Statistics::ImageToListSampleFilter<ImageType3F, ImageType3UC> SampleFilterType;
//      SampleFilterType::Pointer sample_filter = SampleFilterType::New();
//      sample_filter->SetInput(inPtr);
//      sample_filter->SetMaskImage(seedPtr);
//      sample_filter->SetMaskValue(1);
//      typedef SampleFilterType::ListSampleType SampleType;
//      typedef SampleFilterType::MeasurementVectorType  MeasurementVectorType;

//      typedef itk::Statistics::MeanSampleFilter< SampleType > MeanAlgorithmType;
//      MeanAlgorithmType::Pointer meanAlgorithm = MeanAlgorithmType::New();
//      meanAlgorithm->SetInput( sample_filter->GetOutput() );
//      meanAlgorithm->Update();

//      typedef itk::Statistics::CovarianceSampleFilter< SampleType > CovarianceAlgorithmType;
//      CovarianceAlgorithmType::Pointer covarianceAlgorithm =
// 	  CovarianceAlgorithmType::New();
//      covarianceAlgorithm->SetInput( sample_filter->GetOutput() );
//      covarianceAlgorithm->Update();

//      if (verbose >=1 ) {
//      printf("Seed region mean: %.2f, variance: %.2f.\n", meanAlgorithm->GetMean()[0], covarianceAlgorithm->GetCovarianceMatrix()[0][0]);
//      }

//      // compute the density map, i.e. P(x), where x is pixel value, and P is
//      // Gaussian pdf.
//      typedef itk::Statistics::GaussianMembershipFunction< MeasurementVectorType >
// DensityFunctionType;
//      DensityFunctionType::Pointer densityFunction = DensityFunctionType::New();
//      densityFunction->SetMean(meanAlgorithm->GetMean());
//      // densityFunction->SetCovariance(covarianceAlgorithm->GetCovarianceMatrix());
//      CovarianceAlgorithmType::MatrixType covariance = covarianceAlgorithm->GetCovarianceMatrix();
//      covariance[0][0] = 120*120;
//      densityFunction->SetCovariance(covariance);
     
//      // make a density volume.
//      ImageType3F::Pointer densityPtr = ImageType3F::New();
//      densityPtr->SetRegions(inPtr->GetLargestPossibleRegion());
//      densityPtr->Allocate();
//      densityPtr->FillBuffer( 0 );
//      densityPtr->SetOrigin(inPtr->GetOrigin());
//      densityPtr->SetSpacing(inPtr->GetSpacing());
//      densityPtr->SetDirection(inPtr->GetDirection());
//      IteratorType3F densityIt(densityPtr, densityPtr->GetLargestPossibleRegion());
     
//      IteratorType3F inIt(inPtr, inPtr->GetLargestPossibleRegion());
//      IteratorType3UC maskIt(maskPtr, maskPtr->GetLargestPossibleRegion());
//      for (inIt.GoToBegin(), maskIt.GoToBegin(), densityIt.GoToBegin(); !maskIt.IsAtEnd(); ++ maskIt, ++ inIt, ++ densityIt) {
// 	  if (maskIt.Get() > 0) {
// 	       densityIt.Set(densityFunction->Evaluate(inIt.Get()));
// 	  }
//      }	       

//      if (verbose >= 1) {
// 	  save_volume(densityPtr, "density.nii.gz");
//      }
    
     // // normalize the density into (0, 1) with a nonlinear mapping.
     // sigmoidFilter->SetInput(densityPtr);
     // sigmoidFilter->SetOutputMinimum( 0 );
     // sigmoidFilter->SetOutputMaximum( 1 );
     // sigmoidFilter->SetAlpha(density_alpha);
     // sigmoidFilter->SetBeta(density_beta);
     // sigmoidFilter->Update();
     // if (verbose >= 0) {
     // 	  save_volume(sigmoidFilter->GetOutput(), "density_nor.nii.gz");
     // }

     // weight the speed map by the probability map, to make sure the non-vessel
     // region really have low speed.


}
     

