#include "NumberExprNode.h"

#include "../states.h"

llvm::Value * NumberExprNode::codegen(){
    return llvm::ConstantInt::get(TheContext, llvm::APInt(64, Val, true));
}
