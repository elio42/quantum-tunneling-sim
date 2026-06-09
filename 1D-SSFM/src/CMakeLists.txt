cmake_minimum_required(VERSION 3.15)
project(SSFM1D)

add_compile_definitions(_GNU_SOURCE)

file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp)
add_executable(SSFM1D ${SOURCES})

target_compile_options(SSFM1D PRIVATE
    -O3 -g -Wall -Wextra
    -fsanitize=undefined,float-divide-by-zero
    -fno-sanitize-recover=all
    -fno-omit-frame-pointer
    -march=native
)

target_link_options(SSFM1D PRIVATE
    -fsanitize=undefined,float-divide-by-zero
)

target_include_directories(SSFM1D PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
find_package(FFTW3 REQUIRED)
target_include_directories(SSFM1D PRIVATE ${FFTW3_INCLUDE_DIRS})
target_link_libraries(SSFM1D PRIVATE ${FFTW3_LIBRARIES})
