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
        In,

        Def,
        Struct,
        Extern,

        If,
        Else,
        For,

        Binary,
        Unary,

        LeftParen,
        RightParen,
        LeftSquare,
        RightSquare,

        LeftCurly,
        RightCurly,

        LessThan,
        GreaterThan,

        Modulo,
        Equal,
        DoubleEqual,
        Plus,
        Minus,
        Asterisk,
        Slash,

        Semicolon,
        Hash,
        Dot,
        Comma,
        Colon,
        SingleQuote,
        DoubleQuote,
        Comment,
        Return,
        Pipe,
        End,
        Unexpected
    };

    Token(Kind kind = Kind::End) noexcept : m_kind{kind} {}

    Token(Kind kind, const char * beg, std::size_t len) noexcept
            : m_kind{kind}, m_lexeme(beg, len) {}

    Token(Kind kind, const char * beg, const char * end) noexcept
            : m_kind{kind}, m_lexeme(beg, std::distance(beg, end)) {}

    Kind kind() const noexcept { return m_kind; }

    void kind(Kind kind) noexcept { m_kind = kind; }

    bool is_kind(Kind kind) const noexcept { return m_kind == kind; }

    bool is_one_of(Kind k1, Kind k2) const noexcept { return is_kind(k1) || is_kind(k2); }

    template <typename... Ts>
    bool is_one_of(Kind k1, Kind k2, Ts... ks) const noexcept {
        return is_kind(k1) || is_one_of(k2, ks...);
    }

    std::string_view lexeme() const noexcept { return m_lexeme; }

    void lexeme(std::string_view lexeme) noexcept {
        m_lexeme = std::move(lexeme);
    }

private:
    Kind             m_kind{};
    std::string_view m_lexeme{};
};

static std::ostream& operator<<(std::ostream& os, const Token::Kind& kind) {
    static const char * const names[]{
            "Number", "Identifier",
            "Var", "In",
            "Def", "Struct", "Extern", 
            "If", "Else", "For",
            "Binary", "Unary", 
            "LeftParen", "RightParen", "LeftSquare", "RightSquare", "LeftCurly", "RightCurly",
            "LessThan", "GreaterThan", "Modulo", "Equal", "DoubleEqual", "Plus", "Minus", "Asterisk", "Slash", 
            "Semicolon",   "Hash", "Dot", "Comma", "Colon",      
            "SingleQuote", "DoubleQuote", "Comment", "Return", "Pipe", "End",
            "Unexpected",
    };
    return os << names[static_cast<int>(kind)];
}
