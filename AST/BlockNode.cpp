#include "BlockNode.h"

#include "../states.h"

llvm::Value * BlockNode::codegen(){
    

    //Builder.GetInsertBlock()->Create(TheContext, "", Builder.GetInsertBlock()->getParent());

    //llvm::Function * TheFunction = Builder.GetInsertBlock()->getParent();

    //llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext, "", TheFunction);
    //Builder.SetInsertPoint(BB);

    llvm::Value * ValRet = nullptr;

    for(auto &i : this->l){
        if(!(ValRet = i->codegen()))
            return nullptr;
    }

    for (unsigned i = 0, e = VarNames.size(); i != e; ++i)
      std::get<0>(NamedValues[VarNames[i].first]) = OldBindings[i];

    return ValRet;
}