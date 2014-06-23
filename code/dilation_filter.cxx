#include <common.h>
#include <utility.h>
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkSubtractImageFilter.h"

namespace po = boost::program_options;
int main(int argc, char* argv[])
{
     std::string in_file, out_file, diff_file;
     unsigned short radius = 5, verbose = 0;
     bool diff = false;

     po::options_description mydesc("Options can only used at commandline");
     mydesc.add_options()
	  ("help,h", "Mophological closing filter")
	   ("input,i", po::value<std::string>(&in_file)->default_value("input.nii.gz"), 
	    "Input binary volumge.")
	   ("output,o", po::value<std::string>(&out_file)->default_value("output.nii.gz"), 
	    "Output binary volumge.")
	  ("radius,r", po::value<unsigned short>(&radius)->default_value(5), 
	   "Radius of the structure elment.")
	  ("diff,d", po::bool_switch(&diff),
	   "Whether print the different of the intput and output images.")       
	   ("diffout,t", po::value<std::string>(&diff_file)->default_value("diff.nii.gz"), 
	    "The difference volume between input and output.")

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
     ReaderType3UC::Pointer inReader = ReaderType3UC::New();
     inReader->SetFileName(in_file);
     inReader->Update();
     ImageType3UC::Pointer inPtr = inReader->GetOutput();


     typedef itk::BinaryBallStructuringElement<ImageType3UC::PixelType, ImageType3UC::ImageDimension>
	  StructuringElementType;
     StructuringElementType structuringElement;
     structuringElement.SetRadius(radius);
     std::cout << "radius: " << radius << std::endl;
     structuringElement.CreateStructuringElement();

     typedef itk::BinaryDilateImageFilter <ImageType3UC, ImageType3UC, StructuringElementType>  FilterType;
     FilterType::Pointer myFilter = FilterType::New();
     myFilter->SetInput(inPtr);
     myFilter->SetKernel(structuringElement);
     myFilter->Update();

     save_volume(myFilter->GetOutput(), out_file);

     // print
     if (diff) {
	  typedef itk::SubtractImageFilter<ImageType3UC> SubtractType;
	  SubtractType::Pointer diff = SubtractType::New();
	  diff->SetInput1(inPtr);
	  diff->SetInput2(myFilter->GetOutput());
	  save_volume(diff->GetOutput(), diff_file);
     }

     return 0;
}
