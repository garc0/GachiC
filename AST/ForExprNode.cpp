#include "ForExprNode.h"

#include "../defs_ast.h"

llvm::Value * ForExprNode::codegen(bool is_lvalue) {
  llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();

  llvm::Value *StartVal = Start->codegen();
  if (!StartVal)
    return nullptr;

  llvm::AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, VarName, StartVal->getType());

  Builder.CreateStore(StartVal, Alloca);
  llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(TheContext, "loop", TheFunction);

  Builder.CreateBr(LoopBB);

  Builder.SetInsertPoint(LoopBB);

  llvm::AllocaInst *OldVal = std::get<0>(NamedValues[VarName]);
  std::get<0>(NamedValues[VarName]) = Alloca;

  if (!Body->codegen())
    return nullptr;

  llvm::Value *StepVal = nullptr;
  if (Step) {
    StepVal = Step->codegen();
    if (!StepVal)
      return nullptr;
  } else 
    StepVal = llvm::ConstantInt::get(TheContext, llvm::APInt(64, 1));

  llvm::Value *EndCond = End->codegen();
  if (!EndCond)
    return nullptr;


  llvm::Value *CurVar = Builder.CreateLoad(Alloca, VarName.c_str());
  llvm::Value *NextVar = Builder.CreateAdd(CurVar, StepVal, "next_var");
  Builder.CreateStore(NextVar, Alloca);

  llvm::BasicBlock *AfterBB =
      llvm::BasicBlock::Create(TheContext, "after_loop", TheFunction);

  Builder.CreateCondBr(EndCond, LoopBB, AfterBB);

  Builder.SetInsertPoint(AfterBB);

  if (OldVal)
    std::get<0>(NamedValues[VarName]) = OldVal;
  else
    NamedValues.erase(VarName);

  return llvm::Constant::getNullValue(llvm::Type::getInt64Ty(TheContext));
}

