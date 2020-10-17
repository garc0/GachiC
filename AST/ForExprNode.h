#pragma once

#include "BaseNode.h"

class ForExprNode : public BaseNode {
  std::string VarName;
  std::unique_ptr<BaseNode> Start, End, Step, Body;

public:
  ForExprNode(const std::string &VarName, std::unique_ptr<BaseNode> Start,
             std::unique_ptr<BaseNode> End, std::unique_ptr<BaseNode> Step,
             std::unique_ptr<BaseNode> Body)
      : VarName(VarName), Start(std::move(Start)), End(std::move(End)),
        Step(std::move(Step)), Body(std::move(Body)) {}

  llvm::Value *codegen() override;
};
