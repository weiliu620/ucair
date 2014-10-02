UCAIR Pulmonary Disease Project. 
=====

This repository includes some c++ and python code for the lung and vessel
extraction. The code is only compiled at Linux. 

Requirement to build the c++ code under Linux: 

(1) Latest build of ITK.

(2) Latest Boost library (http://www.boost.org/). Most libraries in Boost are
header only, and do not need to build. However, this project need the program
option library, file system library, and system library. These three libraries
do need to be build by following BOOST document. 

(3) Set environment variable as (in Bash):

export BOOST_ROOT=YOUR_BOOST_BUILD_DIR

(4) For out-of-source build of this project, create a build dir. In the build
dir, run 

ccmake SOURCE_DIR

SOURCE_DIR is the git_root_dir/code, which contains CMakeLists.txt file that
ccmake will read for configuration. 

Press 'c' to configure, and ccmake will complain ITK not found. Give the ITK
build path, and 'c' again. ccmake still complain boost not found, but it does
not matter since we already give BOOST path as shell env. variable. When no
error found, press 'g' to generate Make file. 

run "make" in the current build dir, or "make VERBOSE=1" to get more build
information. The binary executable should be build and save into current build
dir. 

---------------------------------------

Requirement to use the Python script: 

Need Numpy, Scipy and SimpleITK. All can be installed through "pip" command. 


