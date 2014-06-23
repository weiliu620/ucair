#include <common.h>
#include <utility.h>
#include <itkExtractImageFilter.h>

namespace po = boost::program_options;
int main(int argc, char* argv[])
{
     std::string in_file, out_file;
     unsigned xstart = 0, ystart = 0, zstart = 0;
     int xsize = 0, ysize = 0, zsize = 0;
     unsigned short verbose = 0;

     po::options_description mydesc("Options can only used at commandline");
     mydesc.add_options()
	  ("help,h", "Extract a slice from a volume. ")

	  ("input,i", po::value<std::string>(&in_file)->default_value("in.nii.gz"), 
	   "input image.")
	  ("output,o", po::value<std::string>(&out_file)->default_value("out.nii.gz"), 
	   "output roi image.")

	  ("xs", po::value<unsigned>(&xstart)->default_value(0), "x start index.")
	  ("xm", po::value<int>(&xsize)->default_value(-1), "x size.")
	  ("ys", po::value<unsigned>(&ystart)->default_value(0), "y start index.")
	  ("ym", po::value<int>(&ysize)->default_value(-1), "y size.")

	  ("zs", po::value<unsigned>(&zstart)->default_value(0), "z start index.")
	  ("zm", po::value<int>(&zsize)->default_value(-1), "z size.")

	  ("verbose,v", po::value<unsigned short>(&verbose)->default_value(0), 
	   "verbose level. 0 for minimal output. 3 for most output.");

     po::variables_map vm;        
     po::store(po::parse_command_line(argc, argv, mydesc), vm);
     po::notify(vm);    

     try {
	  if ( (vm.count("help")) | (argc == 1) ) {
	       std::cout << "Usage: find_path [options]\n";
	       std::cout << mydesc << "\n";
	       return 0;
	  }
     }
     catch(std::exception& e) {
	  std::cout << e.what() << "\n";
	  return 1;
     }    


     // read input file.
     ReaderType3F::Pointer inReader = ReaderType3F::New();
     inReader->SetFileName(in_file);
     inReader->Update();
     ImageType3F::Pointer inPtr = inReader->GetOutput();

     ImageType3F::IndexType desiredStart;
     ImageType3F::SizeType desiredSize = inPtr->GetLargestPossibleRegion().GetSize();
     desiredStart[0] = xstart;
     desiredStart[1] = ystart;
     desiredStart[2] = zstart;

     if (xsize == 1) {
	  desiredSize[0] = 0;
     }
     else if (xsize == -1) {
     }
     else {
	  desiredSize[0] = xsize;
     }

     if (ysize == 1) {
	  desiredSize[1] = 0;
     }
     else if (ysize == -1) {
     }
     else {
	  desiredSize[1] = ysize;
     }


     if (zsize == 1) {
	  desiredSize[2] = 0;
     }
     else if (zsize == -1) {
     }
     else {
	  desiredSize[2] = zsize;
     }

     typedef itk::ExtractImageFilter< ImageType3F, ImageType2DF > FilterType;

     FilterType::Pointer filter = FilterType::New();
     ImageType3F::RegionType desiredRegion(desiredStart, desiredSize);
     desiredRegion.GetImageDimension();

     filter->SetExtractionRegion(desiredRegion);
     filter->SetInput(inPtr);
     filter->SetDirectionCollapseToIdentity(); // This is required.
     // filter->SetDirectionCollapseToSubmatrix();


     WriterType2DF::Pointer writer = WriterType2DF::New();
	  
     writer->SetInput(filter->GetOutput());
     writer->SetFileName(out_file);
     try 
     { 
	  writer->Update(); 
     } 
     catch( itk::ExceptionObject & err ) 
     { 
	  std::cerr << "ExceptionObject caught !" << std::endl; 
	  std::cerr << err << std::endl; 
	  return EXIT_FAILURE;
     } 

     std::cout << "save_volume(): File " << out_file << " saved.\n";

     return 0;
}




