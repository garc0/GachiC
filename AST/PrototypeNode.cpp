#include "PrototypeNode.h"

#include <vector>

#include "../defs_ast.h"

llvm::Function * PrototypeNode::codegen(bool is_lvalue) {
  std::vector<llvm::Type *> f_args;
  for(auto i: this->Args){
    f_args.push_back(std::get<1>(i));
  }

  llvm::FunctionType *FT =
      llvm::FunctionType::get(this->RetType, f_args, false);

  llvm::Function *F =
      llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Name, TheModule.get());

  // Set names for all arguments.
  unsigned Idx = 0;
  for (auto &Arg : F->args())
    Arg.setName(std::get<0>(Args[Idx++]));

  return F;
}