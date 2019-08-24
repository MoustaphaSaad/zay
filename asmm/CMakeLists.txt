cmake_minimum_required(VERSION 3.9)

# list the header files
set(HEADER_FILES
	include/asmm/Ins.h
	include/asmm/Err.h
	include/asmm/Src.h
	include/asmm/scan/Pos.h
	include/asmm/scan/Rng.h
	include/asmm/scan/Scanner.h
	include/asmm/scan/Tkn.h
	include/asmm/scan/Token_Listing.h
	include/asmm/parse/Ins.h
	include/asmm/Exports.h
)

# list the source files
set(SOURCE_FILES
	src/asmm/scan/Tkn.cpp
	src/asmm/scan/Scanner.cpp
	src/asmm/Src.cpp
)

# add library target
add_library(asmm
	${HEADER_FILES}
	${SOURCE_FILES}
)

target_link_libraries(asmm
	PUBLIC
		MoustaphaSaad::mn
		MoustaphaSaad::vm
)

add_library(MoustaphaSaad::asmm ALIAS asmm)

# make it reflect the same structure as the one on disk
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

# let's enable warnings as errors
if(WIN32)
	target_compile_options(asmm
		PRIVATE
			/WX /W4
	)
elseif(UNIX)
	target_compile_options(asmm
		PRIVATE
			-Wall -Werror
	)
endif()

# enable C++17
# disable any compiler specifc extensions
# add d suffix in debug mode
target_compile_features(asmm PUBLIC cxx_std_17)
set_target_properties(asmm PROPERTIES
	CXX_EXTENSIONS OFF
	DEBUG_POSTFIX d
)

# define debug macro
target_compile_definitions(asmm PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")

# generate exports header file
include(GenerateExportHeader)
generate_export_header(asmm
	EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/asmm/Exports.h
)

# list include directories
target_include_directories(asmm
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)