#ifndef STATES_H
#define STATES_H
#pragma once


#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"


#include <map>
#include <memory>

#include "AST/ast.h"


extern llvm::LLVMContext TheContext;
extern llvm::IRBuilder<> Builder;

extern std::unique_ptr<llvm::Module> TheModule;

extern std::map<std::string, llvm::Type*> NamedStructures;
extern std::map<llvm::Type *, std::vector<std::pair<std::string, llvm::Type *>>> StructFields;

extern std::map<std::string, std::pair<llvm::AllocaInst *, llvm::Type*>> NamedValues;
extern std::map<std::string, std::unique_ptr<DefNode>> FunctionProtos;

#endif