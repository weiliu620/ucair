#include <common.h>
#include <utility.h>
namespace po = boost::program_options;

int main( int argc, char* argv[] )
{
     std::string input_file, output_file;
     unsigned short verbose = 0;
     po::options_description mydesc("Options can only used at commandline");
     mydesc.add_options()
	  ("help,h", "convert vector mha file to 4D nifti file")
	  ("input,i", po::value<std::string>(&input_file)->default_value("input.mha"), 
	   "Input file name. Must be mha format.")
	  ("output,o", po::value<std::string>(&output_file)->default_value("output.nii.gz"), 
	   "output file. Usually be nii.gz format.")
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

     // read original gray intensity image.
     ReaderTypeArray3D::Pointer inReader = ReaderTypeArray3D::New();
     inReader->SetFileName(input_file);
     inReader->Update();
     ImageTypeArray3D::Pointer inPtr = inReader->GetOutput();

     ImageType4D::Pointer outPtr = ImageType4D::New();
     ImageType4D::IndexType outIdx;
     outIdx.Fill(0);
     ImageType4D::SizeType outSize;
     ImageTypeArray3D::SizeType inSize = inPtr->GetLargestPossibleRegion().GetSize();
     
     outSize[0] = inSize[0];
     outSize[1] = inSize[1];
     outSize[2] = inSize[2];
     outSize[3] = 3;
     ImageType4D::RegionType outRegion;
     outRegion.SetIndex(outIdx);
     outRegion.SetSize(outSize);
     outPtr->SetRegions(outRegion);
     outPtr->Allocate();
     outPtr->FillBuffer(0);

     itk::ImageRegionIterator<ImageTypeArray3D> inIt(inPtr, inPtr->GetLargestPossibleRegion());

     ImageTypeArray3D::IndexType inIdx;
     for (inIt.GoToBegin(); !inIt.IsAtEnd(); ++ inIt) {     
	  inIdx = inIt.GetIndex();
	  outIdx[0] = inIdx[0];
	  outIdx[1] = inIdx[1];
	  outIdx[2] = inIdx[2];
	  for (outIdx[3] = 0; outIdx[3] < 3; outIdx[3] ++) {
	       outPtr->SetPixel(outIdx, inIt.Get()[outIdx[3]]);
	  }
     }

     save_volume(outPtr, output_file);
}
