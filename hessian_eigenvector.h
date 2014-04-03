#include <common.h>

int hessian_eigenvector(HessianImageType::Pointer hessianPtr,
			ImageType3F::Pointer vesselnessPtr,
			ImageTypeArray3F::Pointer eigenvectorPtr,
			HessianPar par);
int multiscale_hessian(ImageType3F::Pointer intensityPtr,
		       ImageType3F::Pointer vesselnessPtr,
		       ImageType3F::Pointer scalePtr,
		       ImageTypeArray3F::Pointer eigenvectorPtr,
		       ImageType3UC::Pointer maskPtr,
		       HessianPar par);

int mytest(HessianImageType::Pointer hessianPtr);


