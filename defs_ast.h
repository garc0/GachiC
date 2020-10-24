#pragma once

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "AST/BaseNode.h"
#include "AST/VariableExprNode.h"
#include "AST/NumberExprNode.h"
#include "AST/UnaryExprNode.h"
#include "AST/BinaryExprNode.h"
#include "AST/CallExprNode.h"
#include "AST/VarExprNode.h"
#include "AST/IfNode.h"
#include "AST/ForExprNode.h"
#include "AST/PrototypeNode.h"
#include "AST/FunctionNode.h"
#include "AST/BlockNode.h"
#include "AST/StructNode.h"
#include "AST/ArrayNode.h"

#include "states.h"

static std::unique_ptr<BaseNode> LogError(const char *Str) {
  fprintf(stderr, "Error: %s \n", Str);
  return nullptr;
}

static llvm::Value * LogErrorV(const char *Str) {
  LogError(Str);
  return nullptr;
}

static llvm::Function * getFunction(std::string Name) {
  if (auto *F = TheModule->getFunction(Name))
    return F;


  auto FI = FunctionProtos.find(Name);
  if (FI != FunctionProtos.end())
    return FI->second->codegen();

  return nullptr;
}

// copypasta
static llvm::AllocaInst * CreateEntryBlockAlloca(llvm::Function *TheFunction,
                                          llvm::StringRef VarName, llvm::Type* type) {
  llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                   TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(type, nullptr, VarName);
}
