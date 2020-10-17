#pragma once

#include "BaseNode.h"


/// BinaryExprNode - Expression class for a binary operator.
class BinaryExprNode : public BaseNode {
  Token Op;
  std::unique_ptr<BaseNode> LHS, RHS;

public:
  BinaryExprNode(Token Op, std::unique_ptr<BaseNode> LHS,
                std::unique_ptr<BaseNode> RHS)
      : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

  llvm::Value *codegen() override;
};