set(DECODER_SRC
    QuadTreeDecoder.c
    Decoder.cpp
    test_decoder.c
)
add_library(decoder STATIC ${DECODER_SRC})

target_link_libraries(decoder utils ifs_transform)
