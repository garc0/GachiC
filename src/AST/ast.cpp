#include "ast.h"

#include "../states.h"

llvm::Function * getFunction(std::string Name) {
  if (auto *F = TheModule->getFunction(Name))
    return F;


  auto FI = FunctionProtos.find(Name);
  if (FI != FunctionProtos.end())
    return std::visit(VisitorFunction{}, *(FI->second).get());

  return nullptr;
}

llvm::AllocaInst * CreateEntryBlockAlloca(llvm::Function *TheFunction,
                                          llvm::StringRef VarName, llvm::Type* type) {
  llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                   TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(type, nullptr, VarName);
}