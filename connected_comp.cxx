#include <common.h>
#include <utility.h>
#include "itkConnectedComponentImageFilter.h"
#include "itkLabelShapeKeepNObjectsImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include <itkBinaryClosingByReconstructionImageFilter.h>
#include "itkBinaryBallStructuringElement.h"

namespace po = boost::program_options;
int main(int argc, char* argv[])
{
     std::string in_file, out_file;
     unsigned short radius = 5, verbose = 0;

     po::options_description mydesc("Options can only used at commandline");
     mydesc.add_options()
	  ("help,h", "Mophological opening filter")
	   ("input,i", po::value<std::string>(&in_file)->default_value("input.nii.gz"), 
	    "Input binary volumge.")
	   ("output,o", po::value<std::string>(&out_file)->default_value("output.nii.gz"), 
	    "Output binary volumge.")
	  // ("radius,r", po::value<unsigned short>(&radius)->default_value(5), 
	  //  "Radius of the structure elment.")
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

     // read in vesselness file
     ReaderType3UC::Pointer inReader = ReaderType3UC::New();
     inReader->SetFileName(in_file);
     inReader->Update();
     ImageType3UC::Pointer inPtr = inReader->GetOutput();

     typedef itk::ConnectedComponentImageFilter <ImageType3UC, ImageType3DU > ConnectedComponentImageFilterType;

     ConnectedComponentImageFilterType::Pointer connected =  ConnectedComponentImageFilterType::New ();
     connected->SetInput(inPtr);
     connected->Update();

     // keep the largest component.
     std::cout << "Number of objects: " << connected->GetObjectCount() << std::endl;
 
     typedef itk::LabelShapeKeepNObjectsImageFilter< ImageType3DU > LabelShapeKeepNObjectsImageFilterType;
     LabelShapeKeepNObjectsImageFilterType::Pointer labelShapeKeepNObjectsImageFilter = LabelShapeKeepNObjectsImageFilterType::New();
     labelShapeKeepNObjectsImageFilter->SetInput( connected->GetOutput() );
     labelShapeKeepNObjectsImageFilter->SetBackgroundValue( 0 );
     labelShapeKeepNObjectsImageFilter->SetNumberOfObjects( 1 );
     labelShapeKeepNObjectsImageFilter->SetAttribute( LabelShapeKeepNObjectsImageFilterType::LabelObjectType::NUMBER_OF_PIXELS);

     save_volume(labelShapeKeepNObjectsImageFilter->GetOutput(), out_file);

     // // rescale output.
     // typedef itk::RescaleIntensityImageFilter< ImageType3DU, ImageType3UC > RescaleFilterType;
     // RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
     // rescaleFilter->SetOutputMinimum(0);
     // rescaleFilter->SetOutputMaximum(itk::NumericTraits<unsigned char>::max() );
     // std::cout << " max of unsigned char: " << itk::NumericTraits<unsigned char>::max() << std::endl;
     // printf("max of unsigned char: %i\n", itk::NumericTraits<unsigned char>::max() );
     // rescaleFilter->SetInput(labelShapeKeepNObjectsImageFilter->GetOutput());

     // save_volume(rescaleFilter->GetOutput(), out_file);

     // // closing operation. 

     // // define structure element.
     // typedef itk::BinaryBallStructuringElement<ImageType3UC::PixelType, ImageType3UC::ImageDimension>
     // 	  StructuringElementType;
     // StructuringElementType structuringElement;
     // structuringElement.SetRadius(radius);
     // structuringElement.CreateStructuringElement();

     // typedef itk::BinaryClosingByReconstructionImageFilter <ImageType3UC, StructuringElementType>  BinaryClosingByReconstructionImageFilterType;
     // BinaryClosingByReconstructionImageFilterType::Pointer closingFilter = BinaryClosingByReconstructionImageFilterType::New();
     // closingFilter->SetInput(rescaleFilter->GetOutput());
     // closingFilter->SetKernel(structuringElement);
     // closingFilter->Update();

     // // save_volume(rescaleFilter->GetOutput(), out_file);
     // save_volume(closingFilter->GetOutput(), out_file);

     return 0;
}
