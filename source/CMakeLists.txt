cmake_minimum_required(VERSION 2.8.12)

project(komp)

#Use C++11
set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "-std=c++11 -stdlib=libc++")


#Use bison to generate the parser
add_executable(bison IMPORTED)
set_property(TARGET bison PROPERTY IMPORTED_LOCATION "bison")
set(PARSER_Y ${CMAKE_CURRENT_SOURCE_DIR}/lang.ypp)
set(PARSER_H ${CMAKE_CURRENT_BINARY_DIR}/parser.hpp)
set(PARSER_C ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
add_custom_command(OUTPUT ${PARSER_H} ${PARSER_C}
	DEPENDS ${PARSER_Y}
	COMMAND bison -d --name-prefix komp_ParserCallback_ --file-prefix=parser ${PARSER_Y}
	COMMAND mv parser.tab.cpp parser.cpp
	COMMAND mv parser.tab.hpp parser.hpp
)

#Use flex to generate the scanner
add_executable(flex IMPORTED)
set_property(TARGET flex PROPERTY IMPORTED_LOCATION "flex")
set(SCANNER_L ${CMAKE_CURRENT_SOURCE_DIR}/lang.l)
set(SCANNER_H ${CMAKE_CURRENT_BINARY_DIR}/scanner.hpp)
set(SCANNER_C ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp)
add_custom_command(OUTPUT ${SCANNER_H} ${SCANNER_C}
	DEPENDS ${SCANNER_L} ${PARSER_H}
	COMMAND flex --header-file=${SCANNER_H} --outfile=${SCANNER_C} ${SCANNER_L}
)



include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(komp
	main.cpp
	Grammar.hpp
	ScannerCallback.hpp
	ScannerCallback.cpp
	ParserCallback.hpp
	ParserCallback.cpp
	${SCANNER_L}
	${SCANNER_H}
	${SCANNER_C}
	${PARSER_Y}
	${PARSER_H}
	${PARSER_C}
)