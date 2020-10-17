#include "StructNode.h"

#include <vector>

#include "../states.h"

llvm::Value * StructNode::codegen(){
    
    auto struct_val = llvm::StructType::create(TheContext, this->_struct_name.c_str());

    std::vector<llvm::Type * > struct_body;

    for(auto & i: this->_elements)
        struct_body.push_back(i.second);

    struct_val->setBody(struct_body);

    return (llvm::Value *)struct_val;
}