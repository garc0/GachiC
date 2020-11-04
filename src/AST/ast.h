#ifndef AST_H
#define AST_H
#pragma once

#include "llvm/IR/Type.h"


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
#include <variant>
#include <iostream>

#include <type_traits>

#include "../defs.h"

#include "../states.h"

#include "../Lexer/Token.h"


class Node {
public:
    enum class Type{
      Base,
      Number,
      Variable,
      VarExpr,
      Type,
      Binary,
      Unary,
      If,
      For,
      While,
      Struct,
      Prototype,
      Fucntion,
      Call,
      Block
  };

  virtual ~Node() = default;
};

class ArrayExprNode {
public:
    ArrayExprNode(std::string var_name_, std::vector<std::unique_ptr<ASTNode>> indexes_): 
            _var_name(std::move(var_name_)), _indexes(std::move(indexes_)){}


    ArrayExprNode() = delete;
    ArrayExprNode &operator=(const ArrayExprNode &) = delete;
    ArrayExprNode(const ArrayExprNode&) = delete;
    ArrayExprNode(ArrayExprNode &&) = default;
    ArrayExprNode &operator=(ArrayExprNode &&) = default;
    ~ArrayExprNode() = default;

    std::vector<std::unique_ptr<ASTNode>> _indexes;
    std::string _var_name;
};

class ArrayInitNode {
public:
  ArrayInitNode(std::vector<std::unique_ptr<ASTNode>> body_) :
      _elements(std::move(body_)){}
    
    
  ArrayInitNode() = delete;
  ArrayInitNode &operator=(const ArrayInitNode &) = delete;
  ArrayInitNode(const ArrayInitNode&) = delete;
  ArrayInitNode(ArrayInitNode &&) = default;
  ArrayInitNode &operator=(ArrayInitNode &&) = default;
  ~ArrayInitNode() = default;

  std::unique_ptr<ASTNode> _type_of_elements;

  std::vector<std::unique_ptr<ASTNode>> _elements;
};

class BinaryExprNode  {
public:
  BinaryExprNode(Token Op, std::unique_ptr<ASTNode> LHS,
                std::unique_ptr<ASTNode> RHS)
      : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

  BinaryExprNode() = delete;
  BinaryExprNode &operator=(const BinaryExprNode &) = delete;
  BinaryExprNode(const BinaryExprNode&) = delete;
  BinaryExprNode(BinaryExprNode &&) = default;
  BinaryExprNode &operator=(BinaryExprNode &&) = default;
  ~BinaryExprNode() = default;


  Token Op;
  std::unique_ptr<ASTNode> LHS, RHS;
};

class BlockNode {
public:
  BlockNode(std::vector<std::unique_ptr<ASTNode>> exprs = std::vector<std::unique_ptr<ASTNode>>{}): l(std::move(exprs)){}

  BlockNode &operator=(const BlockNode &) = delete;
  BlockNode(const BlockNode&) = delete;
  BlockNode(BlockNode &&) = default;
  BlockNode &operator=(BlockNode &&) = default;
  ~BlockNode() = default;

  std::vector<llvm::AllocaInst *> OldBindings;
  std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> VarNames;

  std::vector<std::unique_ptr<ASTNode>> l;

};

class CallExprNode  {
public:
  CallExprNode(const std::string &Callee,
              std::vector<std::unique_ptr<ASTNode>> Args)
      : Callee(Callee), Args(std::move(Args)) {}

  CallExprNode() = delete;
  CallExprNode &operator=(const CallExprNode &) = delete;
  CallExprNode(const CallExprNode&) = delete;
  CallExprNode(CallExprNode &&) = default;
  CallExprNode &operator=(CallExprNode &&) = default;
  ~CallExprNode() = default;

  std::string Callee;
  std::vector<std::unique_ptr<ASTNode>> Args;
};

class ForExpr  {
public:
  ForExpr(const std::string &VarName, std::unique_ptr<ASTNode> Start,
             std::unique_ptr<ASTNode> End, std::unique_ptr<ASTNode> Step,
             std::unique_ptr<ASTNode> Body)
      : VarName(VarName), Start(std::move(Start)), End(std::move(End)),
        Step(std::move(Step)), Body(std::move(Body)) {}

  

  ForExpr() = delete;
  ForExpr &operator=(const ForExpr &) = delete;
  ForExpr(const ForExpr&) = delete;
  ForExpr(ForExpr &&) = default;
  ForExpr &operator=(ForExpr &&) = default;
  ~ForExpr() = default;

  std::string VarName;
  std::unique_ptr<ASTNode> Start, End, Step, Body;
};

class FunctionNode {
public:
  FunctionNode(std::unique_ptr<DefNode> Proto,
              std::unique_ptr<ASTNode> Body)
      : Proto(std::move(Proto)), Body(std::move(Body)) {}

  FunctionNode() = delete;
  FunctionNode &operator=(const FunctionNode &) = delete;
  FunctionNode(const FunctionNode&) = delete;
  FunctionNode(FunctionNode &&) = default;
  FunctionNode &operator=(FunctionNode &&) = default;
  ~FunctionNode() = default;


  auto & getProto(){
    return *this->Proto.get();
  }

  std::unique_ptr<DefNode> getProtoPtr(){
    return std::move(Proto);
  }

  auto * getBody(){
    return this->Body.get();
  }


  std::unique_ptr<DefNode> Proto;
  std::unique_ptr<ASTNode> Body;
};

class IfExpr  {
public:
  IfExpr(std::unique_ptr<ASTNode> Cond, std::unique_ptr<ASTNode> Then,
            std::unique_ptr<ASTNode> Else)
      : Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {}

  //llvm::Value *codegen(bool is_lvalue = false) override;
  IfExpr() = delete;
  IfExpr &operator=(const IfExpr &) = delete;
  IfExpr(const IfExpr&) = delete;
  IfExpr(IfExpr &&) = default;
  IfExpr &operator=(IfExpr &&) = default;
  ~IfExpr() = default;

  std::unique_ptr<ASTNode> Cond, Then;

  std::unique_ptr<ASTNode> Else;
};

class NumberExprNode  {
public:
  NumberExprNode(std::string val_)
    : _val(val_) {}

  NumberExprNode() = delete;
  NumberExprNode &operator=(const NumberExprNode &) = delete;
  NumberExprNode(const NumberExprNode&) = delete;
  NumberExprNode(NumberExprNode &&) = default;
  NumberExprNode &operator=(NumberExprNode &&) = default;
  ~NumberExprNode() = default;

  std::string _val;
};

class CharNode  {
public:
  CharNode(unsigned char val_)
    : _val(val_) {}

  CharNode() = delete;
  CharNode &operator=(const CharNode &) = delete;
  CharNode(const CharNode&) = delete;
  CharNode(CharNode &&) = default;
  CharNode &operator=(CharNode &&) = default;
  ~CharNode() = default;

  unsigned char _val;
};

class StringNode  {
public:
  StringNode(std::string val_)
    : _val(std::move(val_)) {}

  StringNode() = delete;
  StringNode &operator=(const StringNode &) = delete;
  StringNode(const StringNode&) = delete;
  StringNode(StringNode &&) = default;
  StringNode &operator=(StringNode &&) = default;
  ~StringNode() = default;

  std::string _val;
};

class StickNode {
public:
  StickNode(ASTNode * block_, 
        std::pair<std::string, std::unique_ptr<ASTNode>> val_, 
        bool to_alloc_ = true
      )
    : _block(block_), _val(std::move(val_)), _to_alloc(to_alloc_) {}

  StickNode() = delete;
  StickNode &operator=(const StickNode &) = delete;
  StickNode(const StickNode&) = delete;
  StickNode(StickNode &&) = default;
  StickNode &operator=(StickNode &&) = default;
  ~StickNode() = default;

  std::pair<std::string, std::unique_ptr<ASTNode>> _val;
  ASTNode * _block;

  bool _to_alloc = true;
};

class TypeNode  {
public:

  enum class type_id{
    pointer,

    nothing,

    u1,

    u8,
    u16,
    u32,
    u64,

    i8,
    i16,
    i32,
    i64,

    f8,
    f16,
    f32,
    f64,

    struct_type,
    array,
  }; 

  using type_pair = std::pair<type_id, std::string>;

  TypeNode(std::vector<type_pair> t)
    : _t(std::move(t)) {}

  TypeNode() = delete;
  TypeNode &operator=(const TypeNode &) = delete;
  TypeNode(const TypeNode&) = delete;
  TypeNode(TypeNode &&) = default;
  TypeNode &operator=(TypeNode &&) = default;
  ~TypeNode() = default;

  std::vector<type_pair> _t;
};

class PrototypeNode {
public:
  PrototypeNode(const std::string &Name, 
              std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> Args, 
              std::unique_ptr<ASTNode> Ret_Type)
      : Name(std::move(Name)), Args(std::move(Args)), RetType(std::move(Ret_Type)) {}

  PrototypeNode() = delete;

  PrototypeNode &operator=(const PrototypeNode &) = delete;
  PrototypeNode(const PrototypeNode &) = delete;
  PrototypeNode(PrototypeNode &&) = default;
  PrototypeNode &operator=(PrototypeNode &&) = default;

  ~PrototypeNode() = default;

  const std::string &getName() const { return Name; }

  const std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> &getArgs() const {
     return Args; 
  }

  ASTNode * getTypeRet(){
    return RetType.get();
  }

  ASTNode* getArgType(std::size_t i){
    if(i >= Args.size()) {
      std::cout << "i >= Args.size()\n";
      return nullptr;
    };
    return this->Args[i].second.get();
  }


  std::string Name;
  std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> Args;

  std::unique_ptr<ASTNode> RetType = nullptr;

};

class StructNode {  
public:

  StructNode(std::string struct_name, 
      std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> elems)
      : _struct_name(std::move(struct_name)), _elements(std::move(elems))
          {}

  StructNode() = delete;

  StructNode &operator=(const StructNode &) = delete;
  StructNode(const StructNode &) = delete;
  StructNode(StructNode &&) = default;
  StructNode &operator=(StructNode &&) = default;

  ~StructNode() = default;

  std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> _elements;
  std::string _struct_name;
};

class StructExprNode  {

public:
  StructExprNode(
    std::string Name, 
    std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> elems)
      : Name(std::move(Name)), _elements(std::move(elems)) {}

  StructExprNode() = delete;

  StructExprNode &operator=(const StructExprNode &) = delete;
  StructExprNode(const StructExprNode&) = delete;
  StructExprNode(StructExprNode &&) = default;
  StructExprNode &operator=(StructExprNode &&) = default;

  ~StructExprNode() = default;

  const std::string &getName() const { return Name; }

  std::string Name;
  std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> _elements;

};

class UnaryExprNode {
public:
  UnaryExprNode(Token Opcode, std::unique_ptr<ASTNode> Operand)
      : Opcode(Opcode), Operand(std::move(Operand)) {}

  UnaryExprNode() = delete;

  UnaryExprNode &operator=(const UnaryExprNode &) = delete;
  UnaryExprNode(const UnaryExprNode&) = delete;
  UnaryExprNode(UnaryExprNode &&) = default;
  UnaryExprNode &operator=(UnaryExprNode &&) = default;

  ~UnaryExprNode() = default;

  Token Opcode;
  std::unique_ptr<ASTNode> Operand;
};

class VarExprNode {
public:
  VarExprNode(ASTNode * block,
      std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> VarNames)
      : l_block(block), VarNames(std::move(VarNames)) {}

  VarExprNode() = delete;
  VarExprNode &operator=(const VarExprNode &) = delete;
  VarExprNode(const VarExprNode&) = delete;
  VarExprNode(VarExprNode &&) = default;
  VarExprNode &operator=(VarExprNode &&) = default;
  ~VarExprNode() = default;

  std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> VarNames;
  
  ASTNode * l_block;
};

class VariableExprNode  {
public:
  VariableExprNode(const std::string &Name) : Name(Name) {}

  VariableExprNode() = delete;

  VariableExprNode &operator=(const VariableExprNode &) = delete;
  VariableExprNode(const VariableExprNode&) = delete;
  VariableExprNode(VariableExprNode &&) = default;
  VariableExprNode &operator=(VariableExprNode &&) = default;

  ~VariableExprNode() = default;

  const std::string &getName() const { return Name; }

  std::string Name;
};

class WhileExpr {
public:

    WhileExpr(std::unique_ptr<ASTNode> cond_, std::optional<std::unique_ptr<ASTNode>> step_,
              std::unique_ptr<ASTNode> body_)
                :
        _cond(std::move(cond_)), _step(std::move(step_)), _body(std::move(body_)){}

    WhileExpr() = delete;

    WhileExpr &operator=(const WhileExpr &) = delete;
    WhileExpr(const WhileExpr &) = delete;
    WhileExpr(WhileExpr &&) = default;
    WhileExpr &operator=(WhileExpr &&) = default;

    ~WhileExpr() = default;
    
    std::unique_ptr<ASTNode> _cond, _body;

    std::optional<std::unique_ptr<ASTNode>> _step;
};


template <class T, typename... Args>
std::unique_ptr<ASTNode> make_node(Args&&... args) {
  return std::make_unique<ASTNode>(T{std::forward<Args>(args)...});
}

template <class T, typename... Args>
std::unique_ptr<DefNode> make_def(Args&&... args) {
  return std::make_unique<DefNode>(T{std::forward<Args>(args)...});
}

#include "../CodeGen/Visitor.h"


static std::unique_ptr<ASTNode> LogError(std::string_view Str) {
  std::cerr << "Error: " << (Str) << std::endl;
  return nullptr;
}

static llvm::Value * LogErrorV(std::string_view Str) {
  LogError(Str);
  return nullptr;
}

extern llvm::Function * getFunction(std::string Name);

// copypasta
extern llvm::AllocaInst * CreateEntryBlockAlloca(llvm::Function *TheFunction,
                                          llvm::StringRef VarName, llvm::Type* type);

#endif