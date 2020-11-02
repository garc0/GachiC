cmake_minimum_required(VERSION 3.15)
project(gc)

enable_testing()

set(CMAKE_CXX_STANDARD 17)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${LLD_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})

add_definitions(${LLVM_DEFINITIONS})

add_executable(gc 
                src/main.cpp 

                src/Lexer/Lexer.cpp
                src/Parser/Parser.cpp
                src/CodeGen/Visitor.cpp
                src/AST/ast.cpp
                
                src/states.cpp 
                )

set_property(TARGET gc PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_link_libraries(gc LINK_PUBLIC ${LLVM_AVAILABLE_LIBS})


