#include <common.h>
#include <utility.h>

namespace po = boost::program_options;
int main(int argc, char* argv[])
{
     std::string in_file, out_file, mask_file;
     unsigned short verbose = 0;
     unsigned max = 100;
     po::options_description mydesc("Options can only used at commandline");
     mydesc.add_options()
	  ("help,h", "Inverse a distance map.")
	   ("input,i", po::value<std::string>(&in_file)->default_value("input.nii.gz"), 
	    "Input distance map volume")

	   ("mask,m", po::value<std::string>(&mask_file)->default_value("mask.nii.gz"), 
	    "body mask volume file")

	   ("output,o", po::value<std::string>(&out_file)->default_value("output.nii.gz"), 
	    "Output heat map volume")

	  ("max,x", po::value<unsigned>(&max)->default_value(100), 
	   "max value of the output map, i.e. max distance travelled by the Fast Marching method.")

	  ("verbose,v", po::value<unsigned short>(&verbose)->default_value(0), 
	   "verbose level. 0 for minimal output. 3 for most output.");

     po::variables_map vm;        
     po::store(po::parse_command_line(argc, argv, mydesc), vm);
     po::notify(vm);    

     try {
	  if ( (vm.count("help")) | (argc == 1) ) {
	       std::cout << "Usage: inverse_distmap [options]\n";
	       std::cout << mydesc << "\n";
	       return 0;
	  }
     }
     catch(std::exception& e) {
	  std::cout << e.what() << "\n";
	  return 1;
     }    

     // read in distance file
     ReaderType3D::Pointer inReader = ReaderType3D::New();
     inReader->SetFileName(in_file);
     inReader->Update();
     ImageType3D::Pointer inPtr = inReader->GetOutput();
     itk::ImageRegionIterator<ImageType3D> inIt(inPtr, inPtr->GetLargestPossibleRegion());

     // read in mask file.
     ReaderType3UC::Pointer maskReader = ReaderType3UC::New();
     maskReader->SetFileName(mask_file);
     maskReader->Update();
     ImageType3UC::Pointer maskPtr = maskReader->GetOutput();
     itk::ImageRegionIterator<ImageType3UC> maskIt(maskPtr, maskPtr->GetLargestPossibleRegion() );

     for (inIt.GoToBegin(), maskIt.GoToBegin(); !maskIt.IsAtEnd(); ++ inIt, ++ maskIt) {
	  if (maskIt.Get() > 0) {
	       if (inIt.Get() < max) {
		    inIt.Set(max - inIt.Get());
	       }
	       else {
		    inIt.Set(0);
	       }
	  }
	  else {
	       inIt.Set(0);
	  }
     } // inIt

     save_volume(inPtr, out_file);

     return 0;
}

