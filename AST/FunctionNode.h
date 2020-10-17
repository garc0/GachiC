#pragma once

#include "BaseNode.h"

#include "../defs_ast.h"

class FunctionNode {
  std::unique_ptr<PrototypeNode> Proto;
  std::unique_ptr<BaseNode> Body;

public:
  FunctionNode(std::unique_ptr<PrototypeNode> Proto,
              std::unique_ptr<BaseNode> Body)
      : Proto(std::move(Proto)), Body(std::move(Body)) {}

  llvm::Function *codegen();
};