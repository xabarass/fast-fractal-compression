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
set(INTEL_DEFAULT_FLAGS "-Wall")
set(OPTIMIZATION_FLAGS  "-O3")
set(INTEL_OPTIMIZATION_FLAGS  "-O3 -march=CORE-AVX2 -axCORE-AVX2 -xCORE-AVX2 -parallel -finline-functions -ip -fp-model fast -unroll -opt-prefetch=4")
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
    IF(INTEL_COMPILER)
        SET(DEFAULT_FLAGS "${INTEL_DEFAULT_FLAGS} -fma")
    ELSE(INTEL_COMPILER)
        SET(DEFAULT_FLAGS "${DEFAULT_FLAGS} -mfma")
    ENDIF(INTEL_COMPILER)
ELSE(USE_FMA)
    IF(INTEL_COMPILER)
        SET(DEFAULT_FLAGS "${INTEL_DEFAULT_FLAGS} -mno-fma")
    ELSE(INTEL_COMPILER)
        SET(DEFAULT_FLAGS "${DEFAULT_FLAGS} -no-fma")
    ENDIF(INTEL_COMPILER)
ENDIF(USE_FMA)

IF(USE_VECTORIZE)
    add_definitions(-DUTILS_TYPE="vectorized")
    SET(DEFAULT_FLAGS "${DEFAULT_FLAGS} -mavx -mavx2")
ELSE(USE_VECTORIZE)
    add_definitions(-DUTILS_TYPE="scalar")
    SET(DEFAULT_FLAGS "${DEFAULT_FLAGS} ")
ENDIF(USE_VECTORIZE)

IF(USE_MATH_FLAGS)
    SET(DEFAULT_FLAGS "${DEFAULT_FLAGS} -march=native -ffast-math -funsafe-math-optimizations -ftree-vectorize -funroll-all-loops")
ELSE(USE_MATH_FLAGS)
    SET(DEFAULT_FLAGS "${DEFAULT_FLAGS} ")
ENDIF(USE_MATH_FLAGS)

if(USE_CACHE_OPT)
    if(USE_BULK_COMPUTE)
        add_definitions(-DMATCHING_TYPE="-cache-opt-bulk-average-pixel")
    else(USE_BULK_COMPUTE)
        add_definitions(-DMATCHING_TYPE="-cache-opt")
    endif(USE_BULK_COMPUTE)
else(USE_CACHE_OPT)
    add_definitions(-DMATCHING_TYPE="-full-recursive")
endif(USE_CACHE_OPT)

set(CMAKE_C_FLAGS           "${DEFAULT_FLAGS} -std=c99")
set(CMAKE_CXX_FLAGS         "${DEFAULT_FLAGS} -std=c++11")

# Default optimization
set(CMAKE_C_FLAGS_RELEASE   ${OPTIMIZATION_FLAGS})
set(CMAKE_CXX_FLAGS_RELEASE ${OPTIMIZATION_FLAGS})

# For degbugging only
set(CMAKE_C_FLAGS_DEBUG     ${DEBUG_FLAGS})
set(CMAKE_CXX_FLAGS_DEBUG   ${DEBUG_FLAGS})

IF(INTEL_COMPILER)
        message(STATUS "Setting the intel compiler flag")
        set(CMAKE_CXX_COMPILER "icpc")
        set(CMAKE_C_COMPILER "icc")
        set(CMAKE_C_FLAGS "${INTEL_DEFAULT_FLAGS} -std=c99")
        set(CMAKE_CXX_FLAGS "${INTEL_DEFAULT_FLAGS} -std=c++11")
        set(CMAKE_C_FLAGS_RELEASE   ${INTEL_OPTIMIZATION_FLAGS})
        set(CMAKE_CXX_FLAGS_RELEASE ${INTEL_OPTIMIZATION_FLAGS})
ENDIF(INTEL_COMPILER)

IF(GENERATE_FLOP_COUNT)
    message("Counting operations, will slow down execution")
    add_definitions(-DCOUNT_FLOPS=1)
ENDIF(GENERATE_FLOP_COUNT)

add_subdirectory(src)
