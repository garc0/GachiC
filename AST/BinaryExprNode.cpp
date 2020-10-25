#include "BinaryExprNode.h"

#include "../defs_ast.h"


llvm::Value * BinaryExprNode::codegen(bool is_lvalue) {
    if (Op.kind() == Token::Kind::Equal) {

        llvm::Value * Variable = LHS->codegen(true);    

        if(!Variable){
            std::cout << "lvalue is not valid" << std::endl;
            return nullptr;
        }

        if(!Variable->getType()->isPointerTy()){
            std::cout << "lvalue is not valid" << std::endl;
            return nullptr;
        }

        llvm::Value * Val = RHS->codegen();
        if (!Val)
            return nullptr;

        if (!Variable)
            return LogErrorV("Unknown variable name\n");

        Builder.CreateStore(Val, Variable);
        return Val;
    }


    if(Op.kind() == Token::Kind::Dot){
        
        llvm::Value * variable = LHS->codegen(true);

        if (!variable)
            return LogErrorV("Unknown variable name\n");

        llvm::Type * type_struct = variable->getType();

        llvm::Type * type_struct_p = type_struct;

        if(type_struct->isPointerTy()){
            type_struct_p = type_struct->getPointerElementType();
        }

        if(StructFields.find(type_struct_p) == StructFields.end())
            return LogErrorV("Not found a struct def");

        auto struct_def = StructFields[type_struct_p];


        VariableExprNode * RHSE = static_cast<VariableExprNode *>(RHS.get());
        if (!RHSE)
            return LogErrorV("Oops, what the hell\n");

        auto name_var = RHSE->getName();

        std::size_t i = 0;

        while(i < struct_def.size() && name_var != struct_def[i].first) i++;

        if(i == struct_def.size())
            return LogErrorV("Not found a field of structure");
        
        llvm::Value * indexList[2] = {llvm::ConstantInt::get(TheContext, llvm::APInt(32, 0)), llvm::ConstantInt::get(TheContext, llvm::APInt(32, i))};

        auto elem_ptr = Builder.CreateGEP(variable, indexList);

        if(is_lvalue) return elem_ptr;
        return Builder.CreateLoad(elem_ptr);
    }


    if(Op.kind() == Token::Kind::Ass){

    }

    llvm::Value * L = LHS->codegen();
    llvm::Value * R = RHS->codegen();
    if (!L || !R)  return nullptr;


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
            return Builder.CreateICmpSLT(L, R, "cmpu");
        case Token::Kind::GreaterThan:
            return Builder.CreateICmpSLT(R, L, "cmpu");
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
