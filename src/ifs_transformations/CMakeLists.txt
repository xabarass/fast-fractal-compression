set(IFS_TRANS_SRC
    test_ifs.c
    ifs_transform.c)

if(USE_VECTORIZE)
    message("Using vectorized downsampling")
    list(APPEND IFS_TRANS_SRC downsampling_vectorized.c)
else(USE_VECTORIZE)
    message("Using scalar downsampling")
    list(APPEND IFS_TRANS_SRC downsampling_scalar.c)
endif(USE_VECTORIZE)

add_library(ifs_transform STATIC ${IFS_TRANS_SRC})
target_link_libraries(ifs_transform utils)
