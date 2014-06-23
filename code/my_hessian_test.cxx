#include <common.h>
#include <utility.h>
#include "hessian_eigenvector.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkHessianRecursiveGaussianImageFilter.h"
#include "hessian_eigenvector.h"
#include "itkHessianToObjectnessMeasureImageFilter.h"
#include "itkJoinSeriesImageFilter.h"

namespace po = boost::program_options;

int main( int argc, char* argv[] )
{
     std::string input_file, vesselness_file, eigenvector_file, mask_file, scalemapFileName;
     HessianPar par;
     po::options_description mydesc("Options can only used at commandline");
     mydesc.add_options()
	  ("help,h", "Multiscale Hessian filter.")
	  ("input,i", po::value<std::string>(&input_file)->default_value("input.nii.gz"), 
	   "Input file name.")
	  ("vesselness,o", po::value<std::string>(&vesselness_file)->default_value("vesselness.nii.gz"), 
	   "Output vesselness file name.")
	  ("eigenvector,e", po::value<std::string>(&eigenvector_file)->default_value("eigenvector.mha"), 
	   "Output file name.")
	  ("mask,m", po::value<std::string>(&mask_file)->default_value("mask.nii.gz"), 
	   "mask file. Must be binary.")
	  ("scalemap,c", po::value<std::string>(&scalemapFileName)->default_value("scale_map.nii.gz"), 
	   "scale map file name.")

	  ("min,n", po::value<double>(&par.sigma_min)->default_value(1), 
	   "Minimal sigma")
	  ("max,x", po::value<double>(&par.sigma_max)->default_value(10), 
	   "Maxmal sigma")
	  ("steps,s", po::value<unsigned int>(&par.steps)->default_value(10), 
	   "Number of sigma steps.")

	  ("alpha,a", po::value<double>(&par.alpha)->default_value(0.5), 
	   "Alpha for Hessian filter.")
	  ("beta,b", po::value<double>(&par.beta)->default_value(1), 
	   "Beta for Hessian filter.")
	  ("gamma,g", po::value<double>(&par.gamma)->default_value(5), 
	   "Gamma for Hessian filter.")
	  ("brightobj", po::value<bool>(&par.bright_object)->default_value(true),
	   "Set this flag if the tube structure is bright compared to background.")
	   ("scaleobj", po::value<bool>(&par.scale_objectness)->default_value(true),
	    "Scale the objectness measure with the magnitude of the largest absolute eigenvalue.")

	  ("verbose,v", po::value<unsigned short>(&par.verbose)->default_value(0), 
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
     ReaderType3F::Pointer inReader = ReaderType3F::New();
     inReader->SetFileName(input_file);
     inReader->Update();
     ImageType3F::Pointer inPtr = inReader->GetOutput();

     // read mask file
     ReaderType3UC::Pointer maskReader = ReaderType3UC::New();
     maskReader->SetFileName(mask_file);
     maskReader->Update();
     ImageType3UC::Pointer maskPtr = maskReader->GetOutput();

     // Create vesselness image buffer.
     ImageType3F::Pointer vesselnessPtr = ImageType3F::New();
     vesselnessPtr->SetRegions(inPtr->GetLargestPossibleRegion() );
     vesselnessPtr->Allocate();
     // a small initial value, making sure each pixel is updated at least once,
     // and eigenvectors will be updated, too.
     vesselnessPtr->FillBuffer(-1); 
     vesselnessPtr->SetSpacing(inPtr->GetSpacing());
     vesselnessPtr->SetDirection(inPtr->GetDirection());
     vesselnessPtr->SetOrigin(inPtr->GetOrigin());
     
     // create eigenvector image.
     ImageTypeArray3F::Pointer eigenvectorPtr = ImageTypeArray3F::New();
     eigenvectorPtr->SetRegions(inPtr->GetLargestPossibleRegion() );
     eigenvectorPtr->Allocate();
     eigenvectorPtr->FillBuffer(itk::NumericTraits< ImageTypeArray3F::PixelType >::Zero);

     HessianImageType::Pointer hessianPtr = HessianImageType::New();

     // create a scale image buffer.
     ImageType3F::Pointer scalePtr = ImageType3F::New();
     scalePtr->SetRegions(inPtr->GetLargestPossibleRegion() );
     scalePtr->Allocate();
     scalePtr->FillBuffer( 0 );
     scalePtr->SetSpacing(inPtr->GetSpacing());
     scalePtr->SetDirection(inPtr->GetDirection());
     scalePtr->SetOrigin(inPtr->GetOrigin());
     

     // test multiple scale hessian.     
     multiscale_hessian(inPtr,
			vesselnessPtr,
			scalePtr,
			eigenvectorPtr,
			maskPtr,
			par);
     save_volume(vesselnessPtr, vesselness_file);
     save_volume(scalePtr, scalemapFileName);
     save_volume(eigenvectorPtr, eigenvector_file);
     return 0;
}



