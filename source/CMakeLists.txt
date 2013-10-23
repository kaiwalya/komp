cmake_minimum_required(VERSION 2.8.12)

project(komp)
enable_testing()

#Use C++11
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -std=c++11 -stdlib=libc++")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -O0 -g")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(komp
	include/komp/komp.hpp
	include/komp/typ.hpp
	komp.cpp
	log.hpp
	tutil.hpp
	tutil.cpp
)
add_executable(testKomp tests/simple.cpp)
target_link_libraries(testKomp komp)
add_executable(testTyp tests/testTyp.cpp)
target_link_libraries(testTyp komp)

add_test(testKomp testKomp)
add_test(testTyp testTyp)
