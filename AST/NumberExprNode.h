#pragma once

#include "BaseNode.h"

#include "../states.h"

class NumberExprNode : public BaseNode {
private:
  std::string _val;
public:
  NumberExprNode(std::string Val) : _val(Val) {}

  llvm::Value *codegen() override;
};