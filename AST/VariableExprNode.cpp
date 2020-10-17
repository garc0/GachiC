#include "VariableExprNode.h"

#include "../states.h"

llvm::Value * VariableExprNode::codegen() {
  llvm::Value *V = std::get<0>(NamedValues[Name]);

  if (!V){
    std::cout << "Unknown variable name" << std::endl;
    return nullptr;
  }

  // Load the value.
  return Builder.CreateLoad(V, Name.c_str());
}