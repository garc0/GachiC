#pragma once

#include "BaseNode.h"

#include <map>
#include <string>

class StructNode : public BaseNode{  
public:

    StructNode(std::string struct_name, 
        std::vector<std::pair<std::string, llvm::Type *>> elems):
            _struct_name(std::move(struct_name)), _elements(std::move(elems))
            {}
    ~StructNode(){}

    llvm::Value *codegen() override;
private:
    std::vector<std::pair<std::string, llvm::Type *>> _elements;
    std::string _struct_name;
};