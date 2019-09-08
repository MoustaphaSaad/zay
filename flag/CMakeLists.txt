cmake_minimum_required(VERSION 3.9)

# list the header files
set(HEADER_FILES
	include/flag/Flag.h
	include/flag/Exports.h
)

# list the source files
set(SOURCE_FILES
	src/flag/Flag.cpp
)

# add library target
add_library(flag
	${HEADER_FILES}
	${SOURCE_FILES}
)

target_link_libraries(flag
	PUBLIC
		MoustaphaSaad::mn
)

add_library(MoustaphaSaad::flag ALIAS flag)

# make it reflect the same structure as the one on disk
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

# let's enable warnings as errors
if(WIN32)
	target_compile_options(flag
		PRIVATE
			/WX /W4
	)
elseif(UNIX)
	target_compile_options(flag
		PRIVATE
			-Wall -Werror
	)
endif()

# enable C++17
# disable any compiler specifc extensions
# add d suffix in debug mode
target_compile_features(flag PUBLIC cxx_std_17)
set_target_properties(flag PROPERTIES
	CXX_EXTENSIONS OFF
	DEBUG_POSTFIX d
)

# define debug macro
target_compile_definitions(flag PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")

# generate exports header file
include(GenerateExportHeader)
generate_export_header(flag
	EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/flag/Exports.h
)

# list include directories
target_include_directories(flag
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)