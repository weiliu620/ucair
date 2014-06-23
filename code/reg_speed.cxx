#include <common.h>
#include <utility.h>
#include <itkMultiplyImageFilter.h>
#include <itkAddImageFilter.h>
namespace po = boost::program_options;
int main(int argc, char* argv[])
{
     std::string speed_file, out_file;
     unsigned short verbose = 0;
     double alpha = 0.01;

     po::options_description mydesc("Options can only used at commandline");
     mydesc.add_options()
	  ("help,h", "Regularize speed map by alpha * 1 + (1-alpha) * Speed.")
	  ("speed,p", po::value<std::string>(&speed_file)->default_value("speed.nii.gz"), 
	   "Speed image.")

	  ("output,o", po::value<std::string>(&out_file)->default_value("output.nii.gz"), 
	   "Output binary volumge.")

	  ("alpha,a", po::value<double>(&alpha)->default_value(0.01),
	   "constant spped added to the input speed map. Too small values results in more propagation time. Too big value overwhelms true speed map.")

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
     ImageType3F::Pointer speedPtr = speedReader->GetOutput();

     // add a constant to the speed map.
     typedef itk::MultiplyImageFilter <ImageType3F, ImageType3F, ImageType3F> MultiplyImageFilterType;
     MultiplyImageFilterType::Pointer mulImageFilter = MultiplyImageFilterType::New();
     mulImageFilter->SetInput(speedPtr);
     mulImageFilter->SetConstant2(1- alpha);
     mulImageFilter->ReleaseDataFlagOn();

     // add a constant to the speed map.
     typedef itk::AddImageFilter <ImageType3F, ImageType3F, ImageType3F> AddImageFilterType;
     AddImageFilterType::Pointer addImageFilter = AddImageFilterType::New();
     addImageFilter->SetInput(mulImageFilter->GetOutput());
     addImageFilter->SetConstant2(alpha);

     save_volume(addImageFilter->GetOutput(), out_file);
}



