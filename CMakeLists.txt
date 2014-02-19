cmake_minimum_required(VERSION 2.8)
PROJECT(vessel)

SET(CMAKE_VERBOSE_MAKEFILE OFF)

FIND_PACKAGE(ITK REQUIRED)
IF(ITK_FOUND)
  INCLUDE(${ITK_USE_FILE})
ENDIF(ITK_FOUND)

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
  add_executable(dijk
    dijk.cxx
    utility.cxx
    )
  
  target_link_libraries(dijk ${ITK_LIBRARIES} ${Boost_LIBRARIES})
endif()

