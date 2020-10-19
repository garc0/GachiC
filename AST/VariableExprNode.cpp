#include "VariableExprNode.h"

#include "../states.h"

llvm::Value * VariableExprNode::codegen() {
  
  auto f = NamedValues.find(Name);
  if(f == NamedValues.end()){
    std::cout << "Unknown variable name" << std::endl;
    return nullptr;
  }

  llvm::Value * V = std::get<0>(NamedValues[Name]);

  if (!V){
    std::cout << "Variable not defined" << std::endl;
    return nullptr;
  }

  // Load the value.
  return Builder.CreateLoad(V, Name.c_str());
}