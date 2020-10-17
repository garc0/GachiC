#pragma once

#include "BaseNode.h"

class PrototypeNode {
private:

  std::string Name;
  std::vector<std::pair<std::string, llvm::Type *>> Args;

  llvm::Type * RetType = nullptr;

public:
  PrototypeNode(const std::string &Name, 
              std::vector<std::pair<std::string, llvm::Type *>> Args = {}, 
              llvm::Type *  Ret_Type = nullptr)
      : Name(std::move(Name)), Args(std::move(Args)), RetType(Ret_Type) {}

  llvm::Function *codegen();
  const std::string &getName() const { return Name; }

  auto getFnName() const {
    return Name;
  }
  
  llvm::Type * getArgType(std::size_t i){
    if(i >= Args.size()) {
      std::cout << "i >= Args.size()\n";
      return nullptr;
    };
    return this->Args[i].second;
  }

};
