#pragma once

#include "BaseNode.h"

#include "../states.h"

class NumberExprNode : public BaseNode {
private:
  int64_t Val;
  
  llvm::Type * type_val;

public:
  NumberExprNode(double_t Val, llvm::Type * t = llvm::Type::getInt64Ty(TheContext)) : Val(Val), type_val(t) {}

  llvm::Value *codegen() override;
};