#pragma once

#include <vector>

#include "BaseNode.h"

class TypeNode : public BaseNode{
public:
    TypeNode(std::vector<std::string> stype_) : _stype(std::move(stype_)) {}
    ~TypeNode(){}

    llvm::Value * codegen(bool is_lvlalue = false) override;
private:

    std::vector<std::string> _stype;

};