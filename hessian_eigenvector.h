int hessian_eigenvector(HessianImageType::Pointer hessianPtr,
			ImageType3D::Pointer eigenvaluePtr,
			ImageTypeArray3D::Pointer eigenvectorPtr);

int multiscale_hessian(ImageType3D::Pointer intensityPtr,
		       ImageType3D::Pointer vesselnessPtr,
		       ImageType3D::Pointer scalePtr,
		       ImageTypeArray3D::Pointer eigenvectorPtr,
		       ImageType3UC::Pointer maskPtr,
		       double sigma_max,
		       double sigma_min,
		       unsigned n_steps);


