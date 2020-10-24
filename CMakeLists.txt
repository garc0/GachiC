cmake_minimum_required(VERSION 3.13.4)
project(GachiC)

enable_testing()

set(CMAKE_CXX_STANDARD 17)

SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS_INIT} -L/usr/lib/llvm-10/lib ")

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})


link_libraries()
execute_process(COMMAND llvm-config --libs OUTPUT_VARIABLE LIBS)
execute_process(COMMAND llvm-config --system-libs OUTPUT_VARIABLE SYS_LIBS)
execute_process(COMMAND llvm-config --ldflags OUTPUT_VARIABLE LDF)


string(STRIP "${LIBS}" LIBS)
string(STRIP "${SYS_LIBS}" SYS_LIBS)
string(STRIP "${LDF}" LDF)

link_libraries(${LIBS} ${SYS_LIBS} ${LDF})

execute_process(COMMAND llvm-config --cxxflags OUTPUT_VARIABLE CMAKE_CXX_FLAGS)
string(STRIP "${CMAKE_CXX_FLAGS}" CMAKE_CXX_FLAGS)

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
                )


