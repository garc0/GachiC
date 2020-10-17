#pragma once 

#include "BaseNode.h"

#include <vector>

class BlockNode : public BaseNode{
public:
    BlockNode(std::vector<std::unique_ptr<BaseNode>> exprs = std::vector<std::unique_ptr<BaseNode>>{}): l(std::move(exprs)){}

    std::vector<llvm::AllocaInst *> OldBindings;
    std::vector<std::pair<std::string, std::unique_ptr<BaseNode>>> VarNames;

    std::vector<std::unique_ptr<BaseNode>> l;

    llvm::Value * codegen() override;
};