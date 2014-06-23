#include <common.h>
#include <iomanip>
#include <utility.h>
#include "itkVector.h"
#include "itkListSample.h"
#include "itkGaussianMixtureModelComponent.h"
#include "itkExpectationMaximizationMixtureModelEstimator.h"
#include <itkImageToListSampleFilter.h>
#include "itkSampleClassifierFilter.h"
#include "itkMaximumDecisionRule.h"

namespace po = boost::program_options;

typedef itk::Vector< float, 1 > MeasurementVectorType;
typedef itk::Statistics::ListSample< MeasurementVectorType > SampleType;
// define components.
typedef itk::Statistics::GaussianMixtureModelComponent< SampleType > ComponentType;

// define estimator
typedef itk::Statistics::ExpectationMaximizationMixtureModelEstimator< SampleType > EstimatorType;
typedef itk::Statistics::SampleClassifierFilter< SampleType > FilterType;
typedef itk::Statistics::MaximumDecisionRule  DecisionRuleType;
typedef FilterType::ClassLabelVectorObjectType               ClassLabelVectorObjectType;
typedef FilterType::ClassLabelVectorType                     ClassLabelVectorType;
typedef FilterType::ClassLabelType        ClassLabelType;

int main( int argc, char* argv[] )
{
     std::string input_file, seg_file, mask_file;
     unsigned n_comp = 5, maxit = 50;
     unsigned short verbose = 0;
     po::options_description mydesc("Because of the need of negative number as arguments, there is no short form of argument in this code.");
     mydesc.add_options()
	  ("help,h", "Multiscale Hessian filter.")
	  ("input", po::value<std::string>(&input_file)->default_value("input.nii.gz"), 
	   "Input gray level intensity image filename.")
	  ("seg", po::value<std::string>(&seg_file)->default_value("seg.nii.gz"), 
	   "Output segmentation label filename.")

	  ("mask", po::value<std::string>(&mask_file)->default_value("mask.nii.gz"), 
	   "mask file. Must be binary.")

	  ("ncomp", po::value<unsigned >(&n_comp)->default_value(5), 
	   "Number of components, or classes.")

	  ("mean", po::value<std::vector<double> >()->multitoken(), "Initial mean vector of the GMM. Must be same number of ncomp.")

	  ("sigma", po::value<std::vector<double> >()->multitoken(), "Initial standard deviation vector of the GMM, assuming a diagonal cov matrix. Must be same number of ncomp")

	  ("prop", po::value<std::vector<double> >()->multitoken(), "Initial proportion vector of the GMM, assuming a diagonal cov matrix. Must be same number of ncomp")

	  ("maxit", po::value<unsigned>(&maxit)->default_value(50), 
	   "Max number of EM iterations.")

	  ("verbose", po::value<unsigned short>(&verbose)->default_value(0), 
	   "verbose level. 0 for minimal output. 3 for most output.");


     po::variables_map vm;        
     po::store(po::parse_command_line(argc, argv, mydesc, po::command_line_style::unix_style ^ po::command_line_style::allow_short), vm);
     po::notify(vm);    

     try {
	  if ( (vm.count("help")) | (argc == 1) ) {
	       std::cout << "Usage: groupmrf [options]\n";
	       std::cout << mydesc << "\n";
	       return 0;
	  }
     }
     catch(std::exception& e) {
	  std::cout << e.what() << "\n";
	  return 1;
     }    

     std::vector<double> mean_opt, sigma_opt, prop_opt;
     if (!vm["mean"].empty() && (mean_opt = vm["mean"].as<std::vector<double> >()).size() == n_comp) {
     }
     else {
	  std::cout << "mean parameter must have length n_comp.\n";
	  exit(1);
     }
     if (!vm["sigma"].empty() && (sigma_opt = vm["sigma"].as<std::vector<double> >()).size() == n_comp) {
     }
     else {
	  std::cout << "Sigma parameter must have length n_comp.\n";
     }

     if (!vm["prop"].empty() && (prop_opt = vm["prop"].as<std::vector<double> >()).size() == n_comp) {
     }
     else {
	  std::cout << "prop parameter must have length n_comp.\n";
     }

     // read original gray intensity image.
     ReaderType3F::Pointer inReader = ReaderType3F::New();
     inReader->SetFileName(input_file);
     inReader->Update();
     ImageType3F::Pointer inPtr = inReader->GetOutput();

     // read mask file
     ReaderType3UC::Pointer maskReader = ReaderType3UC::New();
     maskReader->SetFileName(mask_file);
     maskReader->Update();
     ImageType3UC::Pointer maskPtr = maskReader->GetOutput();

     // convert image to list sample. ITK has imageToListSample filter, but
     // could not save the correspondence between voxels and samples. We need
     // such correspondence since we need to save a label volume. So, here we
     // convert the volume to samples by hand, and same the mapping, too.

     // first define a image buffer to map voxel to linear sample index. 
     ImageType3U::Pointer to_samplePtr = ImageType3U::New();
     to_samplePtr->SetRegions(inPtr->GetLargestPossibleRegion());
     to_samplePtr->Allocate();
     to_samplePtr->FillBuffer(0);
     itk::ImageRegionIterator<ImageType3U> to_sampleIt(to_samplePtr, to_samplePtr->GetLargestPossibleRegion());
     
     SampleType::Pointer sample = SampleType::New();

     itk::ImageRegionIterator<ImageType3UC> maskIt(maskPtr, maskPtr->GetLargestPossibleRegion());
     itk::ImageRegionIterator<ImageType3F> featureIt(inPtr, inPtr->GetLargestPossibleRegion());
     
     unsigned n  = 0;
     MeasurementVectorType this_sample;
     for(maskIt.GoToBegin(), featureIt.GoToBegin(), to_sampleIt.GoToBegin(); !maskIt.IsAtEnd(); ++ maskIt, ++ featureIt, ++ to_sampleIt) {
     	  if (maskIt.Get() > 0) {
     	       this_sample[0] = featureIt.Get();
     	       sample->PushBack(this_sample);
     	       to_sampleIt.Set(n);
     	       n ++;
     	  }
     }

     printf("gmm(), total number of samples inside mask: %i\n", sample->Size());

     // init mean and sigma
     typedef itk::Array< double > ParametersType;
     ParametersType params( 2 );
     std::vector< ParametersType > initialParameters( n_comp );
     
     for (unsigned k  = 0; k < n_comp; k ++) {
	  params[0] = mean_opt[k];
	  params[1] = sigma_opt[k] * sigma_opt[k];
	  initialParameters[k] = params;
     }

     std::vector< ComponentType::Pointer > components;
     for ( unsigned int k = 0; k < n_comp; k++) {
	 components.push_back( ComponentType::New() );
	 (components[k])->SetSample( sample );
	 (components[k])->SetParameters( initialParameters[k] );
    }
     
     EstimatorType::Pointer estimator = EstimatorType::New();
     estimator->SetSample( sample );
     estimator->SetMaximumIteration( maxit );

     itk::Array< double > initialProportions(n_comp);
     for (unsigned k  = 0; k < n_comp; k ++) {
	  initialProportions[k] = prop_opt[k];
     }
     estimator->SetInitialProportions( initialProportions );

     for ( unsigned int i = 0; i < n_comp; i++)
     {
	  estimator->AddComponent( (ComponentType::Superclass*)
				   (components[i]).GetPointer() );
     }

     estimator->Update();
     for ( unsigned int i = 0; i < n_comp; i++ )
     {
	  std::cout << "Cluster[" << i << "]" << std::endl;
	  std::cout << "    Parameters:" << std::endl;

	  std::cout << "         " << (components[i])->GetFullParameters() << std::endl;
	  std::cout << "    Proportion: ";
	  std::cout << std::setprecision(4) << "         " << estimator->GetProportions()[i] << std::endl;
     }

     // Display the membership of each sample
     DecisionRuleType::Pointer    decisionRule = DecisionRuleType::New();
     ClassLabelVectorObjectType::Pointer  classLabelsObject = ClassLabelVectorObjectType::New();
     ClassLabelVectorType & classLabelVector  = classLabelsObject->Get();

     for (unsigned k = 0; k < n_comp; k ++) {
	  classLabelVector.push_back( k + 1);
	  }
  
     FilterType::Pointer sampleClassifierFilter = FilterType::New();
     sampleClassifierFilter->SetInput( sample );
     sampleClassifierFilter->SetNumberOfClasses( n_comp  );
     sampleClassifierFilter->SetClassLabels( classLabelsObject );
     sampleClassifierFilter->SetDecisionRule( decisionRule );
     sampleClassifierFilter->SetMembershipFunctions( estimator->GetOutput() );
     sampleClassifierFilter->Update();

     const FilterType::MembershipSampleType* membershipSample = sampleClassifierFilter->GetOutput();
     FilterType::MembershipSampleType::ConstIterator iter = membershipSample->Begin();

     // save label to volume
     // first define a image buffer to save labels.
     ImageType3U::Pointer labelPtr = ImageType3U::New();
     labelPtr->SetRegions(inPtr->GetLargestPossibleRegion());
     labelPtr->Allocate();
     labelPtr->FillBuffer(0);
     labelPtr->SetOrigin(inPtr->GetOrigin());
     labelPtr->SetDirection(inPtr->GetDirection());
     labelPtr->SetSpacing(inPtr->GetSpacing());
     itk::ImageRegionIterator<ImageType3U> labelIt(labelPtr, labelPtr->GetLargestPossibleRegion());

     ImageType3UC::IndexType maskIdx;
     for(maskIt.GoToBegin(), to_sampleIt.GoToBegin(), labelIt.GoToBegin(); !maskIt.IsAtEnd(); ++ maskIt, ++ to_sampleIt, ++ labelIt) {
     	  if (maskIt.Get() > 0) {
     	       maskIdx = maskIt.GetIndex();
     	       labelIt.Set(membershipSample->GetClassLabel(to_sampleIt.Get()));
     	  }
     }

     save_volume(labelPtr, seg_file);
}

