#include "StructNode.h"

#include <vector>

#include "../states.h"

llvm::Value * StructNode::codegen(){
    
    auto struct_val = llvm::StructType::create(TheContext, this->_struct_name.c_str());

    NamedStructures[this->_struct_name] = (llvm::Type * )struct_val;
    StructFields[struct_val] = _elements;

    std::vector<llvm::Type * > struct_body;

    for(auto & i: this->_elements){
        struct_body.push_back(i.second);
    }

    struct_val->setBody(struct_body);


    return (llvm::Value * )struct_val;
}

llvm::Value * StructExprNode::codegen(){

    if(NamedStructures.find(this->Name) == NamedStructures.end()){
        std::cout << "Invalid structure name" << std::endl;
        return nullptr;
    }

    std::vector<llvm::Constant *> fields;

    for(auto & i : this->_elements)
        fields.push_back((llvm::Constant *)(i.second->codegen()));
    
    return llvm::ConstantStruct::get((llvm::StructType*)NamedStructures[this->Name], llvm::ArrayRef<llvm::Constant *>(fields.data(), fields.size()));
}