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


class Token {
public:
    enum class Kind {
        Number,
        Identifier,
        
        Var,
        Stick,
        Your,
        In,
        My,
        Out,
        Cum,

        Def,
        Master,
        Struct,
        Array,
        Extern,

        If,
        Else,
        For,
        While,

        Binary,
        Unary,

        LeftParen,
        RightParen,
        LeftSquare,
        RightSquare,

        LeftCurly,
        RightCurly,

        LessThan,
        LessEqual,
        GreaterThan,
        GreaterEqual,

        Modulo,
        Equal,
        DoubleEqual,
        Ass,
        Ampersand,
        DoubleAmpersand,
        Plus,
        Minus,
        Asterisk,
        Not,
        NotEqual,
        Slash,

        Semicolon,
        Hash,
        Dot,
        Comma,
        Colon,
        SingleQuote,
        DoubleQuote,
        Char,
        Comment,
        Return,
        Pipe,
        DoublePipe,
        End,
        Unexpected
    };

    Token(Kind kind = Kind::End)  : m_kind{kind} {}

    Token(Kind kind, const char * beg, std::size_t len) 
            : m_kind{kind}, m_lexeme(beg, len) {}

    Token(Kind kind, const char * beg, const char * end) 
            : m_kind{kind}, m_lexeme(beg, std::distance(beg, end)) {}
    
    Token(Kind kind, std::string lexeme_)
            : m_kind{kind}, m_lexeme(std::move(lexeme_)) {}

    Kind kind() const  { return m_kind; }

    void kind(Kind kind) { m_kind = kind; }

    bool is_kind(Kind kind) const  { return m_kind == kind; }

    bool is_one_of(Kind k1, Kind k2) const  { return is_kind(k1) || is_kind(k2); }

    template <typename... Ts>
    bool is_one_of(Kind k1, Kind k2, Ts... ks) const {
        return is_kind(k1) || is_one_of(k2, ks...);
    }

    const std::string & lexeme() const { return m_lexeme; }

    const auto get_line() const { return _line; }
    const auto get_pos() const { return _pos; }

    void set_line(std::size_t line_) { _line = line_; }
    void set_pos(std::size_t pos_) { _pos = pos_; }

    void lexeme(std::string lexeme) {
        m_lexeme = std::move(lexeme);
    }

private:
    Kind            m_kind{};
    std::string     m_lexeme{};

    std::size_t _line = 0;
    std::size_t _pos = 0;
};

static std::ostream& operator<<(std::ostream& os, const Token::Kind& kind) {
    static const char * const names[]{
            "Number", "Identifier",
            "Var", "Stick", "Your", "In", "My", "Out", "Cum",
            "Def", "Master", "Struct", "Array", "Extern", 
            "If", "Else", "For", "While",
            "Binary", "Unary", 
            "LeftParen", "RightParen", "LeftSquare", "RightSquare", "LeftCurly", "RightCurly",
            "LessThan", "LessEqual", "GreaterThan", "GreaterEqual", "Modulo", "Equal", "DoubleEqual", "Ass", "Ampersand", "DoubleAmpersand", "Plus", "Minus", "Asterisk", "Not", "NotEqual", "Slash", 
            "Semicolon", "Hash", "Dot", "Comma", "Colon",      
            "SingleQuote", "DoubleQuote", "Char", "Comment", "Return", "Pipe", "DoublePipe", "End",
            "Unexpected",
    };
    return os << names[static_cast<int>(kind)];
}
