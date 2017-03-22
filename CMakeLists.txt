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

set(DEFAULT_FLAGS "-Wall -Wpedantic -Wchkp -Wno-cpp -Wno-variadic-macros")
set(OPTIMIZATION_FLAGS  "-O3 -march=native")
set(DEBUG_FLAGS         "-O0 -g")

# ================================================================================
# Options to customize build
# ================================================================================
option(USE_FMA "Enable FMA instructions" OFF)
option(USE_VECTORIZE "Allow vectorization" OFF)

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
    SET(DEFAULT_FLAGS "${DEFAULT_FLAGS} -mfma")
ELSE(USE_VECTORIZE)
    SET(DEFAULT_FLAGS "${DEFAULT_FLAGS} -mavx")
ENDIF(USE_VECTORIZE)

set(CMAKE_C_FLAGS           ${DEFAULT_FLAGS})
set(CMAKE_CXX_FLAGS         "${DEFAULT_FLAGS} -std=c++11")

# Default optimization
set(CMAKE_C_FLAGS_RELEASE   ${OPTIMIZATION_FLAGS})
set(CMAKE_CXX_FLAGS_RELEASE ${OPTIMIZATION_FLAGS})

# For degbugging only
set(CMAKE_C_FLAGS_DEBUG     ${DEBUG_FLAGS})
set(CMAKE_CXX_FLAGS_DEBUG   ${DEBUG_FLAGS})

add_subdirectory(src)
