cmake_minimum_required(VERSION 2.8.12)

project(kaam)

#Use C++11
set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "-std=c++11 -stdlib=libc++")


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(kaam
	include/kaam/header.hpp
	main.cpp
	tests/simple.cpp
)