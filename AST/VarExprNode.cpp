#include "VarExprNode.h"

#include <vector>

#include<algorithm>  
#include<iterator> 

#include "../defs_ast.h"

llvm::Value * VarExprNode::codegen() {
  llvm::Function * TheFunction = Builder.GetInsertBlock()->getParent();

  llvm::Value * ValToRet;
  
  for (unsigned i = 0, e = VarNames.size(); i != e; ++i) {
    const std::string &VarName = VarNames[i].first;
    BaseNode *Init = VarNames[i].second.get();

    llvm::Value * InitVal;
    if (Init) {
      if (!InitVal)
        return nullptr;
    } else 
      InitVal = llvm::ConstantInt::get(TheContext, llvm::APInt(64, (uint64_t)0));

    ValToRet = InitVal = Init->codegen();

    llvm::AllocaInst * Alloca = CreateEntryBlockAlloca(TheFunction, "", InitVal->getType());

    Builder.CreateStore(InitVal, Alloca);

    this->l_block->OldBindings.push_back(std::get<0>(NamedValues[VarName]));

    std::get<0>(NamedValues[VarName])= Alloca;
  }

  this->l_block->VarNames = std::move(this->VarNames);

  return ValToRet;
}