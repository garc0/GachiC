#pragma once

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"


#include <map>
#include <array>
#include <memory>
#include <string>
#include <optional>


#include "../Lexer/Lexer.h"
#include "../Lexer/Token.h"

#include "../defs_ast.h"

using namespace llvm;

class Parser{
public:
    Parser(std::unique_ptr<Lexer> lex) : _lex(std::move(lex)){
        eat();
    }
    ~Parser(){}

    Token getCurrentToken(){
        return this->_cToken;
    }

    Token eat(){
        this->_cToken = _lex->next();
        return _cToken;
    }

    std::optional<Token> expectNext(Token::Kind kind, std::string descr = ""){
        if(!_cToken.is_kind(kind)){
            std::cout << "Expected " << kind << " | " << descr <<  std::endl;
            return std::nullopt;
        }
        return this->eat();
    }

    std::unique_ptr<PrototypeNode>  parsePrototype();
    std::unique_ptr<PrototypeNode>  parseExtern();
    std::unique_ptr<FunctionNode>   parseDef();
    
    std::unique_ptr<BaseNode>       parseStruct();

    std::unique_ptr<FunctionNode>   parseTopLevelExpr(){
        auto E = parseExpression();
        if(E){
            auto Proto = std::make_unique<PrototypeNode>("__anon_expr",
                                                 std::vector<std::pair<std::string, llvm::Type *>>());
            return std::make_unique<FunctionNode>(std::move(Proto), std::move(E));
        }
        return nullptr;
    }

private:
    Token _cToken;
    std::unique_ptr<Lexer> _lex;

    std::unique_ptr<BlockNode> _cBlock = nullptr;

    std::map<std::string_view, uint16_t> _bOp = {
        {"=",   2},
        {"<",   20},
        {">",   20},
        {"+",   40},
        {"-",   40},
        {"*",   60},
        {"/",   60},
        {"%",   70},
    };

    std::unique_ptr<BaseNode> parseNumber();
    std::unique_ptr<BaseNode> parseParen();

    std::unique_ptr<BaseNode> parseBlock();

    std::unique_ptr<BaseNode> parseIdentifier();
    std::unique_ptr<BaseNode> parseVar();
    std::unique_ptr<BaseNode> parseFor();
    std::unique_ptr<BaseNode> parseIf();

    std::unique_ptr<BaseNode> parseExpression();

    std::unique_ptr<BaseNode> parseBinOpRHS(int ExprPrec, 
                                std::unique_ptr<BaseNode> LHS);

    std::unique_ptr<BaseNode> parsePrimary();
    std::unique_ptr<BaseNode> parseUnary();

    llvm::Type * parseType();

    int GetTokPrecedence(Token tok);

};