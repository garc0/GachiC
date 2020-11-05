#include "Parser.h"

std::unique_ptr<ASTNode> Parser::parseNumber() {
    auto Result = make_node<NumberExprNode>(std::string(this->_cToken.lexeme()));
    this->eat();
    return std::move(Result);
}

std::unique_ptr<ASTNode> Parser::parseParen() {

  this->expectNext(Token::Kind::LeftParen);

  auto V = parseExpression();
  if (!V)
    return nullptr;

  if(!this->expectNext(Token::Kind::RightParen).has_value())
    return nullptr;

  return std::move(V);
}

std::unique_ptr<ASTNode> Parser::parseBlock() {
    this->expectNext(Token::Kind::LeftCurly);

    auto oldBlock = std::move(_cBlock);
    _cBlock = make_node<BlockNode>(); 

    while(this->_cToken.kind() != Token::Kind::RightCurly){
        auto V = parseExpression();

        if (!V){
            return nullptr;
        }
        if(!_cBlock) {
            std::cout << "_cBlock == nullptr" << std::endl;
            return nullptr;
        }

        std::visit(overload {
            [&](BlockNode &arg) { arg.l.push_back(std::move(V)); },
            [](auto &arg) { std::cerr << "WTF\n"; },
        }, *_cBlock.get());

        if(!this->expectNext(Token::Kind::Semicolon).has_value())
            return nullptr;
    }


    if(!this->expectNext(Token::Kind::RightCurly).has_value())
        return nullptr;

    auto retBlock = std::move(_cBlock);
    _cBlock = std::move(oldBlock);

    return std::move(retBlock);
}

std::unique_ptr<ASTNode> Parser::parseIdentifier() {
    std::string IdName(this->_cToken.lexeme());
    this->eat(); 

    if (this->_cToken.kind() == Token::Kind::LeftParen){
        // Call
        this->eat(); 
        std::vector<std::unique_ptr<ASTNode>> Args;
        if (this->_cToken.kind() != Token::Kind::RightParen) {
            while (true) {
                if (auto Arg = parseExpression())
                    Args.push_back(std::move(Arg));
                else
                    return nullptr;

                if (this->_cToken.kind() == Token::Kind::RightParen)
                    break;

                if(!this->expectNext(Token::Kind::Comma).has_value())
                    return nullptr;
            }
        }

        if(!this->expectNext(Token::Kind::RightParen, "in call").has_value())
            return nullptr;

        return make_node<CallExprNode>(IdName, std::move(Args));
    }

    if(this->_cToken.kind() == Token::Kind::LeftCurly){
        // struct init
        this->expectNext(Token::Kind::LeftCurly);

        std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> fields;

        while(this->_cToken.kind() != Token::Kind::RightCurly){
            if(!this->_cToken.is_kind(Token::Kind::Identifier)){
                std::cerr << "Field name, ah?" << std::endl;
                return nullptr;
            }

            std::string field_name(this->_cToken.lexeme());

            this->eat();

            this->expectNext(Token::Kind::Colon);

            std::unique_ptr<ASTNode> val = this->parseExpression();

            if(!val) {
                std::cerr << "Why can't you be just a normal!?" << std::endl;
                return nullptr;
            }

            fields.push_back(std::make_pair(std::move(field_name), std::move(val)));

            if(this->_cToken.kind() != Token::Kind::RightCurly)
                this->expectNext(Token::Kind::Comma);
        }

        this->expectNext(Token::Kind::RightCurly);

        return make_node<StructExprNode>(IdName, std::move(fields));

    }

    if(this->_cToken.kind() == Token::Kind::LeftSquare){
        //Array index
        
        std::vector<std::unique_ptr<ASTNode>> indexes;

        while(this->_cToken.is_kind(Token::Kind::LeftSquare)){

            this->eat();

            indexes.push_back(std::move(this->parseExpression()));
            
            this->expectNext(Token::Kind::RightSquare);
        }   

        return make_node<ArrayExprNode>(IdName, std::move(indexes));
    }
    
    // just a variable; 
    return make_node<VariableExprNode>(IdName);
}

std::unique_ptr<ASTNode> Parser::parseIf() {

    if(!this->expectNext(Token::Kind::If).has_value())
        return nullptr;

    auto Cond = parseExpression();
    if (!Cond)
        return nullptr;
    

    auto Then = parseExpression();
    if (!Then)
     return nullptr;



    if(!this->_cToken.is_kind(Token::Kind::Else))
        return make_node<IfExpr>(std::move(Cond), std::move(Then), nullptr);    
    
    this->eat();

    auto Else = parseExpression();
    if (!Else)
        return nullptr;

    return make_node<IfExpr>(std::move(Cond), std::move(Then),
                                        std::move(Else));
}

std::unique_ptr<ASTNode> Parser::parseFor() {
    this->expectNext(Token::Kind::For);

    if (this->_cToken.is_kind(Token::Kind::Identifier))
        return log_err("expected identifier after for");

    std::string IdName(this->_cToken.lexeme());
    this->eat(); // eat identifier.

    if(!this->expectNext(Token::Kind::Equal).has_value())
        return nullptr;

    auto Start = parseExpression();
    if (!Start)
        return nullptr;
    
    if(!this->expectNext(Token::Kind::Comma).has_value())
        return nullptr;

    auto End = parseExpression();
    if (!End)
        return nullptr;

    // The step value is optional.
    std::unique_ptr<ASTNode> Step;
    if (this->_cToken.is_kind(Token::Kind::Comma)) {
        this->eat();
        Step = parseExpression();
    }

    if (!Step)
        return nullptr;

    auto Body = parseExpression();
    if (!Body)
        return nullptr;

    return make_node<ForExpr>(IdName, std::move(Start), std::move(End),
                                        std::move(Step), std::move(Body));
}

std::unique_ptr<ASTNode> Parser::parseWhile() {
    this->expectNext(Token::Kind::While);

    auto cond = parseExpression();

    if(!cond)
        return nullptr;

    std::optional<decltype(cond)> step = nullptr;

    if(this->_cToken.is_kind(Token::Kind::Pipe)){
        this->eat();
        
        step = parseExpression();

        this->expectNext(Token::Kind::Pipe);
    } else step = nullptr;


    auto body = parseExpression();

    if(!cond)
        return nullptr;

    return make_node<WhileExpr>(std::move(cond), std::move(step), std::move(body));
}

std::unique_ptr<ASTNode> Parser::parseType(){

    std::vector<TypeNode::type_pair> t_v;

    while(this->_cToken.is_one_of(Token::Kind::Asterisk, Token::Kind::LeftSquare) ){
        if(this->_cToken.is_kind(Token::Kind::Asterisk)){
            this->eat();
            t_v.push_back({TypeNode::type_id::pointer, "*"});
        }else{
            this->expectNext(Token::Kind::LeftSquare);
            if(!this->_cToken.is_kind(Token::Kind::Number)){
                std::cerr << "Expected a number" << std::endl;
                return nullptr;
            }
            t_v.push_back({TypeNode::type_id::array, this->_cToken.lexeme().c_str()});
            this->eat();
            this->expectNext(Token::Kind::RightSquare);
        }
    }

    auto s = this->_cToken.lexeme();

    this->eat();

    if(s == "nothing")  t_v.push_back({TypeNode::type_id::nothing, s.c_str()});

    if(s == "bool")     t_v.push_back({TypeNode::type_id::u1, s.c_str()});

    if(s == "u8")       t_v.push_back({TypeNode::type_id::u8, s.c_str()});
    if(s == "u16")      t_v.push_back({TypeNode::type_id::u16, s.c_str()});
    if(s == "u32")      t_v.push_back({TypeNode::type_id::u32, s.c_str()});
    if(s == "u64")      t_v.push_back({TypeNode::type_id::u64, s.c_str()});

    if(s == "i8")       t_v.push_back({TypeNode::type_id::i8, s.c_str()});
    if(s == "i16")      t_v.push_back({TypeNode::type_id::i16, s.c_str()});
    if(s == "i32")      t_v.push_back({TypeNode::type_id::i32, s.c_str()});
    if(s == "i64")      t_v.push_back({TypeNode::type_id::i64, s.c_str()});

    if(s == "f32")      t_v.push_back({TypeNode::type_id::f32, s.c_str()});
    if(s == "f64")      t_v.push_back({TypeNode::type_id::f64, s.c_str()});

    // it's fine
    t_v.push_back({TypeNode::type_id::struct_type, s.c_str()});

    return make_node<TypeNode>(std::move(t_v));
}

std::unique_ptr<ASTNode> Parser::parseVar() {
    this->expectNext(Token::Kind::Var);

    std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> VarNames;

    if (this->_cToken.kind() != Token::Kind::Identifier)
      return log_err("expected identifier after var");

    while (true) {
        std::string Name(this->_cToken.lexeme());
        this->eat(); // eat identifier.

        // Read the optional initializer.
        std::unique_ptr<ASTNode> Init = nullptr;
        if (this->_cToken.kind() == Token::Kind::Equal) {
            this->eat(); // eat the '='.

            Init = parseExpression();
            if (!Init)
                return nullptr;
        }

        VarNames.push_back(std::make_pair(Name, std::move(Init)));

        // ',' found, exit loop.
        if (this->_cToken.kind() != Token::Kind::Comma)
          break;
        
        this->eat();

        if (this->_cToken.kind() != Token::Kind::Identifier)
            return log_err("expected identifier list after var");
    }

    return make_node<VarExprNode>(_cBlock.get(), std::move(VarNames));
}

std::unique_ptr<ASTNode> Parser::parseStick() {
    this->expectNext(Token::Kind::Stick);

    std::pair<std::string, std::unique_ptr<ASTNode>> _to_stick;

    if(this->_cToken.is_kind(Token::Kind::Out))
    {
        this->eat();
        if(!this->_cToken.is_kind(Token::Kind::Identifier))
            return nullptr;

        _to_stick.first = this->_cToken.lexeme();
        _to_stick.second = nullptr;

        this->eat();

        return make_node<StickNode>(_cBlock.get(), std::move(_to_stick), false);
    }

    this->expectNext(Token::Kind::Your);

    std::unique_ptr<ASTNode> sz = nullptr;

    if(this->_cToken.is_kind(Token::Kind::LeftSquare)){
        this->expectNext(Token::Kind::LeftSquare);

        sz = parseExpression();

        this->expectNext(Token::Kind::RightSquare);
    }

    _to_stick.second = parseType();

    if(!_to_stick.second){
        std::cout << "Failed to parse type" << std::endl;
        return nullptr;
    }
    
    this->expectNext(Token::Kind::In);
    this->expectNext(Token::Kind::My);

    if(!this->_cToken.is_kind(Token::Kind::Identifier))
        return nullptr;

    _to_stick.first = this->_cToken.lexeme();

    this->eat();

   return make_node<StickNode>(_cBlock.get(), std::move(_to_stick), true, std::move(sz));
}

std::unique_ptr<ASTNode> Parser::parseStruct() {
    this->eat();

    if(this->_cToken.kind() != Token::Kind::Identifier)
        return log_err("expected Name after 'struct'");

    std::string name_struct(this->_cToken.lexeme());
    this->eat();

    if(this->_cToken.kind() != Token::Kind::LeftCurly)
        return log_err("expected LeftCurly after 'struct'");

    this->eat();

    std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> body;

    while(this->_cToken.kind() != Token::Kind::RightCurly){
        if(this->_cToken.kind() != Token::Kind::Identifier)
            return log_err("expected Identifier in struct");

        std::string field_name(this->_cToken.lexeme());
        this->eat();

        if(!this->expectNext(Token::Kind::Colon).has_value())
            return nullptr;

        
        body.push_back({field_name, std::move(parseType())});

        if(!this->expectNext(Token::Kind::Semicolon).has_value())
            return nullptr;
    }

    this->eat();

    return make_node<StructNode>(std::move(name_struct), std::move(body));
}

std::unique_ptr<ASTNode> Parser::parseArray(){

    std::vector<std::unique_ptr<ASTNode>> body;

    this->expectNext(Token::Kind::LeftSquare);

    while(this->_cToken.kind() != Token::Kind::RightSquare){
        auto E = this->parseExpression();

        body.push_back(std::move(E));

        if(this->_cToken.kind() != Token::Kind::RightSquare)
            this->expectNext(Token::Kind::Comma);
    }

    this->eat();

    return make_node<ArrayInitNode>(std::move(body));
}

std::unique_ptr<ASTNode> Parser::parseChar(){
    
    this->expectNext(Token::Kind::SingleQuote);

    uint8_t c = this->_cToken.lexeme()[0];

    this->eat();

    if(!this->expectNext(Token::Kind::SingleQuote).has_value())
    return nullptr;

    return make_node<CharNode>(c);
}

std::unique_ptr<ASTNode> Parser::parseString(){
    
    this->expectNext(Token::Kind::DoubleQuote);

    std::string body;

    while(this->_cToken.kind() != Token::Kind::DoubleQuote){
        uint8_t c = this->_cToken.lexeme()[0];

        this->eat();
        body.push_back(c);
    }

    this->expectNext(Token::Kind::DoubleQuote);

    return make_node<StringNode>(std::move(body));
}

std::unique_ptr<ASTNode> Parser::parsePrimary() {
    switch (this->_cToken.kind()) {
        default:
        std::cout  << this->_cToken.kind() << "\n" ;
        return log_err("unknown token when expecting an expression");

        case Token::Kind::Identifier:   return parseIdentifier();
        case Token::Kind::SingleQuote:  return parseChar();
        case Token::Kind::DoubleQuote:  return parseString();
        case Token::Kind::LeftSquare:   return parseArray();
        case Token::Kind::LeftCurly:    return parseBlock();
        case Token::Kind::LeftParen:    return parseParen();
        case Token::Kind::Stick:        return parseStick();
        case Token::Kind::Number:       return parseNumber();
        case Token::Kind::While:        return parseWhile();
        case Token::Kind::For:          return parseFor();
        case Token::Kind::Var:          return parseVar();
        case Token::Kind::If:           return parseIf();
    }

    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseUnary() {
    if (!(
        (this->_cToken.kind() >= Token::Kind::LessThan) && 
        (this->_cToken.kind() <= Token::Kind::Slash) || 
        this->_cToken.kind() == Token::Kind::Return
        ))
        return parsePrimary();

    // If this is a unary operator, read it.
    auto Opc = this->_cToken;
    this->eat();

    if (auto Operand = parseUnary())
        return make_node<UnaryExprNode>(Opc, std::move(Operand));
    return nullptr;
}

int32_t Parser::GetTokPrecedence(Token tok) {
    if (!(tok.kind() >= Token::Kind::LessThan && tok.kind() <= Token::Kind::Slash ||
     tok.kind() == Token::Kind::Colon ||
     tok.kind() == Token::Kind::Dot ||
     tok.kind() == Token::Kind::Equal))
        return -1;

    int32_t TokPrec = 0;

    while(_bOp[TokPrec] != tok.lexeme())
        TokPrec++;

    if (TokPrec >= _bOp.size())
        return -1;
    return TokPrec;
}

 std::unique_ptr<ASTNode> Parser::parseBinOpRHS(int32_t ExprPrec, std::unique_ptr<ASTNode> LHS) {
  
    while (true) {
        int32_t TokPrec = GetTokPrecedence(this->_cToken);

        if (TokPrec < ExprPrec)
            return LHS;

        auto BinOp = this->_cToken;
        this->eat(); 

        auto RHS = BinOp.is_kind(Token::Kind::Ass) ? parseType() : parseUnary();
        if (!RHS) return nullptr;

        int NextPrec = GetTokPrecedence(this->_cToken);
        if (TokPrec < NextPrec) {
            RHS = parseBinOpRHS(TokPrec, std::move(RHS));
            if (!RHS) return nullptr;
        }

        LHS = make_node<BinaryExprNode>(BinOp, std::move(LHS), std::move(RHS));
    }

    // ok
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
    auto LHS = parseUnary();
    if (!LHS) return nullptr;

    return parseBinOpRHS(0, std::move(LHS));
}

std::unique_ptr<DefNode> Parser::parsePrototype() {

    std::string function_name;

    if(this->_cToken.is_kind(Token::Kind::Identifier))
        function_name = this->_cToken.lexeme();
    else if(this->_cToken.is_kind(Token::Kind::Master))
        function_name = "main";
    
    else return nullptr;

    this->eat();

    if(!this->expectNext(Token::Kind::LeftParen, "in prototype").has_value())
       return nullptr;


    std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>> ArgNames;
    while (this->_cToken.is_kind(Token::Kind::Identifier))
    {

        auto _a = (this->_cToken.lexeme());
        this->eat();

        if(this->expectNext(Token::Kind::Colon).has_value()){

            ArgNames.push_back(
                std::make_pair<std::string, std::unique_ptr<ASTNode>>(
                    std::string(_a),
                    std::move(parseType())));
        }else {
            std::cerr << "Argument has no type" << std::endl; 
            return nullptr;
        }

        if(!this->_cToken.is_kind(Token::Kind::RightParen))
            this->expectNext(Token::Kind::Comma);

    }

    if(!this->expectNext(Token::Kind::RightParen, "in prototype").has_value())
        return nullptr;

    std::unique_ptr<ASTNode> Ret_Type = nullptr;


    if (this->expectNext(Token::Kind::Cum).has_value())
            Ret_Type = parseType();


    return make_def<PrototypeNode>(function_name, std::move(ArgNames), std::move(Ret_Type));
}

std::unique_ptr<DefNode> Parser::parseExtern() {
    this->expectNext(Token::Kind::Extern);
    return this->parsePrototype();
}

std::unique_ptr<DefNode> Parser::parseDef() {
    this->expectNext(Token::Kind::Def);

    auto Proto = parsePrototype();
    if (!Proto)
     return nullptr;

    std::unique_ptr<ASTNode> E = parseExpression();
    if (!E)  std::cerr << "Fucking bad" << std::endl;

    return make_def<FunctionNode>(std::move(Proto), std::move(E));
}

// Copypasta
std::unique_ptr<DefNode> Parser::parseMain() {
    auto Proto = parsePrototype();
    if (!Proto)
     return nullptr;

    std::unique_ptr<ASTNode> E = parseExpression();
    if (!E)  std::cerr << "Fucking bad" << std::endl;

    return make_def<FunctionNode>(std::move(Proto), std::move(E));
}

std::unique_ptr<DefNode>  Parser::parseTopLevelExpr(){
        auto E = parseExpression();
        if(E){
            auto Proto = make_def<PrototypeNode>("__anon_expr",
                                                 std::vector<std::pair<std::string, std::unique_ptr<ASTNode>>>(), nullptr);
            return  make_def<FunctionNode>(std::move(Proto), std::move(E));
        }
        return nullptr;
    }