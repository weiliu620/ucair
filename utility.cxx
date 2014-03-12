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

int save_volume(ImageType3DI::Pointer ptr, std::string filename)
{

     WriterType3DI::Pointer writer = WriterType3DI::New();
	  
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


int save_volume(ImageType3DC::Pointer ptr, std::string filename)
{

     WriterType3DC::Pointer writer = WriterType3DC::New();
	  
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

int save_volume(ImageType3DUC::Pointer ptr, std::string filename)
{

     WriterType3DUC::Pointer writer = WriterType3DUC::New();
	  
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

int save_volume(ImageType3B::Pointer ptr, std::string filename)
{

     WriterType3B::Pointer writer = WriterType3B::New();
	  
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

int save_volume(ImageType2UC::Pointer ptr, std::string filename)
{

     WriterType2UC::Pointer writer = WriterType2UC::New();
	  
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
int save_volume(ImageType3D::Pointer ptr, std::string filename)
{

     WriterType3D::Pointer writer = WriterType3D::New();
	  
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
