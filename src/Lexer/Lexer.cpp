#include "Lexer.h"

inline bool is_digit(char c){
    return c >= '0' && c  <= '9';
}

inline bool is_space(char c){
    return (
        c == ' '  ||
        c == '\t' ||
        c == '\r' ||
        c == '\0' ||
        c == '\n'
    ); 
}

inline bool is_word(char c) {
    return (
        (c >= 'a' && c  <= 'z') ||
        (c >= 'A' && c  <= 'Z')
    );
}

inline bool is_identifier_char(char c) noexcept {
    return  is_word(c) ||
            isdigit(c) ||
            c == '_';
}

inline Token Lexer::getAtom() noexcept{

    auto atom = [&](Token::Kind kind) -> Token{
        return Token(kind, &_s[_i++], 1);
    };

    switch (this->_s[_i]) {
        case '\0':
            return Token(Token::Kind::End, &this->_s[_i], 1);
        case '(':
            return atom(Token::Kind::LeftParen);
        case ')':
            return atom(Token::Kind::RightParen);
        case '[':
            return atom(Token::Kind::LeftSquare);
        case ']':
            return atom(Token::Kind::RightSquare);
        case '{':
            return atom(Token::Kind::LeftCurly);
        case '}':
            return atom(Token::Kind::RightCurly);
        case '<':
            if(this->_s[_i + 1] == '='){
                this->_i += 2;
                return Token(Token::Kind::LessEqual, &this->_s[_i - 2], 2);
            }
            return atom(Token::Kind::LessThan);
        case '>':
            if(this->_s[_i + 1] == '='){
                this->_i += 2;
                return Token(Token::Kind::GreaterEqual, &this->_s[_i - 2], 2);
            }
            return atom(Token::Kind::GreaterThan);
        case '%':
            return atom(Token::Kind::Modulo);
        case '=':
            if(this->_s[_i + 1] == '='){
                this->_i += 2;
                return Token(Token::Kind::DoubleEqual, &this->_s[_i - 2], 2);
            }
            return atom(Token::Kind::Equal);
        case '&':
            return atom(Token::Kind::Ampersand);
        case '+':
            return atom(Token::Kind::Plus);
        case '-':
            return atom(Token::Kind::Minus);
        case '*':
            return atom(Token::Kind::Asterisk);
        case '/':
            return slash_or_comment();
        case '#':
            return atom(Token::Kind::Hash);
        case '.':
            return atom(Token::Kind::Dot);
        case ',':
            return atom(Token::Kind::Comma);
        case ':':
            return atom(Token::Kind::Colon);
        case ';':
            return atom(Token::Kind::Semicolon);
        case '!':
            if(this->_s[_i + 1] == '='){
                this->_i += 2;
                return Token(Token::Kind::NotEqual, &this->_s[_i - 2], 2);
            }
            return atom(Token::Kind::Not);
        case '\'':
            return parse_char();
        case '"':
            return parse_string();
        case '|':
            return atom(Token::Kind::Pipe);

        default:
            return atom(Token::Kind::Unexpected);
    }
}

Token Lexer::parse() noexcept{
    // "spaces" for human
    while(is_space(this->_s[_i]))
        this->_i++;

    // it is word?
    if(is_word(this->_s[_i])) return identifier();
    
    // it is number!?
    if(is_digit(this->_s[_i])) return number();

    // smth else
    return getAtom();
}

std::optional<Token::Kind> stotok(std::string identifier){
    static const std::map<std::string, Token::Kind> keys = {
        {"slave", Token::Kind::Def},
        {"var", Token::Kind::Var},
        {"cumming", Token::Kind::Return},
        {"stick", Token::Kind::Stick},
        {"your", Token::Kind::Your},
        {"in", Token::Kind::In},
        {"my", Token::Kind::My},
        {"cum", Token::Kind::Cum},
        {"out", Token::Kind::Out},
        {"master", Token::Kind::Master},
        {"ass", Token::Kind::Ass},
        {"extern", Token::Kind::Extern},
        {"array", Token::Kind::Array},
        {"if", Token::Kind::If},
        {"else", Token::Kind::Else},
        {"for", Token::Kind::For},
        {"while", Token::Kind::While},
        {"struct", Token::Kind::Struct},
    };

    auto f = keys.find(identifier);

    if(f != keys.end())
        return std::optional<Token::Kind>(f->second);
    
    return std::nullopt;
} 

Token Lexer::identifier() noexcept {
    auto i = this->_i;
    
    _i++;

    while (is_identifier_char(this->_s[_i])) _i++;

    std::string s(&_s[i], _i - i);

    Token::Kind k = stotok(s).value_or(Token::Kind::Identifier);
    
    return Token(k, &_s[i], _i - i);
}

Token Lexer::number() noexcept {
    std::string n{};

    if( this->_s[_i] == 'x' ||
        this->_s[_i] == 'b') 
        this->_i++;
    
    while ( isdigit(this->_s[_i])   ||
            this->_s[_i] == '.'     || 
            this->_s[_i] == '_'){ 
        if(this->_s[_i] != '_')
            n.push_back(this->_s[_i]);
        this->_i++;
    }
    return Token(Token::Kind::Number, std::move(n));
}

Token Lexer::slash_or_comment() noexcept {
    auto i = this->_i;

    this->_i++;
    
    if (this->_s[_i] == '/') {
        i = ++_i;
        while (this->_s[_i] != '\0') 
            if (this->_s[++_i] == '\n') 
                return Token(Token::Kind::Comment, &_s[i], _i - i - 1);
        
        return Token(Token::Kind::Unexpected, &_s[i], 1);
    }
    
    return Token(Token::Kind::Slash, &_s[i], 1);
}

Token Lexer::parse_char() noexcept{
    
    auto atom = [&](Token::Kind kind) -> Token{
        return Token(kind, &_s[_i++], 1);
    };

    this->_tokens.push_back(atom(Token::Kind::SingleQuote));


    uint8_t c = this->_s[_i];

    if(c == '\\'){
        _i++;
        c = this->_s[_i];
        switch(c){
            case 'n':
                c = '\n';
                break;
            case 't':
                c = '\t';
                break;  
            case '\\':
                c = '\\';
                break;  
            case '0':
                c = '\0';
                break;    
        }


        this->_tokens.push_back(Token(Token::Kind::Char, (const char*)&c, 1));
        this->_i++;
    }

    return atom(Token::Kind::SingleQuote);
}

Token Lexer::parse_string() noexcept{
    
    auto atom = [&](Token::Kind kind) -> Token{
        return Token(kind, &_s[_i++], 1);
    };

    this->_tokens.push_back(atom(Token::Kind::DoubleQuote));

    while(this->_s[_i] != '"'){

        uint8_t c = this->_s[_i];

        if(c == '\\'){
            _i++;
            c = this->_s[_i];
            switch(c){
                case 'n':
                    c = '\n';
                    break;
                case 't':
                    c = '\t';
                    break;  
                case '\\':
                    c = '\\';
                    break;  
                case '0':
                    c = '\0';
                    break;    
            }

        }

        this->_tokens.push_back(Token(Token::Kind::Char, (const char*)&c, 1));
        this->_i++;
    }

    return atom(Token::Kind::DoubleQuote);
}