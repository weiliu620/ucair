#include <common.h>
#include <utility.h>

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkHessianToObjectnessMeasureImageFilter.h"
#include "myMultiScaleHessianBasedMeasureImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

namespace po = boost::program_options;

int main( int argc, char* argv[] )
{
     const unsigned int Dimension = 3;
     std::string inputFileName, outputFileName, scalemapFileName;
     double sigmaMinimum = 1.0;
     double sigmaMaximum = 10.0;
     double alpha = 0.5, beta = 1, gamma = 5;
     unsigned int numberOfSigmaSteps = 10;
     unsigned short verbose = 0;
     po::options_description mydesc("Options can only used at commandline");
     mydesc.add_options()
	  ("help,h", "Multiscale Hessian filter.")
	  ("input,i", po::value<std::string>(&inputFileName)->default_value("input.nii.gz"), 
	   "Input file name.")
	  ("output,o", po::value<std::string>(&outputFileName)->default_value("output.nii.gz"), 
	   "Output file name.")
	  ("scalemap,c", po::value<std::string>(&scalemapFileName)->default_value("scale_map.nii.gz"), 
	   "scale map file name.")
	  ("min,m", po::value<double>(&sigmaMinimum)->default_value(1), 
	   "Minimal sigma")
	  ("max,x", po::value<double>(&sigmaMaximum)->default_value(10), 
	   "Maxmal sigma")

	  ("alpha,a", po::value<double>(&alpha)->default_value(0.5), 
	   "Alpha for Hessian filter.")
	  ("beta,b", po::value<double>(&beta)->default_value(1), 
	   "Beta for Hessian filter.")
	  ("gamma,g", po::value<double>(&gamma)->default_value(5), 
	   "Gamma for Hessian filter.")

	  ("steps,s", po::value<unsigned int>(&numberOfSigmaSteps)->default_value(10), 
	   "Number of sigma steps.")

	  ("verbose,v", po::value<unsigned short>(&verbose)->default_value(0), 
	   "verbose level. 0 for minimal output. 3 for most output.");

     po::variables_map vm;        
     po::store(po::parse_command_line(argc, argv, mydesc), vm);
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

  typedef double                              PixelType;
  typedef itk::Image< PixelType, Dimension > ImageType;

  typedef itk::ImageFileReader< ImageType >  ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( inputFileName );

  typedef itk::SymmetricSecondRankTensor< double, Dimension > HessianPixelType;
  typedef itk::Image< HessianPixelType, Dimension >           HessianImageType;
  typedef itk::HessianToObjectnessMeasureImageFilter< HessianImageType, ImageType >
    ObjectnessFilterType;
  ObjectnessFilterType::Pointer objectnessFilter = ObjectnessFilterType::New();
  objectnessFilter->SetBrightObject( true );
  objectnessFilter->SetScaleObjectnessMeasure( false );
  objectnessFilter->SetAlpha( alpha );
  objectnessFilter->SetBeta( beta );
  objectnessFilter->SetGamma( gamma );

  // debug.
  std::cout << "objectnessFilter's object dimension: " << objectnessFilter->GetObjectDimension() << std::endl;

  typedef itk::MultiScaleHessianBasedMeasureImageFilter< ImageType, HessianImageType, ImageType, ImageType >
    MultiScaleEnhancementFilterType;
  MultiScaleEnhancementFilterType::Pointer multiScaleEnhancementFilter =
    MultiScaleEnhancementFilterType::New();
  multiScaleEnhancementFilter->SetInput( reader->GetOutput() );
  multiScaleEnhancementFilter->SetHessianToMeasureFilter( objectnessFilter );
  multiScaleEnhancementFilter->SetSigmaStepMethodToLogarithmic();
  multiScaleEnhancementFilter->SetSigmaMinimum( sigmaMinimum );
  multiScaleEnhancementFilter->SetSigmaMaximum( sigmaMaximum );
  multiScaleEnhancementFilter->SetNumberOfSigmaSteps( numberOfSigmaSteps );
  multiScaleEnhancementFilter->SetNonNegativeHessianBasedMeasure(true);

  multiScaleEnhancementFilter->SetGenerateScalesOutput(true);
  multiScaleEnhancementFilter->SetGenerateHessianOutput(true);
  std::cout << "generate scale outputs status: " << multiScaleEnhancementFilter->GetGenerateScalesOutput() << std::endl;
  std::cout << "generate Hessian outputs status: " << multiScaleEnhancementFilter->GetGenerateHessianOutput() << std::endl;
  
  typedef itk::RescaleIntensityImageFilter< ImageType, ImageType3DUC >  RescaleFilterType;
  RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  rescaleFilter->SetInput( multiScaleEnhancementFilter->GetOutput() );
  
  save_volume(rescaleFilter->GetOutput(), outputFileName);

  // also save the scale map. 
  WriterType3DF::Pointer scale_writer = WriterType3DF::New();
  scale_writer->SetInput( multiScaleEnhancementFilter->GetScalesOutput() );
  scale_writer->SetFileName( scalemapFileName);

  try
  {
       scale_writer->Update();
  }
  catch( itk::ExceptionObject & error )
  {
       std::cerr << "Error: " << error << std::endl;
       return EXIT_FAILURE;
  }
  std::cout << "multilescae_hessian(): file " << scalemapFileName << " saved. " << std::endl; 


  // return EXIT_SUCCESS;
}
