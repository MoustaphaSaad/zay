cmake_minimum_required(VERSION 3.9)

# list the header files
set(HEADER_FILES
	include/zsm/Err.h
	include/zsm/Src.h
	include/zsm/Gen.h
	include/zsm/scan/Pos.h
	include/zsm/scan/Rng.h
	include/zsm/scan/Scanner.h
	include/zsm/scan/Tkn.h
	include/zsm/scan/Token_Listing.h
	include/zsm/parse/Ins.h
	include/zsm/parse/Func.h
	include/zsm/parse/Parser.h
	include/zsm/Exports.h
)

# list the source files
set(SOURCE_FILES
	src/zsm/scan/Tkn.cpp
	src/zsm/scan/Scanner.cpp
	src/zsm/parse/Func.cpp
	src/zsm/parse/Parser.cpp
	src/zsm/Src.cpp
	src/zsm/Gen.cpp
)

# add library target
add_library(zsm
	${HEADER_FILES}
	${SOURCE_FILES}
)

target_link_libraries(zsm
	PUBLIC
		MoustaphaSaad::mn
		MoustaphaSaad::vm
)

add_library(MoustaphaSaad::zsm ALIAS zsm)

# make it reflect the same structure as the one on disk
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

# let's enable warnings as errors
if(WIN32)
	target_compile_options(zsm
		PRIVATE
			/WX /W4
	)
elseif(UNIX)
	target_compile_options(zsm
		PRIVATE
			-Wall -Werror
	)
endif()

# enable C++17
# disable any compiler specifc extensions
# add d suffix in debug mode
target_compile_features(zsm PUBLIC cxx_std_17)
set_target_properties(zsm PROPERTIES
	CXX_EXTENSIONS OFF
	DEBUG_POSTFIX d
)

# define debug macro
target_compile_definitions(zsm PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")

# generate exports header file
include(GenerateExportHeader)
generate_export_header(zsm
	EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/zsm/Exports.h
)

# list include directories
target_include_directories(zsm
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)