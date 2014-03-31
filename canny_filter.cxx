#include <common.h>
#include <utility.h>
#include "itkCannyEdgeDetectionImageFilter.h"
namespace po = boost::program_options;
int main(int argc, char* argv[])
{
     std::string in_file, out_file;
     unsigned short variance = 2, upper_th = 0, lower_th = 0, verbose = 0;

     po::options_description mydesc("Options can only used at commandline");
     mydesc.add_options()
	  ("help,h", "Canny filter outputs the edge map.")
	  ("input,i", po::value<std::string>(&in_file)->default_value("input.nii.gz"), 
	   "Input binary volumge.")
	  ("output,o", po::value<std::string>(&out_file)->default_value("output.nii.gz"), 
	   "Output binary volumge.")
	  ("variance,s", po::value<unsigned short>(&variance)->default_value(2), 
	   "Variance.")
	  ("upper,u", po::value<unsigned short>(&upper_th)->default_value(2), 
	   "Upper threshold.")
	  ("lower,l", po::value<unsigned short>(&lower_th)->default_value(1), 
	   "Lower threshold.")
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

     typedef itk::CannyEdgeDetectionImageFilter<ImageType3F, ImageType3F>  MyFilterType;
     MyFilterType::Pointer myfilter = MyFilterType::New();

     myfilter->SetInput(inPtr);
     myfilter->SetVariance(variance);
     myfilter->SetUpperThreshold( upper_th);
     myfilter->SetLowerThreshold( lower_th );
     myfilter->Update();
     save_volume(myfilter->GetOutput(), out_file);
}
     

