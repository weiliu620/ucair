#include <common.h>
#include <utility.h>
#include "itkImage.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkImageFileReader.h"
#include "itkBinaryBallStructuringElement.h"
 
int main(int argc, char *argv[])
{
  if(argc < 2)
    {
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << " InputImageFile [radius] outputImageFile" << std::endl;
    return EXIT_FAILURE;
    }
 
  unsigned int radius = 2;
  if (argc > 2)
    {
    radius = atoi(argv[2]);
    }
 
  typedef itk::Image<bool , 3>    ImageType;
  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(argv[1]);
 
  typedef itk::BinaryBallStructuringElement<
    ImageType::PixelType,3>                  StructuringElementType;
  StructuringElementType structuringElement;
  structuringElement.SetRadius(radius);
  structuringElement.CreateStructuringElement();
 
  typedef itk::BinaryDilateImageFilter <ImageType, ImageType, StructuringElementType>
          BinaryDilateImageFilterType;
 
  BinaryDilateImageFilterType::Pointer dilateFilter
          = BinaryDilateImageFilterType::New();
  dilateFilter->SetInput(reader->GetOutput());
  dilateFilter->SetKernel(structuringElement);
  dilateFilter->Update();
 
  // save_volume(dilateFilter->GetOutput(), argv[3]);
  save_volume(reader->GetOutput(), argv[3]);
  return EXIT_SUCCESS;
}
