#include <common.h>
#include <utility.h>

int build_graph(lemon::StaticDigraph & g,
		ImageType3DC::Pointer maskPtr,
		ImageType3DU::Pointer nodemapPtr,
		const ParType & par);

int build_ijk_map(lemon::StaticDigraph & g,
		  lemon::StaticDigraph::NodeMap<itk::Index<3> > & ijkmap,
		  ImageType3DC::Pointer maskPtr,
		  ImageType3DU::Pointer nodemapPtr);

int build_cost_map(lemon::StaticDigraph & g,
		   ImageType3DF::Pointer vnessPtr,
		   lemon::StaticDigraph::NodeMap<itk::Index<3> > & ijkmap,		   
		   lemon::StaticDigraph::ArcMap<double> & costmap);

int find_target_nodes(std::set<lemon::StaticDigraph::Node> & target_set,
		      lemon::StaticDigraph & g,
		      ImageType3DC::Pointer maskPtr,
		      ImageType3DU::Pointer nodemapPtr,
		      const ParType & par);

namespace po = boost::program_options;
int main(int argc, char* argv[])
{
     std::string mask_file, vesselness_file, lungmask_file;
     ParType par;
     // program options.
     po::options_description mydesc("Options can only used at commandline");
     mydesc.add_options()
	  ("help,h", "Use Dijkstra's algorithm to find the minimal path .")
	   ("mask,m", po::value<std::string>(&mask_file)->default_value("mask.nii.gz"), 
	    "Mask file. Must be a binary file with same size as input vesselness image. voxels with zero intensity are outside of the region of interest.")
	   ("lungmask,k", po::value<std::string>(&lungmask_file)->default_value("lungmask.nii.gz"), 
	    "Lung mask file. Must be a binary file with same size as input vesselness image. voxels with zero intensity are outside of the region of interest.")
	   ("vesselness,i", po::value<std::string>(&vesselness_file)->default_value("vessel.nii.gz"), 
	    "Vesselness file. The intensity of the image indicates the vesselness of the current voxel.")
	  ("seed,s", po::value<std::vector<int> >()->multitoken(), "Source voxel coordinates. Must be in the format of: --seed i j k. ")
	  ("nbrs,b", po::value<unsigned short>(&par.n_nbrs)->default_value(6), 
	   "Number of neighbors of each voxel. Must be one of 6, 18, or 26. .")
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

     itk::Index<3> seedIdx;
     std::vector<int> seed_opt;
     if (!vm["seed"].empty() && (seed_opt = vm["seed"].as<std::vector<int> >()).size() == 3) {
	  // save the seed coordinates in a itk index. 
	  seedIdx[0] = seed_opt[0];
	  seedIdx[1] = seed_opt[1];
	  seedIdx[2] = seed_opt[2];
     }
     // read in mask file.
     ReaderType3DC::Pointer maskReader = ReaderType3DC::New();
     maskReader->SetFileName(mask_file);
     maskReader->Update();
     ImageType3DC::Pointer maskPtr = maskReader->GetOutput();

     // read in lungmask file.
     ReaderType3DC::Pointer lungmaskReader = ReaderType3DC::New();
     lungmaskReader->SetFileName(lungmask_file);
     lungmaskReader->Update();
     ImageType3DC::Pointer lungmaskPtr = lungmaskReader->GetOutput();

     // read in vesselness file
     ReaderType3DF::Pointer vnessReader = ReaderType3DF::New();
     vnessReader->SetFileName(vesselness_file);
     vnessReader->Update();
     ImageType3DF::Pointer vnessPtr = vnessReader->GetOutput();

     // define a volume to convert (i,j,k) to node id. 
     ImageType3DU::Pointer nodemapPtr = ImageType3DU::New();
     nodemapPtr->SetRegions(maskPtr->GetLargestPossibleRegion());
     nodemapPtr->Allocate();
     nodemapPtr->FillBuffer(0);

     // init a empty static graph.
     lemon::StaticDigraph g;
     
     // build the graph.
     build_graph(g, maskPtr, nodemapPtr, par);

     // Define a map to convert node to voxel ijk coordinates.
     lemon::StaticDigraph::NodeMap< itk::Index<3> > ijkmap(g);

     // build the ijkmap.
     build_ijk_map(g, ijkmap, maskPtr, nodemapPtr);

     // define the cost of the arcs and init to zero.
     CostMap costmap(g, 0);

     // build cost map based on the vesselness volume.
     build_cost_map(g, vnessPtr, ijkmap, costmap);

     // find the shortest path by Dijkstra.
     lemon::Dijkstra<lemon::StaticDigraph, CostMap> dijkstra(g, costmap);
     lemon::StaticDigraph::NodeMap<double> distmap(g);
     dijkstra.distMap(distmap);
     dijkstra.init();

     lemon::StaticDigraph::Node s = g.node( nodemapPtr->GetPixel(seedIdx) );
     dijkstra.addSource(s);
     dijkstra.start();


     // find all the target nodes.
     std::set<lemon::StaticDigraph::Node> target_set;
     find_target_nodes(target_set, g, lungmaskPtr, nodemapPtr, par);
     std::cout << "Total number of target nodes: " << target_set.size() << std::endl;

     // debug. save all target node to volume. 
     // define a volume for the accumulated path score.
     // ImageType3DU::Pointer targetnodePtr = ImageType3DU::New();
     // targetnodePtr->SetRegions(maskPtr->GetLargestPossibleRegion());
     // targetnodePtr->Allocate();
     // targetnodePtr->FillBuffer(0);
     // targetnodePtr->SetOrigin( maskPtr->GetOrigin() );
     // targetnodePtr->SetSpacing(maskPtr->GetSpacing() );
     // targetnodePtr->SetDirection(maskPtr->GetDirection() );
     // // save target node to volume.
     // for (std::set<lemon::StaticDigraph::Node>::iterator it = target_set.begin(); it != target_set.end(); ++ it) {
     // 	  targetnodePtr->SetPixel(ijkmap[*it], 1);
     // }
     // save_volume(targetnodePtr, "targetnodes.nii.gz");
     
     
     // lemon::StaticDigraph::Node t = g.node(node_id);

     // define a map to save accumulated path score. 
     lemon::StaticDigraph::NodeMap<double> scoremap(g, 0);
     lemon::Path<lemon::StaticDigraph> this_path;

     unsigned path_id = 0;
     std::set<lemon::StaticDigraph::Node>::iterator target_it;
     for (target_it = target_set.begin(); target_it != target_set.end() ; ++ target_it) {
     	  this_path = dijkstra.path(*target_it);
     	  for (lemon::Path<lemon::StaticDigraph>::ArcIt it(this_path); it != lemon::INVALID; ++ it) {
	  
     	       scoremap[g.source(it)] ++;
     	  }
	  if (path_id % 100 == 0)
	       printf("%i ", path_id);
	  path_id ++;
     }


     // define a volume for the accumulated path score.
     ImageType3DU::Pointer scorePtr = ImageType3DU::New();
     scorePtr->SetRegions(maskPtr->GetLargestPossibleRegion());
     scorePtr->Allocate();
     scorePtr->FillBuffer(0);
     scorePtr->SetOrigin( maskPtr->GetOrigin() );
     scorePtr->SetSpacing(maskPtr->GetSpacing() );
     scorePtr->SetDirection(maskPtr->GetDirection() );

     // save to volume.
     for (lemon::StaticDigraph::NodeIt nodeIt(g); nodeIt !=lemon::INVALID; ++ nodeIt) {
	  scorePtr->SetPixel(ijkmap[nodeIt], scoremap[nodeIt]);
     }
     save_volume(scorePtr, "score.nii.gz");

     // define a volume to save the accumulative cost.
     ImageType3DF::Pointer costPtr = ImageType3DF::New();
     costPtr->SetRegions(maskPtr->GetLargestPossibleRegion());
     costPtr->Allocate();
     costPtr->FillBuffer(0);
     costPtr->SetOrigin( maskPtr->GetOrigin() );
     costPtr->SetSpacing(maskPtr->GetSpacing() );
     costPtr->SetDirection(maskPtr->GetDirection() );

     // update accumulative cost volume from distmap.
     for (lemon::StaticDigraph::NodeIt nodeIt(g); nodeIt !=lemon::INVALID; ++ nodeIt) {
     	  costPtr->SetPixel(ijkmap[nodeIt], distmap[nodeIt]);
     }

     save_volume(costPtr, "cost.nii.gz");


     return 0;
}


int build_graph(lemon::StaticDigraph & g,
		ImageType3DC::Pointer maskPtr,
		ImageType3DU::Pointer nodemapPtr,
		const ParType & par)
{
     typedef itk::NeighborhoodIterator< ImageType3DC> NeighborhoodIteratorType;
     typedef itk::ConstantBoundaryCondition<ImageType3DC>  BoundaryConditionType;

     // Define neighborhood iterator on mask.
     NeighborhoodIteratorType::RadiusType radius;
     radius.Fill(1);
     NeighborhoodIteratorType maskIt(radius, maskPtr, maskPtr->GetLargestPossibleRegion() );
     BoundaryConditionType constCondition;
     constCondition.SetConstant(-1);     
     maskIt.OverrideBoundaryCondition(&constCondition);

     ImageType3DU::IndexType nodemapIdx;
     
     // xplus, xminus, yplus, yminus, zplus, zminus
     // std::array<unsigned int, 6 > neiIdxSet = {{14, 12, 16, 10, 22, 4}}; 
     unsigned int nei_set_array[] = {4, 10, 12, 14, 16, 22, // 6 neighborhood
				     1, 3, 5, 7, 9, 11, 15, 17, 19, 21, 23, 25, // 18 neighborhood
				     0, 2, 6, 8, 18, 20, 24, 26}; // 26 neighborhood

     if (par.n_nbrs != 6 && par.n_nbrs != 18 && par.n_nbrs != 26) {
	  printf("build_graph(): number of neighbors must be 6, 18, or 26. Other values may give inacruate results!\n");
	  exit(1);
     }

     IteratorType3DU nodemapIt(nodemapPtr, nodemapPtr->GetLargestPossibleRegion());

     // compute total number of nodes, and build nodemap. This must be separate
     // from building the edges below, since we need to know the nodemap in
     // order to build edges.
     unsigned n_nodes = 0;
     for (maskIt.GoToBegin(), nodemapIt.GoToBegin(); !maskIt.IsAtEnd(); ++ maskIt, ++ nodemapIt) {
	  if (maskIt.GetCenterPixel() > 0) {
	       nodemapIt.Set(n_nodes);
	       n_nodes ++;
	  }
     }

     // build edges.
     std::vector<std::pair<int,int> > arcs;
     unsigned short offset = 0;
     unsigned cur_node_id = 0, nbr_node_id = 0;
     for (maskIt.GoToBegin(), nodemapIt.GoToBegin(); !maskIt.IsAtEnd(); ++ maskIt, ++ nodemapIt) {
	  if (maskIt.GetCenterPixel() > 0) {
	       cur_node_id = nodemapIt.Get();
	       for (unsigned neiIdx = 0; neiIdx < par.n_nbrs; neiIdx ++) {
		    offset = nei_set_array[neiIdx];
		    if (maskIt.GetPixel(offset) > 0) {
			 // neighbor also in mask.
			 nodemapIdx = maskIt.GetIndex(offset);
			 nbr_node_id = nodemapPtr->GetPixel(nodemapIdx);
			 // undirected graph is represented by directed garph with two arcs.
			 arcs.push_back(std::make_pair(cur_node_id, nbr_node_id));			 
			 // arcs.push_back(std::make_pair(nbr_node_id, cur_node_id));			 
		    } // maskIt.Get
	       } // neiIdx
	  } // maskIt.GetCenter
     } // maskIt

     // build the graph. 
     g.build(n_nodes, arcs.begin(), arcs.end());     
     return 0;
}

int build_ijk_map(lemon::StaticDigraph & g,
		  lemon::StaticDigraph::NodeMap<itk::Index<3> > & ijkmap,
		  ImageType3DC::Pointer maskPtr,
		  ImageType3DU::Pointer nodemapPtr)

{
     IteratorType3DC maskIt(maskPtr, maskPtr->GetLargestPossibleRegion() );     
     IteratorType3DU nodemapIt(nodemapPtr, nodemapPtr->GetLargestPossibleRegion() );     

     // we use a push method. Given the ijk voxel coordinates, find the node, and update the ijkmap. 
     for (maskIt.GoToBegin(), nodemapIt.GoToBegin(); !maskIt.IsAtEnd(); ++ maskIt, ++ nodemapIt) {
	  if (maskIt.Get() > 0) {
	       ijkmap[g.node(nodemapIt.Get())] = maskIt.GetIndex();
	  }
     }
     return 0;
}


int build_cost_map(lemon::StaticDigraph & g,
		   ImageType3DF::Pointer vnessPtr,
		   lemon::StaticDigraph::NodeMap<itk::Index<3> > & ijkmap,		   
		   lemon::StaticDigraph::ArcMap<double> & costmap)
{
     for (lemon::StaticDigraph::ArcIt arcIt(g); arcIt != lemon::INVALID; ++ arcIt) {
	  costmap[arcIt] = exp(-(vnessPtr->GetPixel(ijkmap[g.source(arcIt)]) + vnessPtr->GetPixel(ijkmap[g.target(arcIt)]))) + 0.5;
     }
}

int find_target_nodes(std::set<lemon::StaticDigraph::Node> & target_set,
		      lemon::StaticDigraph & g,
		      ImageType3DC::Pointer maskPtr,
		      ImageType3DU::Pointer nodemapPtr,
		      const ParType & par)
{
     typedef itk::NeighborhoodIterator< ImageType3DC> NeighborhoodIteratorType;
     typedef itk::ConstantBoundaryCondition<ImageType3DC>  BoundaryConditionType;

     // Define neighborhood iterator on mask.
     NeighborhoodIteratorType::RadiusType radius;
     radius.Fill(1);
     NeighborhoodIteratorType maskIt(radius, maskPtr, maskPtr->GetLargestPossibleRegion() );
     BoundaryConditionType constCondition;
     constCondition.SetConstant(-1);     
     maskIt.OverrideBoundaryCondition(&constCondition);

     ImageType3DU::IndexType nodemapIdx;
     
     // xplus, xminus, yplus, yminus, zplus, zminus
     // std::array<unsigned int, 6 > neiIdxSet = {{14, 12, 16, 10, 22, 4}}; 
     unsigned int nei_set_array[] = {4, 10, 12, 14, 16, 22, // 6 neighborhood
				     1, 3, 5, 7, 9, 11, 15, 17, 19, 21, 23, 25, // 18 neighborhood
				     0, 2, 6, 8, 18, 20, 24, 26}; // 26 neighborhood

     if (par.n_nbrs != 6 && par.n_nbrs != 18 && par.n_nbrs != 26) {
	  printf("build_graph(): number of neighbors must be 6, 18, or 26. Other values may give inacruate results!\n");
	  exit(1);
     }

     IteratorType3DU nodemapIt(nodemapPtr, nodemapPtr->GetLargestPossibleRegion());

     // build edges.
     std::vector<std::pair<int,int> > arcs;
     unsigned short offset = 0;
     unsigned cur_node_id = 0, nbr_node_id = 0;
     ImageType3DC::SizeType maskSize = maskPtr->GetLargestPossibleRegion().GetSize();
     if (maskSize[2] == 1) { // 2D image.
	  for (maskIt.GoToBegin(), nodemapIt.GoToBegin(); !maskIt.IsAtEnd(); ++ maskIt, ++ nodemapIt) {
	       if (maskIt.GetCenterPixel() > 0) {
		    if (maskIt.GetPixel(10) <=0 || maskIt.GetPixel(12) <= 0 || maskIt.GetPixel(14) <= 0 || maskIt.GetPixel(16) <= 0) {
			 // one neighbor falls outside of mask. Current voxel
			 // must be on boundary.
			 cur_node_id = nodemapIt.Get();
			 target_set.insert(g.node(cur_node_id));
		    }
	       }
	  } // maskIt
     }

     else { // 3D volume
	  for (maskIt.GoToBegin(), nodemapIt.GoToBegin(); !maskIt.IsAtEnd(); ++ maskIt, ++ nodemapIt) {
	       if (maskIt.GetCenterPixel() > 0) {
		    unsigned neiIdx = 0;
		    bool outside_nbr = false; // one neighbor is outside mask. 
		    do {
			 // for (unsigned neiIdx = 0; neiIdx < par.n_nbrs; neiIdx ++) {
			 offset = nei_set_array[neiIdx];
			 if (maskIt.GetPixel(offset) <= 0) {
			      // tell if one neighbor is outside.
			      outside_nbr = true;
			      // std::cout << "find_target_nodes(): found " << maskIt.GetIndex() << std::endl;
			 }
			 neiIdx ++;
		    }
		    while(neiIdx < par.n_nbrs && !outside_nbr);

		    if (outside_nbr) {
			 // one neighbor falls outside of mask. Current voxel
			 // must be on boundary.
			 cur_node_id = nodemapIt.Get();
			 target_set.insert(g.node(cur_node_id));
		    }
	       }
	  } // maskIt
     }

     return 0;
}
