set(DECODER_SRC
    test_decoder.c
    QuadTreeDecoder.c
    Decoder.cpp
)
add_library(decoder STATIC ${DECODER_SRC})

target_link_libraries(decoder utils ifs_transform)
