#ifndef __COMMON_H__ 
#define __COMMON_H__

#include <new>
#include <iostream>
#include <iterator>
#include <string>

#include <itkNiftiImageIO.h>
#include <itkMetaImageIO.h>
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkNeighborhoodIterator.h"
#include "itkConstantBoundaryCondition.h"
#include <vcl_iostream.h>
#include <vnl/vnl_matlab_print.h>

#include <lemon/list_graph.h>
#include <lemon/smart_graph.h>
#include <lemon/static_graph.h>
#include <lemon/adaptors.h>
#include <lemon/connectivity.h>
#include <lemon/dijkstra.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/errors.hpp>
#include <boost/program_options/option.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/version.hpp>

typedef itk::Image<float, 2> ImageType2DF;

typedef itk::Image<unsigned char, 2> ImageType2UC;
typedef itk::Image<unsigned, 3> ImageType3DU;
typedef itk::Image<short int, 3> ImageType3DS;
typedef itk::Image<int, 3> ImageType3DI;
typedef itk::Image<unsigned, 3> ImageType3U;
typedef itk::Image<char, 3> ImageType3DC;
typedef itk::Image<unsigned char, 3> ImageType3DUC;
typedef itk::Image<unsigned char, 3> ImageType3UC;
typedef itk::Image<float, 3> ImageType3DF;
typedef itk::Image<bool, 3> ImageType3B;
typedef itk::Image<double, 3> ImageType3D;
typedef itk::Image<float, 3> ImageType3F;
typedef itk::FixedArray<double, 3> Array3D; 
typedef itk::FixedArray<float, 3> Array3F;
typedef itk::Image<Array3D, 3> ImageTypeArray3D;
typedef itk::Image<Array3F, 3> ImageTypeArray3F;
typedef itk::SymmetricSecondRankTensor< float, 3 > HessianPixelType;
typedef itk::Image< HessianPixelType, 3 >           HessianImageType;
typedef itk::Image<double, 4> ImageType4D;


typedef itk::ImageFileReader< ImageType2DF >  ReaderType2D;
typedef itk::ImageFileReader< ImageType2UC >  ReaderType2UC;
typedef itk::ImageFileReader< ImageType3D >  ReaderType3D;
typedef itk::ImageFileReader< ImageType3F >  ReaderType3F;
typedef itk::ImageFileReader< ImageType3DS >  ReaderType3DS;
typedef itk::ImageFileReader< ImageType3DF >  ReaderType3DF;
typedef itk::ImageFileReader< ImageType3DI >  ReaderType3DI;
typedef itk::ImageFileReader< ImageType3DC >  ReaderType3DC;
typedef itk::ImageFileReader< ImageType3UC >  ReaderType3UC;
typedef itk::ImageFileReader< ImageType3DU >  ReaderType3DU;
typedef itk::ImageFileReader< ImageType3B >  ReaderType3B;
typedef itk::ImageFileReader< ImageType3U >  ReaderType3U;
typedef itk::ImageFileReader< ImageTypeArray3D >  ReaderTypeArray3D;
typedef itk::ImageFileReader< ImageTypeArray3F >  ReaderTypeArray3F;

typedef itk::ImageFileWriter< ImageType2DF >  WriterType2DF;
typedef itk::ImageFileWriter< ImageType2UC >  WriterType2UC;
typedef itk::ImageFileWriter< ImageType3DS >  WriterType3DS;
typedef itk::ImageFileWriter< ImageType3DI >  WriterType3DI;
typedef itk::ImageFileWriter< ImageType3DF >  WriterType3DF;
typedef itk::ImageFileWriter< ImageType3DC >  WriterType3DC;
typedef itk::ImageFileWriter< ImageType3UC >  WriterType3UC;
typedef itk::ImageFileWriter< ImageType3DUC >  WriterType3DUC;
typedef itk::ImageFileWriter< ImageType3DU >  WriterType3DU;
typedef itk::ImageFileWriter< ImageType3B >  WriterType3B;
typedef itk::ImageFileWriter< ImageType3D >  WriterType3D;
typedef itk::ImageFileWriter< ImageType3F >  WriterType3F;
typedef itk::ImageFileWriter< ImageTypeArray3D >  WriterTypeArray3D;
typedef itk::ImageFileWriter< ImageTypeArray3F >  WriterTypeArray3F;
typedef itk::ImageFileWriter< ImageType4D >  WriterType4D;

typedef itk::ImageRegionConstIterator< ImageType2DF > ConstIteratorType2DF;
typedef itk::ImageRegionConstIterator< ImageType3DF > ConstIteratorType3DF;
typedef itk::ImageRegionConstIterator< ImageType3DC > ConstIteratorType3DC;

typedef itk::ImageRegionIterator< ImageType2DF>       IteratorType2DF;
typedef itk::ImageRegionIterator< ImageType3DF>       IteratorType3DF;
typedef itk::ImageRegionIterator< ImageType3DC>       IteratorType3DC;
typedef itk::ImageRegionIterator< ImageType3DI>       IteratorType3DI;
typedef itk::ImageRegionIterator< ImageType3DU>       IteratorType3DU;
typedef itk::ImageRegionIterator< ImageType3DS>       IteratorType3DS;
typedef itk::ImageRegionIterator< ImageType3F>       IteratorType3F;
typedef itk::ImageRegionIterator< ImageType3UC>       IteratorType3UC;

typedef lemon::StaticDigraph::ArcMap<double> CostMap;


#define EPS 1e-6
#define PI 3.14159265

struct ParType
{
     unsigned short verbose; 
     unsigned short n_nbrs;
};

struct HessianPar{
     double alpha;
     double beta;
     double gamma;
     bool bright_object;
     bool scale_objectness;
     double sigma_min;
     double sigma_max;
     unsigned steps;
     unsigned short verbose;
};

#endif 
