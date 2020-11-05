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

#include <vector>
#include <optional>

#include "Token.h"

class Lexer {
public:

    Lexer() noexcept {
        _it = _tokens.begin();
    }

    void analyze(std::string_view s){
        _line++;
        if(s == "") return;
        this->_s = std::move(s);
        this->_i = 0;

        auto tok = parse();

        tok.set_line(_line);
        tok.set_pos(_i - tok.lexeme().length() + 1);

        if(!tok.is_kind(Token::Kind::Comment))
            _tokens.push_back(tok);
        while(tok.kind() != Token::Kind::End && this->_i < this->_s.size())
        {
            tok = parse();

            tok.set_line(_line);
            tok.set_pos(_i - tok.lexeme().length() + 1);

            if(!tok.is_kind(Token::Kind::Comment))
                _tokens.push_back(tok);
        }

        _it = _tokens.begin();
        return;
    }

    Token get() const { return *_it; }

    Token next(){
        if((*_it).kind() == Token::Kind::End)
            return *_it;
        return *_it++;
    }

    Token to_begin(){
        return *(_it = _tokens.begin());
    }

    void set_end(){
        _tokens.push_back(Token());
    }

private:
    Token parse() noexcept;
    
    Token identifier() noexcept;
    Token number() noexcept;
    Token slash_or_comment() noexcept;
    Token parse_char() noexcept;
    Token parse_string() noexcept;

    Token getAtom() noexcept;
    Token atom(Token::Kind) noexcept;

    // is it faster than std::list?
    std::vector<Token> _tokens;
    std::vector<Token>::iterator _it;

    std::string_view _s;

    std::size_t _line = 0;
    std::size_t _i = 0;
};

