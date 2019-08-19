cmake_minimum_required(VERSION 3.9)

# list the header files
set(HEADER_FILES
	include/alp/Pos.h
	include/alp/Rng.h
	include/alp/Tkn.h
	include/alp/Token_Listing.h
	include/alp/Err.h
	include/alp/Src.h
	include/alp/Scanner.h
	include/alp/Decl.h
	include/alp/AST.h
	include/alp/Parser.h
	include/alp/Gen.h
)

# list the source files
set(SOURCE_FILES
	src/alp/Tkn.cpp
	src/alp/Src.cpp
	src/alp/Scanner.cpp
	src/alp/Decl.cpp
	src/alp/AST.cpp
	src/alp/Parser.cpp
	src/alp/Gen.cpp
)

# add library target
add_library(alp
	${HEADER_FILES}
	${SOURCE_FILES}
)

# zay will link to mn
target_link_libraries(alp
	PUBLIC
		MoustaphaSaad::mn
		MoustaphaSaad::rgx
)

add_library(MoustaphaSaad::alp ALIAS alp)

# make it reflect the same structure as the one on disk
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

# let's enable warnings as errors
if(WIN32)
	target_compile_options(alp
		PRIVATE
			/WX /W4
	)
elseif(UNIX)
	target_compile_options(alp
		PRIVATE
			-Wall -Werror
	)
endif()

# enable C++17
# disable any compiler specifc extensions
# add d suffix in debug mode
target_compile_features(alp PUBLIC cxx_std_17)
set_target_properties(alp PROPERTIES
	CXX_EXTENSIONS OFF
	DEBUG_POSTFIX d
)

# define debug macro
target_compile_definitions(alp PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")

# generate exports header file
include(GenerateExportHeader)
generate_export_header(alp
	EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/alp/Exports.h
)

# list include directories
target_include_directories(alp
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
