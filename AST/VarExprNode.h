#pragma once

#include "BaseNode.h"

#include "BlockNode.h"

class VarExprNode : public BaseNode {
  std::vector<std::pair<std::string, std::unique_ptr<BaseNode>>> VarNames;
  
  BlockNode * l_block;
public:
  VarExprNode(BlockNode * block,
      std::vector<std::pair<std::string, std::unique_ptr<BaseNode>>> VarNames)
      : l_block(block), VarNames(std::move(VarNames)) {}

  llvm::Value * codegen(bool is_lvalue = false) override;
};