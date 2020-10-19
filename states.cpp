#include "states.h"

llvm::LLVMContext TheContext;
llvm::IRBuilder<> Builder(TheContext);

std::unique_ptr<llvm::Module> TheModule;

std::map<std::string, llvm::Type*> NamedStructures;
std::map<llvm::Type *, std::vector<std::pair<std::string, llvm::Type *>>> StructFields;

std::map<std::string, std::pair<llvm::AllocaInst *, llvm::Type *>> NamedValues;
std::map<std::string, std::unique_ptr<PrototypeNode>> FunctionProtos;