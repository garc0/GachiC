#include "UnaryExprNode.h"

#include "../defs_ast.h"

llvm::Value *UnaryExprNode::codegen(bool is_lvalue) {

  if(this->Opcode.kind() == Token::Kind::Return){
    llvm::Value * OperandV = Operand->codegen();
    return Builder.CreateRet(OperandV);
  }

  if(this->Opcode.kind() == Token::Kind::Ampersand){
    return Operand->codegen(true);
  }

  if(this->Opcode.kind() == Token::Kind::Asterisk){
    llvm::Value * OperandV = Operand->codegen();
    return Builder.CreateLoad(OperandV);
  }

  return nullptr;
}