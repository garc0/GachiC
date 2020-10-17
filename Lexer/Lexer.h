#pragma once

#include <iomanip>
#include <iostream>
#include <algorithm>

#include <stdint.h>
#include <string>
#include <string_view>

#include <map>
#include <list>
#include <iterator>

#include <optional>

#include "Token.h"

class Lexer {
public:
    Lexer(const char* s) noexcept : _s(s) {
        auto tok = parse();
        _tokens.push_back(tok);
        while(tok.kind() != Token::Kind::End)
            _tokens.push_back(tok = parse());
        
        _it = _tokens.begin();
    }

    Token get() const{ return *_it;}
    Token next(){
        if((*_it).kind() == Token::Kind::End)
            return *_it;
        return *_it++;
    }

    Lexer() noexcept  {}

private:
    Token parse() noexcept;
    
    Token identifier() noexcept;
    Token number() noexcept;
    Token slash_or_comment() noexcept;

    Token getAtom() noexcept;
    Token atom(Token::Kind) noexcept;

    // is it faster than std::list?
    std::vector<Token> _tokens;
    std::vector<Token>::iterator _it;

    // raw pointer
    const char * _s = nullptr;
    std::size_t _i = 0;
};

