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

    llvm::Value *codegen(bool is_lvalue = false) override;
private:
    std::vector<std::pair<std::string, llvm::Type *>> _elements;
    std::string _struct_name;
};


class StructExprNode : public BaseNode {
  std::string Name;
  std::vector<std::pair<std::string, std::unique_ptr<BaseNode>>> _elements;

public:
  StructExprNode(
    std::string Name, 
    std::vector<std::pair<std::string, std::unique_ptr<BaseNode>>> elems) : 
    Name(std::move(Name)),
    _elements(std::move(elems)
    ) {}

  llvm::Value *codegen(bool is_lvalue = false) override;
  const std::string &getName() const { return Name; }
};