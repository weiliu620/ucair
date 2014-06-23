import os
import subprocess
import fnmatch
import sys
import time
import nibabel as nib
import numpy as np

def convert_dicom(in_dir, out_dir, n_cores):
    """
    Take all the sub-dirs in in_dir as dicom dir, and convet each sub_dir into a nifti file.
    
    in_dir: the input dir which contains the dicom subdirs. 
    out_dir: the output dir which contains the output nifti files.
    n_cores: the number of cpu cores available for conversion. The script use a pool of size n_cores for parallel converting multiple files. 
    """

    all_dicom_dirs = [d for d in os.listdir(in_dir) if fnmatch.fnmatch(d, 'PE*')]
    all_dicom_dirs.sort()

    proc_set = set()
    while True:
        new_proc_set = proc_set.copy()
        for p in proc_set:
            if p.poll() is not None:
                (mystdout, mystderr) = p.communicate()
                # print mystdout
                # print mystderr
                new_proc_set.remove(p)
    
        proc_set = new_proc_set.copy()

        while len(proc_set) < n_cores and len(all_dicom_dirs) > 0:
            this_PE = all_dicom_dirs.pop(0)
            this_PE_dir = os.path.join(in_dir, this_PE)

            matched_dir = [d for d in os.listdir(this_PE_dir) if fnmatch.fnmatch(d, 'Exam*')]
            if matched_dir:
                exam_dir = os.path.join(this_PE_dir, matched_dir[0])
                if os.path.exists(os.path.join(exam_dir, 'Ser_000004')):
                    dicom_dir = os.path.join(exam_dir, 'Ser_000004')
                elif os.path.exists(os.path.join(exam_dir, 'Ser_000002')):
                    dicom_dir = os.path.join(exam_dir, 'Ser_000002')
                else:
                    dicom_dir = exam_dir

                dicom_files = os.listdir(dicom_dir)
                (this_name, this_ext) = os.path.splitext(dicom_files[0])
                if (this_ext == '.dcm') or  (this_ext == '.dcm '):
                    print 'working on {}'.format(dicom_dir)
                    proc_set.add(subprocess.Popen(['/scratch/packages/itk_apps/ConvertBetweenFileFormats/ConvertBetweenFileFormats', dicom_dir, os.path.join(out_dir, this_PE + '.nii.gz')], stdout = subprocess.PIPE))
                else:
                    print '{} does not have .dcm. {} need manual conversion'.format(dicom_files[0], dicom_dir)

            else:
                print '{} abnormal. need manual conversion'.format(this_PE_dir)
                              
        if len(proc_set) == 0:
            break
          
        time.sleep(1)
        

def extract_lung(in_image, out_image, low_th, high_th):
    """
    Given a gray level image, extract the lung mask.

    in_image: input lung CT image.
    out_imag: output lung mask volume.
    low_th: lower threshold. Voxels below this value are believed not in the lung. Choose -900 for CTA image.
    high_th: higher threshod. Voxels above this value are believed not in the lung. Choose -360 for CTA image.
    """

    bin_dir = '/home/weiliu/projects/vessel/build/'
    subprocess.call(['fslmaths', '-dt', 'int', in_image, '-thr', low_th, out_image, '-odt', 'int'])
    subprocess.call(['fslmaths', '-dt', 'int', out_image, '-uthr', high_th, out_image, '-odt', 'int'])
    subprocess.call(['fslmaths', '-dt', 'int', out_image, '-abs', out_image, '-odt', 'int'])

    subprocess.call(['fslmaths', out_image, '-bin', out_image, '-odt', 'char'])
    # multiple by 255 so the morphology filter can handle it.
    subprocess.call(['fslmaths', out_image, '-mul', '255', out_image])

    # morphology closing
    subprocess.call([os.path.join(bin_dir, 'closing_filter'), '-i', out_image, '-o', out_image, '-r', '5'])

    # morphology opening
    subprocess.call([os.path.join(bin_dir, 'opening_filter'), '-i', out_image, '-o', out_image, '-r', '5'])
    # find the largest component.
    subprocess.call([os.path.join(bin_dir, 'connected_comp'), '-i', out_image, '-o', out_image])


def extract_body(in_image, out_image, th):
    """
    extract a foreground body mask.
    
    in_image: input lung CT volume.
    otu_image: output mask of the body.
    th: threshold. Voxels above it will be assumed body.
    
    """
    
    bin_dir = '/home/weiliu/projects/vessel/build/'
    subprocess.call(['fslmaths', '-dt', 'int', in_image, '-uthr', th, out_image, '-odt', 'int'])

    # add one, so body will be one, and backgrounds are still negative. 
    subprocess.call(['fslmaths', '-dt', 'int', out_image, '-add', '1', out_image])

    # binary image. Body will remain one, background will be zero. 
    subprocess.call(['fslmaths', '-dt', 'int', out_image, '-bin', out_image, '-odt', 'int'])

    # lung still outside of body, add it in by filling hole.
    subprocess.call([os.path.join(bin_dir, 'fillhole_filter'), '-i', out_image, '-o', out_image])

    # morphology closing, to remove small regions connected to the main body.
    subprocess.call([os.path.join(bin_dir, 'closing_filter'), '-i', out_image, '-o', out_image, '-r', '5'])

    # # morphology opening
    # subprocess.call([os.path.join(bin_dir, 'opening_filter'), '-i', out_image, '-o', out_image, '-r', '5'])

    # find the largest component.
    subprocess.call([os.path.join(bin_dir, 'connected_comp'), '-i', out_image, '-o', out_image])

def invert_dist_map(in_dist_file, lungmask_file, out_file, out_max):
    """
    Invert a distance file output by the Fast Marching method.

    invert_dist_map(in_dist_file, out_file, out_max):

    After inversion, vessel looks bright, background looks dark. Largest value will be the longest time travelled. 
    """

    I = nib.load(in_dist_file)
    D = I.get_data()

    # take the min of inptu and max value, so the image no more than max value.
    D2 = np.minimum(out_max, D)
    D2 = out_max - D2

    # masking
    lungmask = nib.load(lungmask_file)
    lungmask_data = lungmask.get_data().astype(bool)
    D2[~lungmask_data] = 0

    I_out = nib.Nifti1Image(D2, affine = I.get_affine(), header = I.get_header())
    I_out.to_filename(out_file)
    
