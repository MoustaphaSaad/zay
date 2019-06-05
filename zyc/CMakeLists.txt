cmake_minimum_required(VERSION 3.9)

# list the source files
set(SOURCE_FILES
	src/zyc.cpp
)

# add executable
add_executable(zyc
	${SOURCE_FILES}
)

target_link_libraries(zyc
	PUBLIC
		MoustaphaSaad::zay
)

# make it reflect the same structure as the one on disk
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

# let's enable warnings as errors
if(WIN32)
	target_compile_options(zyc
		PRIVATE
			/WX /W4
	)
elseif(UNIX)
	target_compile_options(zyc
		PRIVATE
			-Wall -Werror
	)
endif()

# enable C++17
# disable any compiler specifc extensions
# add d suffix in debug mode
target_compile_features(zyc PUBLIC cxx_std_17)
set_target_properties(zyc PROPERTIES
	CXX_EXTENSIONS OFF
)

# define debug macro
target_compile_definitions(zyc PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
