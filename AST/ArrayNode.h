#pragma once

#include "BaseNode.h"

#include <vector>

class ArrayExprNode : public BaseNode{
public:
    ArrayExprNode(std::string var_name_, std::unique_ptr<BaseNode> index_): 
            _var_name(std::move(var_name_)), _index(std::move(index_)){}
    ~ArrayExprNode(){}


    llvm::Value *codegen(bool is_lvalue = false) override;
private:
    std::unique_ptr<BaseNode> _index;
    std::string _var_name;
};

class ArrayInitNode : public BaseNode{
public:
    ArrayInitNode(std::vector<std::unique_ptr<BaseNode>> body_) :
        _elements(std::move(body_)){}
    ~ArrayInitNode(){}

    llvm::Value *codegen(bool is_lvalue = false) override;
private:
    llvm::Type * _type_of_elements;

    std::vector<std::unique_ptr<BaseNode>> _elements;
};