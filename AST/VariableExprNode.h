#pragma once 

#include <string>

#include "BaseNode.h"


/// VariableExprNode - Expression class for referencing a variable, like "a".
class VariableExprNode : public BaseNode {
  std::string Name;

public:
  VariableExprNode(const std::string &Name) : Name(Name) {}

  llvm::Value *codegen() override;
  const std::string &getName() const { return Name; }
};