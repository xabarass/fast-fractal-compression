set(ENCODER_SRC
    QuadTreeEncoder.c
    Encoder.cpp
)
add_library(encoder STATIC ${ENCODER_SRC})

target_link_libraries(encoder utils)
