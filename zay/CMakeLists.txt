cmake_minimum_required(VERSION 3.9)

# list the header files
set(HEADER_FILES
	include/zay/scan/Rng.h
	include/zay/scan/Pos.h
	include/zay/scan/Tkn.h
	include/zay/scan/Token_Listing.h
	include/zay/scan/Scanner.h
	include/zay/parse/AST.h
	include/zay/parse/Parser.h
	include/zay/parse/AST_Lisp.h
	include/zay/parse/Type_Sign.h
	include/zay/parse/Expr.h
	include/zay/parse/Var.h
	include/zay/parse/Stmt.h
	include/zay/parse/Decl.h
	include/zay/typecheck/Sym.h
	include/zay/typecheck/Scope.h
	include/zay/typecheck/Typer.h
	include/zay/typecheck/Type_Intern.h
	include/zay/Err.h
	include/zay/Src.h
	include/zay/CGen.h
	include/zay/c/Preprocessor.h
)

# list the source files
set(SOURCE_FILES
	src/zay/scan/Tkn.cpp
	src/zay/scan/Scanner.cpp
	src/zay/parse/AST.cpp
	src/zay/parse/Parser.cpp
	src/zay/parse/AST_Lisp.cpp
	src/zay/parse/Type_Sign.cpp
	src/zay/parse/Expr.cpp
	src/zay/parse/Stmt.cpp
	src/zay/parse/Decl.cpp
	src/zay/typecheck/Scope.cpp
	src/zay/typecheck/Typer.cpp
	src/zay/typecheck/Sym.cpp
	src/zay/typecheck/Type_Intern.cpp
	src/zay/Src.cpp
	src/zay/CGen.cpp
	src/zay/c/Preprocessor.cpp
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