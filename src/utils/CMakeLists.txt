set(UTILS_SRC
    Image.cpp
    ImageData.c
    BMPImage.cpp
)
add_library(utils STATIC ${UTILS_SRC})

#target_link_libraries(utils ...)
