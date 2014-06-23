#include <common.h>
#include <iomanip>
#include <utility.h>

namespace po = boost::program_options;

int main( int argc, char* argv[] )
{
     std::string input_file, seg_file, mask_file;
     unsigned n_comp = 5, maxit = 50;
     unsigned short verbose = 0;
     po::options_description mydesc("Because of the need of negative number as arguments, there is no short form of argument in this code.");
     mydesc.add_options()
	  ("help,h", "extract lung from a connected component map.")
	  ("seg,s", po::value<std::string>(&seg_file)->default_value("seg.nii.gz"), 
	   "Label map geneateted by GMM.")

	  ("ct,t", po::value<std::string>(&ct_file)->default_value("ct.nii.gz"), 
	   "CT gray level image file")

	  ("output,o", po::value<std::string>(&lun_file)->default_value("lung.nii.gz"), 
	   "Output segmentation binary file.")

	  ("ncomp", po::value<unsigned >(&n_comp)->default_value(5), 
	   "Number of components, or classes.")

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

