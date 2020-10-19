#include "NumberExprNode.h"

#include <limits>

#include "../states.h"

template<typename T>
inline bool is_in_range(T v){
    return (v > std::numeric_limits<T>::lowest()) && (v < std::numeric_limits<T>::max());
}

llvm::Value * NumberExprNode::codegen(){
    bool is_float = this->_val.find('.') != std::string::npos;

    if(is_float){
        auto d = std::stod(this->_val);

        if(is_in_range(float(d))) return llvm::ConstantFP::get(TheContext, llvm::APFloat(float(d)));
        if(is_in_range(double(d))) return llvm::ConstantFP::get(TheContext, llvm::APFloat(double(d)));

        return nullptr;
    }
    
    auto i = std::stoll(this->_val);
    if(is_in_range<uint8_t>(uint8_t(i))) return llvm::ConstantInt::get(TheContext, llvm::APInt(8, i));
    if(is_in_range<uint16_t>(uint16_t(i))) return llvm::ConstantInt::get(TheContext, llvm::APInt(16, i));
    if(is_in_range<uint32_t>(uint32_t(i))) return llvm::ConstantInt::get(TheContext, llvm::APInt(32, i));
    if(is_in_range<uint64_t>(uint64_t(i))) return llvm::ConstantInt::get(TheContext, llvm::APInt(64, i));
    return nullptr;
}
