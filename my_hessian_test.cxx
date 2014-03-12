#include <common.h>
#include <utility.h>
#include "hessian_eigenvector.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkHessianRecursiveGaussianImageFilter.h"
#include "hessian_eigenvector.h"
#include "itkHessianToObjectnessMeasureImageFilter.h"

namespace po = boost::program_options;

int main( int argc, char* argv[] )
{
     const unsigned int Dimension = 3;
     std::string input_file, vesselness_file, eigenvector_file, mask_file, scalemapFileName;
     double sigmaMinimum = 1.0;
     double sigmaMaximum = 10.0;
     double alpha = 0.5, beta = 1, gamma = 5;
     unsigned int numberOfSigmaSteps = 10;
     unsigned short verbose = 0;
     po::options_description mydesc("Options can only used at commandline");
     mydesc.add_options()
	  ("help,h", "Multiscale Hessian filter.")
	  ("input,i", po::value<std::string>(&input_file)->default_value("input.nii.gz"), 
	   "Input file name.")
	  ("vesselness,o", po::value<std::string>(&vesselness_file)->default_value("vesselness.nii.gz"), 
	   "Output vesselness file name.")
	  ("eigenvector,e", po::value<std::string>(&eigenvector_file)->default_value("eigenvector.nii.gz"), 
	   "Output file name.")
	  ("mask,m", po::value<std::string>(&mask_file)->default_value("mask.nii.gz"), 
	   "mask file. Must be binary.")
	  ("scalemap,c", po::value<std::string>(&scalemapFileName)->default_value("scale_map.nii.gz"), 
	   "scale map file name.")
	  ("min,n", po::value<double>(&sigmaMinimum)->default_value(1), 
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

     // read original gray intensity image.
     ReaderType3D::Pointer inReader = ReaderType3D::New();
     inReader->SetFileName(input_file);
     inReader->Update();
     ImageType3D::Pointer inPtr = inReader->GetOutput();

     // read mask file
     ReaderType3UC::Pointer maskReader = ReaderType3UC::New();
     maskReader->SetFileName(mask_file);
     maskReader->Update();
     ImageType3UC::Pointer maskPtr = maskReader->GetOutput();

     // // define Hessian filter.
     // typedef itk::HessianRecursiveGaussianImageFilter<ImageType3D >     HessianFilterType;
     // HessianFilterType::Pointer hessianFilter = HessianFilterType::New();
     // hessianFilter->SetInput(inPtr);
     // hessianFilter->SetSigma(1);
     // hessianFilter->Update();
     // hessianFilter->SetNormalizeAcrossScale(true);
     // std::cout << hessianFilter;

     // Create vesselness image buffer.
     ImageType3D::Pointer vesselnessPtr = ImageType3D::New();
     vesselnessPtr->SetRegions(inPtr->GetLargestPossibleRegion() );
     vesselnessPtr->Allocate();
     vesselnessPtr->FillBuffer(0);
     vesselnessPtr->SetSpacing(inPtr->GetSpacing());
     vesselnessPtr->SetDirection(inPtr->GetDirection());
     vesselnessPtr->SetOrigin(inPtr->GetOrigin());
     
     // create eigenvector image.
     ImageTypeArray3D::Pointer eigenvectorPtr = ImageTypeArray3D::New();
     eigenvectorPtr->SetRegions(inPtr->GetLargestPossibleRegion() );
     eigenvectorPtr->Allocate();
     eigenvectorPtr->FillBuffer(itk::NumericTraits< ImageTypeArray3D::PixelType >::Zero);

     HessianImageType::Pointer hessianPtr = HessianImageType::New();

     // hessian_eigenvector(hessianFilter->GetOutput(),
     // 			 vesselnessPtr,
     // 			 eigenvectorPtr);

     // save_volume(vesselnessPtr, vesselness_file);

     // typedef itk::HessianToObjectnessMeasureImageFilter<HessianImageType, ImageType3D> VesselnessFilterType;
     // VesselnessFilterType::Pointer vness_filter = VesselnessFilterType::New();
     // vness_filter->SetInput(hessianFilter->GetOutput() );

     // vness_filter->SetBrightObject( true);
     // vness_filter->SetScaleObjectnessMeasure( true );
     // vness_filter->SetAlpha( 0.5 );
     // vness_filter->SetBeta( 0.5 );
     // vness_filter->SetGamma( 5.0 );
     // vness_filter->Update();
     // std::cout << vness_filter;
     // save_volume(vness_filter->GetOutput(), "ref_vesselness.nii.gz");

     // test multiple scale hessian.

     // create a scale image buffer.
     ImageType3D::Pointer scalePtr = ImageType3D::New();
     scalePtr->SetRegions(inPtr->GetLargestPossibleRegion() );
     scalePtr->Allocate();
     scalePtr->FillBuffer( 0 );
     
     multiscale_hessian(inPtr,
			vesselnessPtr,
			scalePtr,
			eigenvectorPtr,
			maskPtr,
			sigmaMaximum,
			sigmaMinimum,
			numberOfSigmaSteps);
     save_volume(vesselnessPtr, vesselness_file);
     save_volume(scalePtr, scalemapFileName);
      
}


     
     


