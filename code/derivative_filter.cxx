#include <common.h>
#include <utility.h>
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
namespace po = boost::program_options;
int main(int argc, char* argv[])
{
     std::string in_file, out_file;
     unsigned short verbose = 0;
     float sigma = 1.5;

     po::options_description mydesc("Options can only used at commandline");
     mydesc.add_options()
	  ("help,h", "Mophological closing filter")
	  ("input,i", po::value<std::string>(&in_file)->default_value("input.nii.gz"), 
	   "Input binary volumge.")
	  ("output,o", po::value<std::string>(&out_file)->default_value("output.nii.gz"), 
	   "Output binary volumge.")
	  ("sigma,s", po::value<float>(&sigma)->default_value(1.5), 
	   "Radius of the structure elment.")
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

     // read in vesselness file
     ReaderType3F::Pointer inReader = ReaderType3F::New();
     inReader->SetFileName(in_file);
     inReader->Update();
     ImageType3F::Pointer inPtr = inReader->GetOutput();

     typedef itk::GradientMagnitudeRecursiveGaussianImageFilter<ImageType3F>  DerivativeFilterType;
     DerivativeFilterType::Pointer myfilter = DerivativeFilterType::New();

     myfilter->SetInput(inPtr);
     myfilter->SetSigma(sigma);
     myfilter->Update();
     save_volume(myfilter->GetOutput(), out_file);
}
     

