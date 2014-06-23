ucair
=====

This repository includes some c++ and python code for the lung and vessel
extraction. 

Requirement to build the c++ code: 

(1) Latest Boost library (http://www.boost.org/). Most libraries in Boost are
header only, and do not need to build. Unfortunately, this project need the
program option library, file system library, and system library that do need to
build. 

(2) Set environment variable as (in Bash):

export BOOST_ROOT=YOUR_BOOST_BUILD_DIR

(3) For out-of-source build, in the build dir, run ccmake.

Requirement to use the Python script: 

Need Numpy, Scipy and SimpleITK. All can be installed through "pip" command. 


=========================================


Human lung's vessel extraction.

- for loop to go through all target node and find the path. 

re-run multiscale hessian filter with smaller max scale, and more steps, since
the difference between two scales are not 2, but 3-root of 2. 

