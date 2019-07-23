cmake_minimum_required(VERSION 3.9)

# list source files
set(SOURCE_FILES
	src/unittest_main.cpp
	src/unittest_zay.cpp
	src/unittest_cypher.cpp
)

# add executable target
add_executable(unittest
	${SOURCE_FILES}
)

target_link_libraries(unittest
	PRIVATE
		MoustaphaSaad::zay
		MoustaphaSaad::cypher
)

# add doctest folder
target_include_directories(unittest
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/doctest
)
