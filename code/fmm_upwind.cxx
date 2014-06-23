#include <common.h>
#include <utility.h>
#include "itkFastMarchingImageToNodePairContainerAdaptor.h"
#include "itkFastMarchingThresholdStoppingCriterion.h"
#include <itkFastMarchingUpwindGradientImageFilterBase.h>

typedef itk::FastMarchingUpwindGradientImageFilterBase< ImageType3F, ImageType3F > FastMarchingFilterType;
typedef itk::FastMarchingImageToNodePairContainerAdaptor< ImageType3F, ImageType3F, ImageType3UC > AdaptorType;
typedef FastMarchingFilterType::GradientImageType  FloatGradientImage;
typedef FloatGradientImage::PixelType   GradientPixelType;

namespace po = boost::program_options;
int main(int argc, char* argv[])
{
     std::string speed_file, seed_file, out_file, mask_file, trial_file;
     float sigma = 0.01;
     float sigmoid_alpha = -5, sigmoid_beta = 50;
     unsigned short verbose = 0;
     float stop_time = 100;
     double const_speed = 0.01;

     po::options_description mydesc("Options can only used at commandline");
     mydesc.add_options()
	  ("help,h", "Fast Marching method with upwind gradient output.")
	  ("speed,p", po::value<std::string>(&speed_file)->default_value("speed.nii.gz"), 
	   "Speed image.")

	  ("seed,e", po::value<std::string>(&seed_file)->default_value("seed.nii.gz"), 
	   "A mask file for the seed region.")

	  ("mask,m", po::value<std::string>(&mask_file)->default_value("mask.nii.gz"), 
	   "mask file. Propagation happens only within the mask")

	  ("stoptime,t", po::value<float>(&stop_time)->default_value(100), 
	   "The stop time for the Fast Marching filter.")

	  ("output,o", po::value<std::string>(&out_file)->default_value("output.nii.gz"), 
	   "Output time map.")

	  // ("trial,r", po::value<std::string>(&trial_file)->default_value("trial.nii.gz"), 
	  //  "Binary file containing the trail points, i.e. the front end points which will be used for voting")

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

     // read in speed map.
     ReaderType3F::Pointer speedReader = ReaderType3F::New();
     speedReader->SetFileName(speed_file);
     speedReader->Update();
     speedReader->ReleaseDataFlagOn();
     // ImageType3F::Pointer speedPtr = speedReader->GetOutput();

     // read in mask file.
     ReaderType3UC::Pointer maskReader = ReaderType3UC::New();
     maskReader->SetFileName(mask_file);
     maskReader->Update();
     maskReader->ReleaseDataFlagOn();
     ImageType3UC::Pointer maskPtr = maskReader->GetOutput();

     // read in seed file.
     ReaderType3UC::Pointer seedReader = ReaderType3UC::New();
     seedReader->SetFileName(seed_file);
     seedReader->Update();
     seedReader->ReleaseDataFlagOn();
     ImageType3UC::Pointer seedPtr = seedReader->GetOutput();

     FastMarchingFilterType::Pointer marcher = FastMarchingFilterType::New();


     marcher->SetInput( speedReader->GetOutput() );

     AdaptorType::Pointer adaptor = AdaptorType::New();
     adaptor->SetIsForbiddenImageBinaryMask( true );

     // adaptor->SetAliveImage( AliveImage.GetPointer() );
     // adaptor->SetAliveValue( 0.0 );

     adaptor->SetTrialImage( seedPtr.GetPointer() );
     adaptor->SetTrialValue( 1.0 );

     adaptor->SetForbiddenImage( maskPtr.GetPointer() );
     adaptor->Update();

     marcher->SetForbiddenPoints( adaptor->GetForbiddenPoints() );
     marcher->SetTrialPoints( adaptor->GetTrialPoints() );

     // stop criterion.
     typedef  itk::FastMarchingThresholdStoppingCriterion< ImageType3F, ImageType3F > CriterionType;
     CriterionType::Pointer criterion = CriterionType::New();
     criterion->SetThreshold(stop_time);
     marcher->SetStoppingCriterion( criterion );

     // run the marcher.
     marcher->Update();

     // check the results
     FloatGradientImage::Pointer gradPtr = marcher->GetGradientImage();

     if (verbose >= 1) {
	  typedef itk::ImageFileWriter<FloatGradientImage> WriterType;
	  WriterType::Pointer writer = WriterType::New();
	  writer->SetInput(gradPtr);
	  writer->SetFileName("fmm_gradient.mha");
	  try { 
	       writer->Update(); 
	  } 
	  catch( itk::ExceptionObject & err ) 
	  { 
	       std::cerr << "ExceptionObject caught !" << std::endl; 
	       std::cerr << err << std::endl; 
	       return EXIT_FAILURE;
	  } 
	  std::cout << "save_volume(): File " << "fmm_gradient.mha" << " saved.\n";
     }

     save_volume(marcher->GetOutput(), out_file);

     // now collect trail points.
     ImageType3UC::Pointer trialPtr = ImageType3UC::New();
     trialPtr->SetRegions(maskPtr->GetLargestPossibleRegion());
     trialPtr->SetSpacing(maskPtr->GetSpacing());
     trialPtr->SetOrigin(maskPtr->GetOrigin());
     trialPtr->SetDirection(maskPtr->GetDirection());
     trialPtr->Allocate();
     trialPtr->FillBuffer(0);  
     
     FastMarchingFilterType::NodePairContainerType::Pointer node_pairs = marcher->GetTrialPoints();
     FastMarchingFilterType::NodePairContainerType::Iterator nodeIt;
      
     for(nodeIt = node_pairs->Begin(); nodeIt != node_pairs->End(); nodeIt ++)
     {
     	  trialPtr->SetPixel(nodeIt->Value().GetNode(), 1);
     }
     
     // save_volume(trialPtr, trial_file);
     return 0;
}

		       
