cmake_minimum_required(VERSION 3.9)

# list the header files
set(HEADER_FILES
	include/zay/Rng.h
	include/zay/Pos.h
	include/zay/Tkn.h
	include/zay/Token_Listing.h
	include/zay/Err.h
	include/zay/Src.h
	include/zay/Scanner.h
	include/zay/AST.h
	include/zay/Parser.h
	include/zay/AST_Lisp.h
)

# list the source files
set(SOURCE_FILES
	src/zay/Tkn.cpp
	src/zay/Src.cpp
	src/zay/Scanner.cpp
	src/zay/AST.cpp
	src/zay/Parser.cpp
	src/zay/AST_Lisp.cpp
)

# add library target
add_library(zay
	${HEADER_FILES}
	${SOURCE_FILES}
)

# zay will link to mn
target_link_libraries(zay
	PUBLIC
		MoustaphaSaad::mn
)

add_library(MoustaphaSaad::zay ALIAS zay)

# make it reflect the same structure as the one on disk
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TOOL_FILES})

# let's enable warnings as errors
if(WIN32)
	target_compile_options(zay
		PRIVATE
			/WX /W4
	)
elseif(UNIX)
	target_compile_options(zay
		PRIVATE
			-Wall -Werror
	)
endif()

# enable C++17
# disable any compiler specifc extensions
# add d suffix in debug mode
target_compile_features(zay PUBLIC cxx_std_17)
set_target_properties(zay PROPERTIES
	CXX_EXTENSIONS OFF
	DEBUG_POSTFIX d
)

# define debug macro
target_compile_definitions(zay PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")

# generate exports header file
include(GenerateExportHeader)
generate_export_header(zay
	EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/zay/Exports.h
)

# list include directories
target_include_directories(zay
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)