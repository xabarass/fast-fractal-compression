#      _________   _____________________  ____  ______
#     / ____/   | / ___/_  __/ ____/ __ \/ __ \/ ____/
#    / /_  / /| | \__ \ / / / /   / / / / / / / __/
#   / __/ / ___ |___/ // / / /___/ /_/ / /_/ / /___
#  /_/   /_/  |_/____//_/  \____/\____/_____/_____/
#
#  http://www.inf.ethz.ch/personal/markusp/teaching/
#  How to Write Fast Numerical Code 263-2300 - ETH Zurich
#  Copyright (C) 2017 Alen Stojanov (astojanov@inf.ethz.ch)
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program. If not, see http://www.gnu.org/licenses/.

# Save the initial CXX in case those get modified
# in the sub modules.
set(INIT_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(INIT_CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}"  )

# ================================================================================
# Build all dependencies.
# ================================================================================
if (RDTSC_FAILBACK)
    message(STATUS "Building RDTSC failback benchmark")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -std=c++11")

    set(PERF_FILES
        ${CMAKE_CURRENT_LIST_DIR}/pcm/include/perf.h
        ${CMAKE_CURRENT_LIST_DIR}/pcm/perf/perf_failback.cpp
    )

    add_library(pcm ${PERF_FILES})

    include (GenerateExportHeader)
    include_directories(${CMAKE_CURRENT_LIST_DIR}/pcm)
    GENERATE_EXPORT_HEADER(pcm
            BASE_NAME pcm
            EXPORT_MACRO_NAME pcm_EXPORT
            EXPORT_FILE_NAME pcm_Export.h
            STATIC_DEFINE pcm_BUILT_AS_STATIC
    )

else (RDTSC_FAILBACK)
    message(STATUS "Building Intel PCM benchmark")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/pcm)
    include_directories(${CMAKE_CURRENT_LIST_DIR}/pcm)
endif(RDTSC_FAILBACK)

set(CMAKE_CXX_FLAGS "${INIT_CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS   "${INIT_CMAKE_C_FLAGS}"  )
