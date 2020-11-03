#ifndef VISITOR_H
#define VISITOR_H
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


#include <map>
#include <array>
#include <memory>
#include <string>
#include <optional>

#include "../defs.h"


using namespace llvm;

struct lvalue{};
struct rvalue{};


class VisitorFunction{
public:
    VisitorFunction(){}
    ~VisitorFunction(){}

    llvm::Function * operator()(std::nullptr_t &);
    llvm::Function * operator()(PrototypeNode &);
    llvm::Function * operator()(FunctionNode &);

private:
};
class VisitorExpr{
public:
    VisitorExpr(){}
    ~VisitorExpr(){}

    template<class T> llvm::Value * operator()(std::nullptr_t &, T &);
    template<class T> llvm::Value * operator()(ArrayExprNode &, T &);
    template<class T> llvm::Value * operator()(ArrayInitNode &, T &);
    template<class T> llvm::Value * operator()(BinaryExprNode &, T &);
    template<class T> llvm::Value * operator()(BlockNode &, T &);
    template<class T> llvm::Value * operator()(ForExpr &, T &);
    template<class T> llvm::Value * operator()(IfExpr &, T &);
    template<class T> llvm::Value * operator()(CallExprNode &, T &);
    template<class T> llvm::Value * operator()(NumberExprNode &, T &);
    template<class T> llvm::Value * operator()(CharNode &, T &);
    template<class T> llvm::Value * operator()(StringNode &, T &);
    template<class T> llvm::Value * operator()(StructNode &, T &);
    template<class T> llvm::Value * operator()(StructExprNode &, T &);
    template<class T> llvm::Value * operator()(UnaryExprNode &, T &);
    template<class T> llvm::Value * operator()(VarExprNode &, T &);
    template<class T> llvm::Value * operator()(VariableExprNode &, T &);
    template<class T> llvm::Value * operator()(WhileExpr &, T &);
    template<class T> llvm::Value * operator()(TypeNode &, T &);

    template<class RL = rvalue, class T>
    auto expr_visit(T &node){
        std::variant<lvalue, rvalue> rl;
        if constexpr(std::is_same<RL, rvalue>())
            rl = rvalue();
        else
            rl = lvalue();
            
        return std::visit(*this, node, rl);
    }
private:
};

#endif