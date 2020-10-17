#include "IfNode.h"

#include "../states.h"

llvm::Value *IfNode::codegen() {

  llvm::Value *CondV = Cond->codegen();
  if (!CondV)
    return nullptr;


  llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();

  llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(TheContext, "then", TheFunction);
  llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(TheContext, "else");
  llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(TheContext, "if_cont");

  Builder.CreateCondBr(CondV, ThenBB, ElseBB);

  Builder.SetInsertPoint(ThenBB);

  llvm::Value *ThenV = Then->codegen();

  if (!ThenV)
    return nullptr;

  Builder.CreateBr(MergeBB);
  
  
  TheFunction->getBasicBlockList().push_back(ElseBB);
  Builder.SetInsertPoint(ElseBB);

  llvm::Value *ElseV = Else->codegen();
  if (!ElseV)
    return nullptr;

  Builder.CreateBr(MergeBB);

  TheFunction->getBasicBlockList().push_back(MergeBB);
  Builder.SetInsertPoint(MergeBB);


  return llvm::Constant::getNullValue(llvm::Type::getInt64Ty(TheContext));
}
