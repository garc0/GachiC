#include "FunctionNode.h"

#include "../defs_ast.h"
#include "../states.h"

llvm::Function * FunctionNode::codegen(bool is_lvalue) {
  auto &P = *Proto;
  FunctionProtos[Proto->getName()] = std::move(Proto);
  llvm::Function *TheFunction = getFunction(P.getName());
  if (!TheFunction)
    return nullptr;

  llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext, "entry", TheFunction);
  Builder.SetInsertPoint(BB);

  // Record the function arguments in the NamedValues map.
  NamedValues.clear();
  for (auto &Arg : TheFunction->args()) {
    // Create an alloca for this variable.

    llvm::AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, Arg.getName(),
     FunctionProtos[P.getName()]->getArgType(Arg.getArgNo()));

    // Store the initial value into the alloca.
    Builder.CreateStore(&Arg, Alloca);
    // Add arguments to variable symbol table.
    std::get<0>(NamedValues[std::string(Arg.getName())]) = Alloca;
  }

  if(!Body){
    std::cout << "What the hell" << std::endl;
    return nullptr;
  }

  if (llvm::Value * RetVal = Body->codegen()) {
    verifyFunction(*TheFunction);
    return TheFunction;
  }


  // Error reading body, remove function.
  TheFunction->eraseFromParent();

  return nullptr;
}
