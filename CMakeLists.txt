cmake_minimum_required(VERSION 3.15)
project(GachiC)

enable_testing()

set(CMAKE_CXX_STANDARD 17)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${LLD_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})

add_definitions(${LLVM_DEFINITIONS})

# Now build our tools
add_executable(GachiC 
                main.cpp 
                Lexer/Lexer.cpp
                Parser/Parser.cpp
                states.cpp 

                AST/NumberExprNode.cpp
                AST/VariableExprNode.cpp
                AST/VarExprNode.cpp
                AST/UnaryExprNode.cpp
                AST/BinaryExprNode.cpp
                AST/IfNode.cpp
                AST/CallExprNode.cpp
                AST/ForExprNode.cpp
                AST/PrototypeNode.cpp
                AST/FunctionNode.cpp
                AST/BlockNode.cpp
                AST/StructNode.cpp
                AST/ArrayNode.cpp
                AST/TypeNode.cpp
                )

set_property(TARGET GachiC PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_link_libraries(GachiC LINK_PUBLIC ${LLVM_AVAILABLE_LIBS})


