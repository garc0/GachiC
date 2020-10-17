#include "UnaryExprNode.h"

#include "../defs_ast.h"

llvm::Value *UnaryExprNode::codegen() {
  llvm::Value *OperandV = Operand->codegen();
  if (!OperandV)
    return nullptr;

  if(this->Opcode.kind() == Token::Kind::Return){
    return Builder.CreateRet(OperandV);
  }

  return nullptr;
}