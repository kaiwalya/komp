cmake_minimum_required(VERSION 2.8.12)

project(devpack)

include(externalproject)

set(LIBV8_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/libv8")
externalproject_add(
	libv8
	PREFIX ${LIBV8_PREFIX}
	GIT_REPOSITORY git://github.com/v8/v8.git
	BUILD_IN_SOURCE 1
	CONFIGURE_COMMAND make dependencies
	BUILD_COMMAND echo "CC=clang && CXX=clang++ && GYP_DEFINES='clang=1' && make -j8 x64.release" | sh
	INSTALL_COMMAND true
	TEST_COMMAND true
)
include_directories(${LIBV8_PREFIX}/src/libv8/include)


set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "-std=c++11 -stdlib=libc++")


link_directories(${LIBV8_PREFIX}/src/libv8/out/x64.release)
add_executable(devpack main.cpp)
add_dependencies(devpack libv8)
target_link_libraries(devpack
	v8_base.x64
	v8_snapshot
	icui18n
	icudata
	icuuc
)