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
#include <functional>
#include <sstream>

#include "../Lexer/Lexer.h"
#include "../Lexer/Token.h"

#include "../AST/ast.h"

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

    auto log_err(std::string m){
        std::cerr << m << std::endl;

        std::cerr << "line = " << _cToken.get_line() << " | pos = " << _cToken.get_pos() << std::endl; 

        return nullptr;
    }

    std::optional<Token> expectNext(Token::Kind kind, std::string descr = ""){
        if(!_cToken.is_kind(kind)){

            std::stringstream ss;
            ss << "Expected " << kind << " | " << descr;
            log_err(ss.str());
            return std::nullopt;
        }
        return this->eat();
    }

    std::unique_ptr<DefNode>    parsePrototype();
    std::unique_ptr<DefNode>    parseExtern();
    std::unique_ptr<DefNode>    parseDef();

    std::unique_ptr<DefNode>    parseMain();
    
    std::unique_ptr<ASTNode>    parseStruct();

    std::unique_ptr<DefNode>    parseTopLevelExpr();

private:
    Token _cToken;
    std::unique_ptr<Lexer> _lex;

    std::unique_ptr<ASTNode> _cBlock = make_node<BlockNode>();

    std::vector<std::string> _bOp = {
        "=",
        "&&",
        "||",
        "==",
        "!=",
        "<",
        ">",
        "<=",
        ">=",
        "+",
        "-",
        "*",
        "/",
        "%",
        "ass",
        ".",
    };

    std::unique_ptr<ASTNode> parseNumber();
    std::unique_ptr<ASTNode> parseParen();

    std::unique_ptr<ASTNode> parseBlock();

    std::unique_ptr<ASTNode> parseIdentifier();
    std::unique_ptr<ASTNode> parseChar();
    std::unique_ptr<ASTNode> parseString();
    std::unique_ptr<ASTNode> parseArray();
    std::unique_ptr<ASTNode> parseVar();
    std::unique_ptr<ASTNode> parseStick();
    std::unique_ptr<ASTNode> parseFor();
    std::unique_ptr<ASTNode> parseWhile();
    std::unique_ptr<ASTNode> parseIf();

    std::unique_ptr<ASTNode> parseExpression();

    std::unique_ptr<ASTNode> parseBinOpRHS(int32_t ExprPrec, 
                                std::unique_ptr<ASTNode> LHS);

    std::unique_ptr<ASTNode> parsePrimary();
    std::unique_ptr<ASTNode> parseUnary();

    std::unique_ptr<ASTNode> parseType();

    int32_t GetTokPrecedence(Token tok);

};