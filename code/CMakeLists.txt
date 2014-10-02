cmake_minimum_required(VERSION 2.8)
PROJECT(vessel)

SET(CMAKE_VERBOSE_MAKEFILE OFF)

FIND_PACKAGE(ITK REQUIRED)
IF(ITK_FOUND)
  INCLUDE(${ITK_USE_FILE})
ENDIF(ITK_FOUND)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

option (USE_OMP
  "use openmp for parallel execution" on)
if (USE_OMP)
  find_package(OpenMP)
  if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif()
endif(USE_OMP)

## The next part looks for LEMON. Typically, you don't want to modify it.
##
## First, it tries to use LEMON as a CMAKE subproject by looking for
## it in the 'lemon' or 'deps/lemon' subdirectories or in directory
## given by the LEMON_SOURCE_ROOT_DIR variable.
## If LEMON isn't there, then CMAKE will try to find an installed
## version of LEMON. If it is installed at some non-standard place,
## then you must tell its location in the LEMON_ROOT_DIR CMAKE config
## variable. (Do not hard code it into your config! Others may keep
## LEMON at different places.)

FIND_PATH(LEMON_SOURCE_ROOT_DIR CMakeLists.txt
  PATHS ${CMAKE_SOURCE_DIR}/lemon ${CMAKE_SOURCE_DIR}/deps/lemon
  NO_DEFAULT_PATH
  DOC "Location of LEMON source as a CMAKE subproject")

IF(EXISTS ${LEMON_SOURCE_ROOT_DIR})
  ADD_SUBDIRECTORY(${LEMON_SOURCE_ROOT_DIR} deps/lemon)
  SET(LEMON_INCLUDE_DIRS
    ${LEMON_SOURCE_ROOT_DIR}
    ${CMAKE_BINARY_DIR}/deps/lemon
  )
  SET(LEMON_LIBRARIES lemon)
  UNSET(LEMON_ROOT_DIR CACHE)
  UNSET(LEMON_DIR CACHE)
  UNSET(LEMON_INCLUDE_DIR CACHE)
  UNSET(LEMON_LIBRARY CACHE)
ELSE()
  FIND_PACKAGE(LEMON QUIET NO_MODULE)
  FIND_PACKAGE(LEMON REQUIRED)
ENDIF()

INCLUDE_DIRECTORIES(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  ${LEMON_INCLUDE_DIRS}
)

set(Boost_DEBUG 0)
set(Boost_USE_STATIC_LIBS on)
set(Boost_USE_MULTITHREADED ON) 
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.53.0 REQUIRED COMPONENTS program_options filesystem system)

if(Boost_FOUND)
  MESSAGE(STATUS "Project source dir: " ${PROJECT_SOURCE_DIR})
  include_directories(${Boost_INCLUDE_DIRS})

  add_library(utility
    utility.cxx
    )    
  add_executable(dijk
    dijk.cxx

    )

  add_executable(test_hessian_filter
    test_hessian_filter
    )

  add_executable(multiscale_hessian
    multiscale_hessian.cxx

    )

  add_executable(opening_filter
    opening_filter

    )

  add_executable(closing_filter
    closing_filter

    )

  add_executable(dilation_filter
    dilation_filter

    )

  add_executable(erosion_filter
    erosion_filter

    )

  add_executable(connected_comp
    connected_comp

    )

  add_executable(fillhole_filter
    fillhole_filter.cxx

    )

  add_executable(voting_fillhole_filter
    voting_fillhole_filter.cxx

    )

  add_executable(binary_fillhole
    binary_fillhole.cxx
    )

  add_executable(my_hessian_test
    my_hessian_test.cxx
    hessian_eigenvector.cxx

    )

  add_executable(mha_to_nifti
    mha_to_nifti

    )
  
  add_executable(gmm
    gmm.cxx

    )

  add_executable(test_gmm
    test_gmm.cxx
    )    

  add_executable(derivative_filter
    derivative_filter.cxx
    )    

  add_executable(fastmarching
    fastmarching.cxx
    )    

  # test fast marching upwind filter.
  add_executable(test_ffm_upwind
    test_ffm_upwind.cxx
    )

  add_executable(fmm_upwind
    fmm_upwind.cxx
    )

  add_executable(inverse_distmap
    inverse_distmap.cxx
    )

  add_executable(find_path
    find_path.cxx
    )

  add_executable(est_density
    est_density.cxx
    )


  add_executable(reg_speed
    reg_speed.cxx
    )
  add_executable(extract_roi
    extract_roi
    )

  # add_executable(vtkmesh2itkmesh
  #   vtkmesh2itkmesh.cxx
  #   )

  # add_executable(kmeans
  #   kmeans.cxx
    )

  target_link_libraries(dijk utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
  target_link_libraries(test_hessian_filter utility ${ITK_LIBRARIES})
  target_link_libraries(multiscale_hessian utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
  target_link_libraries(opening_filter utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
  target_link_libraries(closing_filter utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
  target_link_libraries(dilation_filter utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
  target_link_libraries(erosion_filter utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
  target_link_libraries(connected_comp utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
  target_link_libraries(fillhole_filter utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
  target_link_libraries(voting_fillhole_filter utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})

  target_link_libraries(binary_fillhole utility ${ITK_LIBRARIES})
  target_link_libraries(my_hessian_test utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
  target_link_libraries(mha_to_nifti utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
  target_link_libraries(gmm utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
  target_link_libraries(test_gmm utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
  target_link_libraries(derivative_filter utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
  target_link_libraries(fastmarching utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
  target_link_libraries(test_ffm_upwind utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
  target_link_libraries(fmm_upwind utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
  target_link_libraries(inverse_distmap utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
  target_link_libraries(find_path utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
  target_link_libraries(est_density utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
  target_link_libraries(reg_speed utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
  target_link_libraries(extract_roi utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
  # target_link_libraries(vtkmesh2itkmesh utility ${ITK_LIBRARIES} ${Boost_LIBRARIES} ${VTK_LIBRARIES})
  # target_link_libraries(kmeans utility ${ITK_LIBRARIES} ${Boost_LIBRARIES})
endif()

