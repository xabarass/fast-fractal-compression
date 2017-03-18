include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(encoder)
add_subdirectory(decoder)
add_subdirectory(ifs_transformations)
add_subdirectory(utils)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/GitVersion.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/GitVersion.cpp" @ONLY)
list(APPEND SRC_FILES "${CMAKE_CURRENT_BINARY_DIR}/GitVersion.cpp")

#Applications entry point
list(APPEND SRC_FILES FractalCompression.cpp)
add_executable(fractal-compression ${SRC_FILES})

target_link_libraries(fractal-compression decoder pcm)
