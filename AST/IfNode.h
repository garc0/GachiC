#pragma once

#include "BaseNode.h"

class IfNode : public BaseNode {
public:
  IfNode(std::unique_ptr<BaseNode> Cond, std::unique_ptr<BaseNode> Then,
            std::unique_ptr<BaseNode> Else = nullptr)
      : Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {}

  llvm::Value *codegen(bool is_lvalue = false) override;

private:
    std::unique_ptr<BaseNode> Cond, Then, Else;
};