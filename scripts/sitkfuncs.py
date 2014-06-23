import SimpleITK as sitk
import numpy as np
from scipy.stats import norm
from scipy import ndimage

def est_density(in_file, seed_file, mask_file, out_file):
    """estimate the density of each pixel according to the predefied seed region.
    
    Estimate the mean and variance of the Gaussian by using the pixels in the
    seed region, and compute the densithy of other pixels being in this
    Gaussian.
    
    in_file: input vollume of gray level image.
    seed_file: a binary image that defines the seed regions.
    mask_file: a binary volume that define the mask where we need to compute density.
    out_file: output volume image.

    """

    in_img = sitk.ReadImage(in_file)
    in_vol = sitk.GetArrayFromImage(in_img)

    seed_img = sitk.ReadImage(seed_file)
    seed_vol = sitk.GetArrayFromImage(seed_img)

    mask_img = sitk.ReadImage(mask_file)
    mask_vol = sitk.GetArrayFromImage(mask_img)

    in_vol_masked = np.ma.masked_where(seed_vol == 0, in_vol)
    mean_val = in_vol_masked.mean()
    std_val = in_vol_masked.std()

    out_vol = norm.pdf(in_vol, mean_val, std_val)
    
    # normalize to [0, 1]
    out_vol = out_vol / out_vol.max()

    # mask out the non-interesting region.
    out_vol[mask_vol == 0] = 0

    out_img = sitk.GetImageFromArray(out_vol)
    out_img.CopyInformation(in_img)
    sitk.WriteImage(out_img, out_file)
    
# def extract_lung(in_file, out_file, thresh):
#     """
#     extract the lung mask from the gray level intensity image.

#     extract_lung(in_file, out_file, low_th, high_th).
#     in_file: input gray level CT volume.
#     out_file: lung mask volume.
#     low_th, high_th: lower and higher threshold of the lung.
    
    
#     """
#     in_img = sitk.ReadImage(in_file)

#     # double threshold.
#     # out_img = sitk.BinaryThreshold(in_img, low_th, high_th)
#     out_img = sitk.DoubleThreshold(in_img, thresh[0], thresh[1], thresh[2], thresh[3])
#     sitk.WriteImage(out_img, "threshold.nii.gz")
#     # out_img = sitk.BinaryMorphologicalClosing(out_img, 5)
#     # sitk.WriteImage(out_img, "afterclosing.nii.gz")
#     # out_img = sitk.BinaryMorphologicalOpening(out_img, 5)
#     # sitk.WriteImage(out_img, "afteropenning.nii.gz")
#     # cc_img = sitk.ConnectedComponent(out_img)
    
#     sitk.WriteImage(cc_img, out_file)


def extract_comp(ct_file, gmm_file, out_file):
    """
    Extract lung component from the GMM label map.

    label_file: GMM label map file.
    out_file: output binary file for the lung area.
    """

    ct_img = sitk.ReadImage(ct_file)
    ct_vol = sitk.GetArrayFromImage(ct_img)

    gmm_img = sitk.ReadImage(gmm_file)
    gmm_vol = sitk.GetArrayFromImage(gmm_img)

    # get number of components.
    n_comp = gmm_vol.max()
    
    lung_mean = 10000
    lung_label = 0
    for k in range(1,n_comp + 1):
        n_samples = gmm_vol[gmm_vol == k].size
        this_mean = ct_vol[gmm_vol == k].sum() / n_samples
        print "comp {}, n_samples: {}, mean: {}".format(k, n_samples, this_mean)        
        if lung_mean > this_mean:
            lung_mean = this_mean
            lung_label = k
    
    # extract a binary component from the label image by using the lung_label.
    gmm_vol[gmm_vol != lung_label] = 0
    gmm_vol[gmm_vol == lung_label] = 1
    label_img = sitk.GetImageFromArray(gmm_vol)
    label_img.CopyInformation(gmm_img)

    # do the connected component.
    out_img = sitk.ConnectedComponent(label_img)

    # remove small components.
    out_img = sitk.RelabelComponent(out_img, 50000);

    # out_img.CopyInformation(label_img)
    sitk.WriteImage(out_img, out_file)


def extract_lung(comp_file, comp_ids, out_file, closing_size):
    """
    Extract the lung component(s) from multiple candidate components.

    Need to run this routine after extract_comp.
    comp_file: output of extract_comp, a set of candidate component map.
    comp_file: lung component candidate
    comp_id: A list of lung component id, as found manually.
    out_file: output binary file for the lung

    """
    comp_img = sitk.ReadImage(comp_file)
    comp_vol = sitk.GetArrayFromImage(comp_img)

    if len(comp_ids) == 1:
        comp_vol[comp_vol != comp_ids[0]] = 0
    elif len(comp_ids) == 2:
        comp_vol[np.logical_and(comp_vol != comp_ids[0], comp_vol != comp_ids[1])] = 0

    comp_vol[comp_vol > 0] = 1

    out_img = sitk.GetImageFromArray(comp_vol)
    out_img.CopyInformation(comp_img)

    # closing filter
    out_img = sitk.BinaryMorphologicalClosing(out_img, closing_size)
    sitk.WriteImage(out_img, out_file)

    

    # (cc_labels, n_labels) = ndimage.label(label_vol)
    # print 'number of connected components: {}'.format(n_labels)

    # # out_img = sitk.GetImageFromArray(label_vol)
    # # out_img.CopyInformation(label_img)
    # # sitk.WriteImage(out_img, "tmp1.nii.gz")

    # sizes = ndimage.sum(label_vol, cc_labels, range(n_labels + 1))

    # # now remove small components by thresholding size
    # mask_size = sizes < 1000
    # remove_pixel = mask_size[cc_labels]
    
    # cc_labels[remove_pixel] = 0

    # # out_img = sitk.GetImageFromArray(cc_labels)
    # # out_img.CopyInformation(label_img)
    # # sitk.WriteImage(out_img, "tmp2.nii.gz")

    # new_label_set = np.unique(cc_labels)
    # cc_labels = np.searchsorted(new_label_set, cc_labels)

    # print cc_labels.shape

    # out_img = sitk.GetImageFromArray(cc_labels)
    # out_img.CopyInformation(label_img)
    # sitk.WriteImage(out_img, out_file)
    
