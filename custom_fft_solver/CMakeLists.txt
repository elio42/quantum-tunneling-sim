# 1. Set the minimum version of CMake required
cmake_minimum_required(VERSION 3.10)
project(HelloCMake)


# 1. Ask CMake to find the required Boost components on your Fedora system
find_package(Boost REQUIRED COMPONENTS iostreams system)


# 3. Tell CMake to build an executable named "hello" from "main.cpp"
add_executable(hello 
    src/main.cpp
    src/fft.cpp
)


# 3. Tell your executable where the Boost headers are
target_include_directories(hello PRIVATE ${Boost_INCLUDE_DIRS})
# 4. Link the actual Boost libraries to your executable
target_link_libraries(hello PRIVATE ${Boost_LIBRARIES})