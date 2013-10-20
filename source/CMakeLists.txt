cmake_minimum_required(VERSION 2.8.12)

project(komp)
enable_testing()

#Use C++11
set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ -g")
set(CMAKE_EXE_LINKER_FLAGS "-std=c++11 -stdlib=libc++")
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(komp
	include/komp/komp.hpp
	komp.cpp
	log.hpp
	tutil.hpp
	tutil.cpp
)
add_executable(komp-test
	tests/simple.cpp
)
target_link_libraries(komp-test
	komp
)

add_test(komp-test komp-test)
