cmake_minimum_required(VERSION 3.9)

# list the header files
set(HEADER_FILES
	include/cypher/Exports.h
	include/cypher/Type.h
	include/cypher/Basic_Block.h
	include/cypher/Func.h
	include/cypher/Pkg.h
	include/cypher/Builder.h
	include/cypher/Val.h
	include/cypher/Ins.h
)

# list the source files
set(SOURCE_FILES
	src/cypher/Val.cpp
	src/cypher/Type.cpp
	src/cypher/Func.cpp
	src/cypher/Pkg.cpp
	src/cypher/Builder.cpp
)

# add library target
add_library(cypher
	${HEADER_FILES}
	${SOURCE_FILES}
)

target_link_libraries(cypher
	PUBLIC
		MoustaphaSaad::mn
)

add_library(MoustaphaSaad::cypher ALIAS cypher)

# make it reflect the same structure as the one on disk
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

# let's enable warnings as errors
if(WIN32)
	target_compile_options(cypher
		PRIVATE
			/WX /W4
	)
elseif(UNIX)
	target_compile_options(cypher
		PRIVATE
			-Wall -Werror
	)
endif()

# enable C++17
# disable any compiler specifc extensions
# add d suffix in debug mode
target_compile_features(cypher PUBLIC cxx_std_17)
set_target_properties(cypher PROPERTIES
	CXX_EXTENSIONS OFF
	DEBUG_POSTFIX d
)

# define debug macro
target_compile_definitions(cypher PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")

# generate exports header file
include(GenerateExportHeader)
generate_export_header(cypher
	EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/cypher/Exports.h
)

# list include directories
target_include_directories(cypher
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)