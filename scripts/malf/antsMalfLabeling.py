import os
import subprocess
import fnmatch
import sys
import numpy
import multiprocessing

def reg(atlas_dir, label_dir, target_image, out_dir):
    """
    Register all the grascale atlas image to the target image, and apply the same transform to the label images.

    atlas_dir: the dir that contains all grayscale atlas.
    label_dir: the dir that contains all lable images. Label images are assumed to have same name with atlas images.
    target_image: the fixed image that the atlas image will register to.
    """

    ants_path = "/scratch/packages/antsbin/bin"
    script_path = "/scratch/packages/ANTs/Scripts"

    # assumeing atlas_dir only has atlas files.
    all_atlas_files = [f for f in os.listdir(atlas_dir)]
    # for atlas in all_atlas_files:
        
    

def reg_quick(fixed_im, moving_im, out_prefix, n_threads):
    """
    Rewrite antsRegistrationSyNQuick script.

    fixed_im: 
    moving_im: 
    out_im:
    """

    ants_path = os.environ['ANTSPATH']
    moving_warped = out_prefix + "Warped.nii.gz"
    fixed_warped = out_prefix + "InverseWarped.nii.gz"
    
    reg_cmd = [os.path.join(ants_path, 'antsRegistration'), 
               '--dimensionality', '3',
               '--float', '1',
               '--output', '[' + out_prefix +  ',' + moving_warped + ',' + fixed_warped, ']',
               '--interpolation', 'Linear',
               '--use-histogram-matching', '1',
               '--winsorize-image-intensities', '[0.005,0.995]',
               '--initial-moving-transform', '[' + fixed_im + ',' + moving_im + ',1]',
               '--transform', 'Rigid[0.1]',
               '--metric', 'MI[' + fixed_im + ',' + moving_im + ',1,32,Regular,0.25]',
               '--convergence', '[1000x500x250x0,1e-6,10]',
               '--shrink-factors', '12x8x4x2',
               '--smoothing-sigmas', '4x3x2x1vox',
               # '--transform', 'Affine[0.1]',
               # '--metric', 'MI[' + fixed_im + ',' + moving_im + ',1,32,Regular,0.25]',
               # '--convergence', '[1000x500x250x0,1e-6,10]',
               # '--shrink-factors', '12x8x4x2',
               # '--smoothing-sigmas', '4x3x2x1vox',
               # '--transform', 'SyN[0.1,3,0]',
               # '--metric', 'MI[' + fixed_im + ',' + moving_im + ',1,32]',
               # '--convergence', '[100x100x70x50x0,1e-6,10]',
               # '--shrink-factors', '10x6x4x2x1',
               # '--smoothing-sigmas', '5x3x2x1x0vox',
               ]

    print reg_cmd
    subprocess.call(reg_cmd)

def reg_allsub(fixed, moving_dir, out_dir, n_threads = 10):
    """
    register all Atlas subjects to target image.
    """

    all_moving_files = [f for f in os.listdir(moving_dir)]
    
    all_jobs = []
    for moving in all_moving_files:
        # assuming .ni.gz files.
        sub_id, dummy_ext = os.path.splitext(moving)
        sub_id, dummy_ext = os.path.splitext(sub_id)
        print "subject ID: {}".format(sub_id)
        out_prefix = os.path.join(out_dir, sub_id)
        job_proc = multiprocessing.Process(target = reg_quick, args = (fixed, os.path.join(moving_dir, moving), out_prefix, n_threads))
        all_jobs.append(job_proc)
        job_proc.start()
        

