#include <common.h>
#include <utility.h>
#include <itkFastMarchingUpwindGradientImageFilterBase.h>
#include <itkLabelContourImageFilter.h>  
#include "itkBinaryImageToLabelMapFilter.h"

typedef itk::FastMarchingUpwindGradientImageFilterBase< ImageType3F, ImageType3F > FastMarchingFilterType;

typedef FastMarchingFilterType::GradientImageType  FloatGradientImage;
typedef FloatGradientImage::PixelType   GradientPixelType;


namespace po = boost::program_options;
int main(int argc, char* argv[])
{
     std::string seed_file, grad_file, lungmask_file, votemap_file;
     unsigned short verbose = 0;

     po::options_description mydesc("Options can only used at commandline");
     mydesc.add_options()
	  ("help,h", "Vote on the minimal path")

	  ("seed,e", po::value<std::string>(&seed_file)->default_value("seed.nii.gz"), 
	   "A mask file for the seed region.")

	  ("lungmask,u", po::value<std::string>(&lungmask_file)->default_value("lungmask.nii.gz"), 
	   "Lung mask file.")

	  ("grad,g", po::value<std::string>(&grad_file)->default_value("gradient.nii.gz"), 
	   "gradient file.")

	  ("votemap,a", po::value<std::string>(&votemap_file)->default_value("votemap.nii.gz"), 
	   "The voting map of the shorest path.")

	  ("verbose,v", po::value<unsigned short>(&verbose)->default_value(0), 
	   "verbose level. 0 for minimal output. 3 for most output.");

     po::variables_map vm;        
     po::store(po::parse_command_line(argc, argv, mydesc), vm);
     po::notify(vm);    

     try {
	  if ( (vm.count("help")) | (argc == 1) ) {
	       std::cout << "Usage: find_path [options]\n";
	       std::cout << mydesc << "\n";
	       return 0;
	  }
     }
     catch(std::exception& e) {
	  std::cout << e.what() << "\n";
	  return 1;
     }    

     // read in lungmask file.
     ReaderType3UC::Pointer lungmaskReader = ReaderType3UC::New();
     lungmaskReader->SetFileName(lungmask_file);
     lungmaskReader->Update();
     ImageType3UC::Pointer lungmaskPtr = lungmaskReader->GetOutput();

     // read in seed mask file.
     ReaderType3UC::Pointer seedReader = ReaderType3UC::New();
     seedReader->SetFileName(seed_file);
     seedReader->Update();
     ImageType3UC::Pointer seedPtr = seedReader->GetOutput();

     // read gradient file
     typedef itk::ImageFileReader<FloatGradientImage> GradReader;
     GradReader::Pointer gradReader = GradReader::New();
     gradReader->SetFileName(grad_file);
     gradReader->Update();
     FloatGradientImage::Pointer gradPtr = gradReader->GetOutput();

     std::vector<ImageType3UC::IndexType> end_points;
     // convert lung mask to contour.
     typedef itk::LabelContourImageFilter<ImageType3UC, ImageType3UC> LabelContourImageFilterType;
     LabelContourImageFilterType::Pointer labelContourImageFilter = LabelContourImageFilterType::New();
     labelContourImageFilter->SetInput(lungmaskPtr);
     labelContourImageFilter->SetFullyConnected(false);
     save_volume(labelContourImageFilter->GetOutput(), "contour.nii.gz");

     typedef itk::BinaryImageToLabelMapFilter<ImageType3UC> BinaryImageToLabelMapFilterType;
     BinaryImageToLabelMapFilterType::Pointer binaryImageToLabelMapFilter = BinaryImageToLabelMapFilterType::New();
     binaryImageToLabelMapFilter->SetInput(labelContourImageFilter->GetOutput());
     binaryImageToLabelMapFilter->SetFullyConnected(true);
     binaryImageToLabelMapFilter->SetInputForegroundValue(1);
     binaryImageToLabelMapFilter->Update();

     if (verbose >= 0) {
	  std::cout << "There are " << binaryImageToLabelMapFilter->GetOutput()->GetNumberOfLabelObjects() << " objects." << std::endl;
     }

     // create a output votemap buffer.
     ImageType3U::Pointer votemapPtr = ImageType3U::New();
     votemapPtr->SetRegions(lungmaskPtr->GetLargestPossibleRegion());
     votemapPtr->SetSpacing(lungmaskPtr->GetSpacing());
     votemapPtr->SetOrigin(lungmaskPtr->GetOrigin());
     votemapPtr->SetDirection(lungmaskPtr->GetDirection());
     votemapPtr->Allocate();
     votemapPtr->FillBuffer(0);

     // loop over each region to locate ending points on the surface. We don't
     // care how many label objects (the surface voxels may consist multiple
     // objects, deteced by the filter). We just count all pixels in all objects
     // as boundary.
     FloatGradientImage::PixelType grad;
     ImageType3UC::IndexType curIdx, newIdx;
     float delta = 0.01;

     std::vector<itk::Offset<3> > neighbor_offsets(6);
     neighbor_offsets[0].Fill(0);
     neighbor_offsets[0][0] = -1; // {-1, 0, 0}

     neighbor_offsets[1].Fill(0);
     neighbor_offsets[1][0] = 1; // {1, 0, 0}

     neighbor_offsets[2].Fill(0);
     neighbor_offsets[2][1] = -1; // {0, -1, 0}

     neighbor_offsets[3].Fill(0);
     neighbor_offsets[3][1] = 1; // {0, 1, 0}

     neighbor_offsets[4].Fill(0);
     neighbor_offsets[4][2] = -1; // {0, 0, -1}

     neighbor_offsets[5].Fill(0);
     neighbor_offsets[5][2] = 1; // {0, 0, 1}
     
     unsigned best_offset_id = 0;
     double best_cos_value = 1, cur_cos_value = 0; // cosine angle btw gradient and offset vector.x
     for(unsigned int i = 0; i < binaryImageToLabelMapFilter->GetOutput()->GetNumberOfLabelObjects(); i++) {
     	  BinaryImageToLabelMapFilterType::OutputImageType::LabelObjectType* labelObject = binaryImageToLabelMapFilter->GetOutput()->GetNthLabelObject(i);
     	  for(unsigned int n = 0; n < labelObject->Size(); n ++) {
     	       // working on this end point.
     	       curIdx =  labelObject->GetIndex(n);
     	       if (verbose >= 2) {
     		    std::cout << "working on: " << curIdx << std::endl;
     	       }

	       // some end points are not reached by the FMM front end,
	       // hence has zero gradient. Ignore them.
	       grad = gradPtr->GetPixel(curIdx);	       
     	       while(seedPtr->GetPixel(curIdx) == 0 && grad.GetNorm() > 0) {
		    grad = grad / grad.GetNorm(); // normalize to unit vector.

		    // compute the offsets that match the gradient best.
		    best_offset_id = 0;
		    best_cos_value = 1; // a worse value so anyone can beat it.
		    for (unsigned s = 0; s < neighbor_offsets.size(); s ++) {
			 cur_cos_value = grad[0] * neighbor_offsets[s][0]
			      + grad[1] * neighbor_offsets[s][1]
			      + grad[2] * neighbor_offsets[s][2];
			 if (cur_cos_value < best_cos_value) {
			      best_cos_value = cur_cos_value;
			      best_offset_id = s;
			 }
		    } // for

		    // move to the new voxel.
		    curIdx = curIdx + neighbor_offsets[best_offset_id];
		    votemapPtr->SetPixel(curIdx, votemapPtr->GetPixel(curIdx) + 1);
		    grad = gradPtr->GetPixel(curIdx);
	       }

	       if (grad.GetNorm() == 0 && verbose >= 3) {
		    std::cout << curIdx << "norm is zero.\n";
	       }

     	  } // n
     } // object i.

     save_volume(votemapPtr, "votemap.nii.gz");
}
