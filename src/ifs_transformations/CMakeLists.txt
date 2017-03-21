set(IFS_TRANS_SRC
    test_ifs.c
        ifs_transform.c ifs_transform.h)
add_library(ifs_transform STATIC ${IFS_TRANS_SRC})

target_link_libraries(ifs_transform utils)
