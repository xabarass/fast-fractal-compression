set(ENCODER_SRC
    QuadTreeEncoder.c
    Encoder.cpp
)

if(USE_VECTORIZE)
    message("Using vectorized encoder utils")
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/simd")
else(USE_VECTORIZE)
    message("Using scalar encoder utils")
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/scalar")
endif(USE_VECTORIZE)

if(USE_CACHE_OPT)
    message("Using cache optimized code")
    if(USE_BULK_COMPUTE)
        message("Doing bulk compute")
        list(APPEND ENCODER_SRC CacheOptBulkMatching.c)
    else(USE_BULK_COMPUTE)
        message("Not doing bulk compute")
        list(APPEND ENCODER_SRC CacheOptMatching.c)
    endif(USE_BULK_COMPUTE)
    else(USE_CACHE_OPT)
        list(APPEND ENCODER_SRC Matching.c)
endif(USE_CACHE_OPT)

add_library(encoder STATIC ${ENCODER_SRC})
target_link_libraries(encoder utils ifs_transform)
