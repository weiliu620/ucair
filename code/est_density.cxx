#include <common.h>
#include <utility.h>
#include <itkImageToListSampleFilter.h>
#include "itkMeanSampleFilter.h"
#include "itkCovarianceSampleFilter.h"
#include "itkGaussianMembershipFunction.h"
#include "itkSigmoidImageFilter.h"
#include <itkRescaleIntensityImageFilter.h>

namespace po = boost::program_options;
int main(int argc, char* argv[])
{
     std::string in_file, seed_file, out_file, bodymask_file;
     unsigned shortverbose = 0;
     float sigma = 0.01;
     // float alpha = 0, beta = 1;
     unsigned short verbose = 0;
     float stop_time = 100, std = 120;

     po::options_description mydesc("Fast marching segmentation.");
     mydesc.add_options()
	  ("help,h", "Given the seed regions defined by a mask, output the probabilistic density map of the the full volume.")
	  ("int,i", po::value<std::string>(&in_file)->default_value("input.nii.gz"), 
	   "Input intensity image.")

	  ("seed,e", po::value<std::string>(&seed_file)->default_value("seed.nii.gz"), 
	   "A mask file for the seed region.")

	   ("bodymask,m", po::value<std::string>(&bodymask_file)->default_value("mask.nii.gz"), 
	    "Body mask file.")

	  ("output,o", po::value<std::string>(&out_file)->default_value("output.nii.gz"), 
	   "Output binary volumge.")

	  ("std,", po::value<float>(&std)->default_value(120), 
	   "Manual given standard deviation of the Gaussian distribtuion.")

	  // ("alpha,", po::value<float>(&alpha)->default_value(0), 
	  //  "Alpha of sigmoid function. Small value generate steep sigmoid function.")

	  // ("beta,", po::value<float>(&beta)->default_value(1), 
	  //  "beta of sigmoid function, indicating the center of the range.")

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


     // convert voxels in the seed regions to samples.
     typedef itk::Statistics::ImageToListSampleFilter<ImageType3F, ImageType3UC> SampleFilterType;
     SampleFilterType::Pointer sample_filter = SampleFilterType::New();
     sample_filter->SetInput(inPtr);
     sample_filter->SetMaskImage(seedPtr);
     sample_filter->SetMaskValue(1);
     typedef SampleFilterType::ListSampleType SampleType;
     typedef SampleFilterType::MeasurementVectorType  MeasurementVectorType;

     typedef itk::Statistics::MeanSampleFilter< SampleType > MeanAlgorithmType;
     MeanAlgorithmType::Pointer meanAlgorithm = MeanAlgorithmType::New();
     meanAlgorithm->SetInput( sample_filter->GetOutput() );
     meanAlgorithm->Update();

     typedef itk::Statistics::CovarianceSampleFilter< SampleType > CovarianceAlgorithmType;
     CovarianceAlgorithmType::Pointer covarianceAlgorithm =
	  CovarianceAlgorithmType::New();
     covarianceAlgorithm->SetInput( sample_filter->GetOutput() );
     covarianceAlgorithm->Update();

     if (verbose >=1 ) {
     printf("Seed region mean: %.2f, variance: %.2f.\n", meanAlgorithm->GetMean()[0], covarianceAlgorithm->GetCovarianceMatrix()[0][0]);
     }

     // compute the density map, i.e. P(x), where x is pixel value, and P is
     // Gaussian pdf.
     typedef itk::Statistics::GaussianMembershipFunction< MeasurementVectorType >
DensityFunctionType;
     DensityFunctionType::Pointer densityFunction = DensityFunctionType::New();
     densityFunction->SetMean(meanAlgorithm->GetMean());
     // densityFunction->SetCovariance(covarianceAlgorithm->GetCovarianceMatrix());
     CovarianceAlgorithmType::MatrixType covariance = covarianceAlgorithm->GetCovarianceMatrix();
     covariance[0][0] = std * std;
     densityFunction->SetCovariance(covariance);
     
     // make a density volume.
     ImageType3F::Pointer densityPtr = ImageType3F::New();
     densityPtr->SetRegions(inPtr->GetLargestPossibleRegion());
     densityPtr->Allocate();
     densityPtr->FillBuffer( 0 );
     densityPtr->SetOrigin(inPtr->GetOrigin());
     densityPtr->SetSpacing(inPtr->GetSpacing());
     densityPtr->SetDirection(inPtr->GetDirection());
     IteratorType3F densityIt(densityPtr, densityPtr->GetLargestPossibleRegion());
     
     IteratorType3F inIt(inPtr, inPtr->GetLargestPossibleRegion());
     IteratorType3UC maskIt(maskPtr, maskPtr->GetLargestPossibleRegion());
     for (inIt.GoToBegin(), maskIt.GoToBegin(), densityIt.GoToBegin(); !maskIt.IsAtEnd(); ++ maskIt, ++ inIt, ++ densityIt) {
	  if (maskIt.Get() > 0) {
	       densityIt.Set(densityFunction->Evaluate(inIt.Get()));
	  }
     }	       

     if (verbose >= 1) {
	  save_volume(densityPtr, "density.nii.gz");
     }

     typedef itk::RescaleIntensityImageFilter< ImageType3F, ImageType3F > RescaleFilterType;
     RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
     rescaleFilter->SetInput(densityPtr);
     rescaleFilter->SetOutputMinimum(0);
     rescaleFilter->SetOutputMaximum(1);

     save_volume(rescaleFilter->GetOutput(), out_file);
}

