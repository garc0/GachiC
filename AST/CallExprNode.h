#pragma once

#include "BaseNode.h"


/// CallExprNode - Expression class for Function calls.
class CallExprNode : public BaseNode {
  std::string Callee;
  std::vector<std::unique_ptr<BaseNode>> Args;

public:
  CallExprNode(const std::string &Callee,
              std::vector<std::unique_ptr<BaseNode>> Args)
      : Callee(Callee), Args(std::move(Args)) {}

  llvm::Value * codegen(bool is_lvalue = false) override;
};