#pragma once

#include "BaseNode.h"

class UnaryExprNode : public BaseNode {
  Token Opcode;
  std::unique_ptr<BaseNode> Operand;

public:
  UnaryExprNode(Token Opcode, std::unique_ptr<BaseNode> Operand)
      : Opcode(Opcode), Operand(std::move(Operand)) {}

  llvm::Value *codegen() override;
};
