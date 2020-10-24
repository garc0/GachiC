#include "ArrayNode.h"

#include "../defs_ast.h"


llvm::Value * ArrayInitNode::codegen(bool is_lvalue){
    std::vector<llvm::Constant * > elems;

    for(auto &i : this->_elements)
        elems.push_back((llvm::Constant *)i->codegen());

    auto const_array = llvm::ConstantArray::get(llvm::ArrayType::get(elems[0]->getType(), elems.size()), 
                                llvm::ArrayRef<llvm::Constant *>(elems.data(), elems.size()) );
    
    return const_array;
}

llvm::Value * ArrayExprNode::codegen(bool is_lvalue){


    llvm::Value * _v_ptr = nullptr;
    {
        auto _variable = NamedValues.find(this->_var_name);

        if(_variable == NamedValues.end())
            return LogErrorV("Not found variable");
        
        _v_ptr = ((NamedValues[this->_var_name]).first);
    }

    auto _i = this->_index->codegen();

    llvm::Value * indexList[2] = { llvm::ConstantInt::get(TheContext, llvm::APInt(32, 0)), _i };

    if(!_v_ptr->getType()->isPointerTy())
        return LogErrorV("This is not a pointer, sucker!");

    auto _elem_ptr = Builder.CreateGEP(_v_ptr, indexList);

    if(is_lvalue) return _elem_ptr;

    return Builder.CreateLoad(_elem_ptr);
}