#include <common.h>
#include <utility.h>
#include "itkFastMarchingImageToNodePairContainerAdaptor.h"
#include "itkFastMarchingThresholdStoppingCriterion.h"
#include <itkFastMarchingUpwindGradientImageFilterBase.h>
#include <itkAddImageFilter.h>

typedef itk::FastMarchingUpwindGradientImageFilterBase< ImageType3F, ImageType3F > FastMarchingFilterType;
typedef itk::FastMarchingImageToNodePairContainerAdaptor< ImageType3F, ImageType3F, ImageType3UC > AdaptorType;
typedef FastMarchingFilterType::GradientImageType  FloatGradientImage;
typedef FloatGradientImage::PixelType   GradientPixelType;

namespace po = boost::program_options;
int main(int argc, char* argv[])
{
     std::string speed_file, seed_file, out_file, bodymask_file;
     float sigma = 0.01;
     float sigmoid_alpha = -5, sigmoid_beta = 50;
     unsigned short verbose = 0;
     float stop_time = 100;
     double const_speed = 0.01;

     po::options_description mydesc("Options can only used at commandline");
     mydesc.add_options()
	  ("help,h", "Fast Marching method with upwind gradient output.")
	  ("speed,p", po::value<std::string>(&speed_file)->default_value("speed.nii.gz"), 
	   "Speed image.")

	  ("seed,e", po::value<std::string>(&seed_file)->default_value("seed.nii.gz"), 
	   "A mask file for the seed region.")

	  ("bodymask,m", po::value<std::string>(&bodymask_file)->default_value("mask.nii.gz"), 
	   "Body mask file.")

	  ("stoptime,t", po::value<float>(&stop_time)->default_value(100), 
	   "The stop time for the Fast Marching filter.")

	  ("output,o", po::value<std::string>(&out_file)->default_value("output.nii.gz"), 
	   "Output binary volumge.")

	  // ("votemap,a", po::value<std::string>(&votemap_file)->default_value("votemap.nii.gz"), 
	  //  "The voting map of the shorest path.")

	  ("constspeed,c", po::value<double>(&const_speed)->default_value(0.01),
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

     // read in original intensity file
     ReaderType3F::Pointer speedReader = ReaderType3F::New();
     speedReader->SetFileName(speed_file);
     speedReader->Update();
     ImageType3F::Pointer speedPtr = speedReader->GetOutput();

     // read in body mask file.
     ReaderType3UC::Pointer maskReader = ReaderType3UC::New();
     maskReader->SetFileName(bodymask_file);
     maskReader->Update();
     ImageType3UC::Pointer maskPtr = maskReader->GetOutput();

     // read in seed mask file.
     ReaderType3UC::Pointer seedReader = ReaderType3UC::New();
     seedReader->SetFileName(seed_file);
     seedReader->Update();
     ImageType3UC::Pointer seedPtr = seedReader->GetOutput();

     FastMarchingFilterType::Pointer marcher = FastMarchingFilterType::New();

     // add a constant to the speed map.
     typedef itk::AddImageFilter <ImageType3F, ImageType3F, ImageType3F> AddImageFilterType;
     AddImageFilterType::Pointer addImageFilter = AddImageFilterType::New();
     addImageFilter->SetInput(speedPtr);
     addImageFilter->SetConstant2(const_speed);
     marcher->SetInput( addImageFilter->GetOutput() );

     AdaptorType::Pointer adaptor = AdaptorType::New();
     adaptor->SetIsForbiddenImageBinaryMask( true );

     // adaptor->SetAliveImage( AliveImage.GetPointer() );
     // adaptor->SetAliveValue( 0.0 );

     adaptor->SetTrialImage( seedPtr.GetPointer() );
     adaptor->SetTrialValue( 1.0 );

     adaptor->SetForbiddenImage( maskPtr.GetPointer() );
     adaptor->Update();

     marcher->SetForbiddenPoints( adaptor->GetForbiddenPoints() );
     marcher->SetAlivePoints( adaptor->GetAlivePoints() );
     marcher->SetTrialPoints( adaptor->GetTrialPoints() );

     // stop criterion.
     typedef  itk::FastMarchingThresholdStoppingCriterion< ImageType3F, ImageType3F > CriterionType;
     CriterionType::Pointer criterion = CriterionType::New();
     criterion->SetThreshold(stop_time);
     marcher->SetStoppingCriterion( criterion );

     // run the marcher.
     marcher->Update();

     // check the results
     FloatGradientImage::Pointer gradPtr = marcher->GetGradientImage();

     if (verbose >= 1) {
	  typedef itk::ImageFileWriter<FloatGradientImage> WriterType;
	  WriterType::Pointer writer = WriterType::New();
	  writer->SetInput(gradPtr);
	  writer->SetFileName("fmm_gradient.mha");
	  try { 
	       writer->Update(); 
	  } 
	  catch( itk::ExceptionObject & err ) 
	  { 
	       std::cerr << "ExceptionObject caught !" << std::endl; 
	       std::cerr << err << std::endl; 
	       return EXIT_FAILURE;
	  } 
	  std::cout << "save_volume(): File " << "fmm_gradient.mha" << " saved.\n";
     }

     save_volume(marcher->GetOutput(), out_file);

     return 0;
}


// int find_shortest_path(ImageType3UC::Pointer lungmaskPtr,
// 		       ImageType3UC::Pointer seedPtr,
// 		       FloatGradientImage::Pointer gradPtr,
// 		       std::set<ImageType3UC::IndexType> & end_points)
// {
//      typedef itk::NeighborhoodIterator< ImageType3UC> NeighborhoodIteratorType;
//      typedef itk::ConstantBoundaryCondition<ImageType3UC>  BoundaryConditionType;
//      unsigned n_nbrs = 6;

//      // Define neighborhood iterator on mask.
//      NeighborhoodIteratorType::RadiusType radius;
//      radius.Fill(1);
//      NeighborhoodIteratorType lungmaskIt(radius, lungmaskPtr, lungmaskPtr->GetLargestPossibleRegion() );
//      BoundaryConditionType constCondition;
//      constCondition.SetConstant(-1);     
//      lungmaskIt.OverrideBoundaryCondition(&constCondition);

//      // xplus, xminus, yplus, yminus, zplus, zminus
//      // std::array<unsigned int, 6 > neiIdxSet = {{14, 12, 16, 10, 22, 4}}; 
//      unsigned int nei_set_array[] = {4, 10, 12, 14, 16, 22, // 6 neighborhood
// 				     1, 3, 5, 7, 9, 11, 15, 17, 19, 21, 23, 25, // 18 neighborhood
// 				     0, 2, 6, 8, 18, 20, 24, 26}; // 26 neighborhood

//      if (n_nbrs != 6 && n_nbrs != 18 && n_nbrs != 26) {
// 	  printf("find_shortest_path(): number of neighbors must be 6, 18, or 26. Other values may give inacruate results!\n");
// 	  exit(1);
//      }

//      unsigned offset = 0, neiIdx = 0;
//      bool outside_nbr = false;
//      for (lungmaskIt.GoToBegin(); !lungmaskIt.IsAtEnd(); ++ lungmaskIt) {
// 	  if (lungmaskIt.GetCenterPixel() > 0) {
// 	       outside_nbr = false;
// 	       neiIdx = 0;
// 	       do {
// 		    offset = nei_set_array[neiIdx];
// 		    if (lungmaskIt.GetPixel(offset) <= 0) {
// 			 // tell if one neighbor is outside.
// 			 outside_nbr = true;
// 		    }
// 		    neiIdx ++;
// 	       } // do
// 	       while (neiIdx < n_nbrs && !outside_nbr);
	       
// 	       if (outside_nbr) {
// 		    // at least one neighbor is outsid of mask, so current pixel is boundary.
// 		    end_points.insert(lungmaskIt.GetIndex());
// 	       }
// 	  } // centerpixel > 0		    
//      }
// }
		       
		       
		       
		       
