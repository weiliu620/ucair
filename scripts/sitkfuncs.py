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

def extract_comp(gmm_file, out_file):
    """extract lung region from GMM segmentation map.

    8/28: I realize I don't have to extract one Gaussian component and run
    connected component analysis on this single Gaussian component. Instead, the
    scalar connected component can run on all Gaussian component map, i.e., the
    output component map of GMM.
    
    gmm_file: label map from GMM output.
    out_file: binary map for lung region.
    """
    
    gmm_img = sitk.ReadImage(gmm_file)
    gmm_vol = sitk.GetArrayFromImage(gmm_img)

    # do the connected component on label image instead of on binary image. Two
    # neighboring pixels are different component if their intensity is above 0.5.
    out_img = sitk.ScalarConnectedComponent(gmm_img, 0.5)
    # remove small components.
    out_img = sitk.RelabelComponent(out_img, 50000);
    sitk.WriteImage(out_img, out_file)

def extract_lung(comp_file, comp_ids, out_file, closing_size):
    """Extract the lung component(s) from multiple candidate components.

    Before running this func, user need to look at the connected component map
    and identify the label id for both left and right lungs. I know it's stupid
    but there will be a way to improve (for example, register to a template and
    get labels from the template).

    comp_file -- the connected component label map generated by extract_comp func.
    comp_ids -- a list of integers, representing the labels of the lung. Left
    and right lung may not be conntected, hence may have multiple labels.
    closing_size -- closeness filter kernel size. I choose 5.
    out_file -- output binary file for the lung.

    Need to run this routine after extract_comp.

    """
    comp_img = sitk.ReadImage(comp_file)
    comp_vol = sitk.GetArrayFromImage(comp_img)

    if len(comp_ids) == 1:
        comp_vol[comp_vol != comp_ids[0]] = 0
    elif len(comp_ids) == 2:
        comp_vol[comp_vol != comp_ids[0] and comp_vol != comp_ids[1]] = 0

    comp_vol[comp_vol > 0] = 1

    out_img = sitk.GetImageFromArray(comp_vol)
    out_img.CopyInformation(comp_img)

    # closing filter
    out_img = sitk.BinaryMorphologicalClosing(out_img, closing_size)
    sitk.WriteImage(out_img, out_file)

def extract_lung_exp(comp_file, comp_ids, out_file):
    """Extract the lung component(s) from multiple candidate components. An
experimental routine to use flood fill in place of closeness filter in order to
fill the hole.

    comp_file -- the connected component label map generated by extract_comp func.
    comp_ids -- a list of integers, representing the labels of the lung. Left
    and right lung may not be conntected, hence may have multiple labels.
    out_file -- output binary file for the lung.

    Need to run this routine after extract_comp.

    Ok it seems all the vessels are connected to the seed (0, 0, 0), so the
    flood filter does not work well to fill the hole.

    """
    comp_img = sitk.ReadImage(comp_file)
    comp_vol = sitk.GetArrayFromImage(comp_img)
    comp_vol = np.uint8(comp_vol)

    if len(comp_ids) == 1:
        comp_vol[comp_vol != comp_ids[0]] = 0
    elif len(comp_ids) == 2:
        comp_vol[np.logical_and(comp_vol != comp_ids[0], comp_vol != comp_ids[1])] = 0

    comp_vol[comp_vol > 0] = 1

    out_img = sitk.GetImageFromArray(comp_vol)
    out_img.CopyInformation(comp_img)

    # flood fill to fill the hole. The idea is from Josh Cates hold_fill.cxx
    # file.
    flood_img = sitk.ConnectedThreshold(out_img, [(0,0,0)], 0, 0, 1)
    sitk.WriteImage(flood_img, "flood.nii.gz")
    flood_img = sitk.InvertIntensity(flood_img, 1)
    sitk.WriteImage(flood_img, "inverted_flood.nii.gz")
    out_img = sitk.Or(out_img, flood_img)
    sitk.WriteImage(out_img, out_file)


    
