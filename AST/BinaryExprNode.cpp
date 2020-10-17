#include "BinaryExprNode.h"

#include "../defs_ast.h"


llvm::Value *BinaryExprNode::codegen() {
    if (Op.kind() == Token::Kind::Equal) {
        VariableExprNode *LHSE = dynamic_cast<VariableExprNode *>(LHS.get());
        if (!LHSE)
            return LogErrorV("destination of '=' must be a variable");
        
        llvm::Value *Val = RHS->codegen();
        if (!Val)
            return nullptr;

        // Look up the name.
        llvm::Value *Variable = std::get<0>(NamedValues[LHSE->getName()]);
        if (!Variable)
            return LogErrorV("Unknown variable name\n");

        Builder.CreateStore(Val, Variable);
        return Val;
    }

    llvm::Value *L = LHS->codegen();
    llvm::Value *R = RHS->codegen();
    if (!L || !R)  return nullptr;

    if(Op.kind() == Token::Kind::Semicolon)
        return Builder.getInt1(false);

    if(L->getType() != R->getType()){
      std::cout << "L->getType() != R->getType()" << std::endl;
    }

    if(!L->getType()->isFloatingPointTy()){
        switch (Op.kind()) {
        case Token::Kind::Plus:
            return Builder.CreateAdd(L, R, "addu");
        case Token::Kind::Minus:
            return Builder.CreateSub(L, R, "subu");
        case Token::Kind::Asterisk:
            return Builder.CreateMul(L, R, "mulu");
        case Token::Kind::Slash:
            return Builder.CreateUDiv(L, R, "divu");
        case Token::Kind::Modulo:
            return Builder.CreateURem(L, R, "modu");
        case Token::Kind::LessThan:
            L = Builder.CreateICmpSLT(L, R, "cmpu");
            return L;
        case Token::Kind::GreaterThan:
            L = Builder.CreateICmpSLT(R, L, "cmpu");
            return L;
        default:
            break;
        }
    }else{
        switch (Op.kind()) {
        case Token::Kind::Plus:
            return Builder.CreateFAdd(L, R, "addf");
        case Token::Kind::Minus:
            return Builder.CreateFSub(L, R, "subf");
        case Token::Kind::Asterisk:
            return Builder.CreateFMul(L, R, "mulf");
        case Token::Kind::Slash:
            return Builder.CreateFDiv(L, R, "divf");
        case Token::Kind::Modulo:
            return nullptr;
        case Token::Kind::LessThan:
            return Builder.CreateFCmpOLT(L, R, "cmpf");
        case Token::Kind::GreaterThan:
            return Builder.CreateFCmpOLT(R, L, "cmpf");
        default:
            break;
        }
    }

    return nullptr;
}
