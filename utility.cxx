#include <common.h>
int save_volume(ImageType3DF::Pointer ptr, std::string filename)
{

     WriterType3DF::Pointer writer = WriterType3DF::New();
	  
     writer->SetInput(ptr);
     writer->SetFileName(filename);
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

     std::cout << "save_volume(): File " << filename << " saved.\n";

     return 0;
}


int save_volume(ImageType3DU::Pointer ptr, std::string filename)
{

     WriterType3DU::Pointer writer = WriterType3DU::New();
	  
     writer->SetInput(ptr);
     writer->SetFileName(filename);
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

     std::cout << "save_volume(): File " << filename << " saved.\n";

     return 0;
}
