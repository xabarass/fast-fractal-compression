cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_VERBOSE_MAKEFILE ON)
project(FractalCompression CXX C)

# ================================================================================
# We want to have information about git from source code
# ================================================================================
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmakeModules/")
include(GetGitRevisionDescription)
include(lib/IntelPCM.cmake)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
git_describe(GIT_TAG)

# ================================================================================
# We want to put files in bin directory
# ================================================================================
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/arch")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# ================================================================================
# Default compiler flags, they will be extended by the other options
# ================================================================================

set(DEFAULT_FLAGS "-Wall -Wpedantic -Wno-cpp -Wno-variadic-macros")
set(OPTIMIZATION_FLAGS  "-O3")
set(DEBUG_FLAGS         "-O0 -g")

# ================================================================================
# Options to customize build
# ================================================================================

# Optimizations regarding compiler optimizations
option(USE_FMA "Enable FMA instructions" OFF)
option(USE_VECTORIZE "Allow vectorization" OFF)
option(USE_MATH_FLAGS "Use math flags" ON)
option(USE_CACHE_OPT "Use cache optimization" ON)
option(USE_BULK_COMPUTE "Use bulk compute of average pixel" ON)

IF (MY_COMPILER_OPTION STREQUAL "Intel")
  MESSAGE(STATUS "** Compiling with Intel settings **")
  SET(CMAKE_CXX_COMPILER "icpc")
  SET(CMAKE_CXX_COMPILER "icc")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -w")
ENDIF ()

# Optimizations regarding cycle counting
option(GENERATE_FLOP_COUNT "Count number of flops, will slow down" OFF)
option(COUNT_DETAIL_RUNTIME "Count number of cycles for each function" OFF)

set(PREDEFINED_CONFIGURATION "" CACHE STRING "Load predefined configuration")

# ================================================================================
# Here we can add predefined option configuration for common build types
# ================================================================================
IF(OPTIMIZATION_TYPE)
    message(STATUS "We are trying to load predefined configuration ${OPTIMIZATION_TYPE}")
ENDIF(OPTIMIZATION_TYPE)

IF(USE_FMA)
    SET(DEFAULT_FLAGS "${DEFAULT_FLAGS} -mfma")
ELSE(USE_FMA)
    SET(DEFAULT_FLAGS "${DEFAULT_FLAGS} -mno-fma")
ENDIF(USE_FMA)

IF(USE_VECTORIZE)
    SET(DEFAULT_FLAGS "${DEFAULT_FLAGS} -mavx -mavx2")
ELSE(USE_VECTORIZE)
    SET(DEFAULT_FLAGS "${DEFAULT_FLAGS} ")
ENDIF(USE_VECTORIZE)

IF(USE_MATH_FLAGS)
    SET(DEFAULT_FLAGS "${DEFAULT_FLAGS} -march=native -ffast-math -funsafe-math-optimizations -ftree-vectorize -funroll-all-loops")
ELSE(USE_MATH_FLAGS)
    SET(DEFAULT_FLAGS "${DEFAULT_FLAGS} ")
ENDIF(USE_MATH_FLAGS)

set(CMAKE_C_FLAGS           "${DEFAULT_FLAGS} -std=c99")
set(CMAKE_CXX_FLAGS         "${DEFAULT_FLAGS} -std=c++11")

# Default optimization
set(CMAKE_C_FLAGS_RELEASE   ${OPTIMIZATION_FLAGS})
set(CMAKE_CXX_FLAGS_RELEASE ${OPTIMIZATION_FLAGS})

# For degbugging only
set(CMAKE_C_FLAGS_DEBUG     ${DEBUG_FLAGS})
set(CMAKE_CXX_FLAGS_DEBUG   ${DEBUG_FLAGS})

IF(GENERATE_FLOP_COUNT)
    message("Counting operations, will slow down execution")
    add_definitions(-DCOUNT_FLOPS=1)
ENDIF(GENERATE_FLOP_COUNT)

add_subdirectory(src)
