#include "Parser.h"

std::unique_ptr<BaseNode> Parser::parseNumber() {
  auto Result = std::make_unique<NumberExprNode>(std::string(this->_cToken.lexeme()));
  this->eat();
  return std::move(Result);
}

std::unique_ptr<BaseNode> Parser::parseParen() {

  this->expectNext(Token::Kind::LeftParen);

  auto V = parseExpression();
  if (!V)
    return nullptr;

  if(!this->expectNext(Token::Kind::RightParen).has_value())
    return nullptr;

  return V;
}

std::unique_ptr<BaseNode> Parser::parseBlock() {
   this->expectNext(Token::Kind::LeftCurly);

  auto oldBlock = std::move(_cBlock);
  _cBlock = std::make_unique<BlockNode>(); 

  while(this->_cToken.kind() != Token::Kind::RightCurly){
    auto V = parseExpression();
    
    if (!V){
      return nullptr;
    }
    if(!_cBlock) {
      std::cout << "_cBlock == nullptr" << std::endl;
      return nullptr;
    }
    _cBlock->l.push_back(std::move(V));

    if(!this->expectNext(Token::Kind::Semicolon).has_value())
        return nullptr;
    
  }
  

  if(!this->expectNext(Token::Kind::RightCurly).has_value())
    return nullptr;

  auto retBlock = std::move(_cBlock);
  _cBlock = std::move(oldBlock);
  
  return std::move(retBlock);
}

std::unique_ptr<BaseNode> Parser::parseIdentifier() {
    std::string IdName(this->_cToken.lexeme());
    this->eat(); 

    if (this->_cToken.kind() == Token::Kind::LeftParen){
        // Call
        this->eat(); 
        std::vector<std::unique_ptr<BaseNode>> Args;
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

        return std::make_unique<CallExprNode>(IdName, std::move(Args));
    }

    if(this->_cToken.kind() == Token::Kind::LeftCurly){
        // struct init
        this->expectNext(Token::Kind::LeftCurly);

        std::vector<std::pair<std::string, std::unique_ptr<BaseNode>>> fields;

        while(this->_cToken.kind() != Token::Kind::RightCurly){
            if(!this->_cToken.is_kind(Token::Kind::Identifier)){
                std::cout << "Field name, ah?" << std::endl;
                return nullptr;
            }

            std::string field_name(this->_cToken.lexeme());

            this->eat();

            this->expectNext(Token::Kind::Colon);

            std::unique_ptr<BaseNode> val = this->parseExpression();

            if(!val) {
                std::cout << "Why can't you be just a normal!?" << std::endl;
                return nullptr;
            }

            fields.push_back(std::make_pair(std::move(field_name), std::move(val)));

            if(this->_cToken.kind() != Token::Kind::RightCurly)
                this->expectNext(Token::Kind::Comma);
        }

        this->expectNext(Token::Kind::RightCurly);

        return std::make_unique<StructExprNode>(std::move(IdName), std::move(fields));

    }

    if(this->_cToken.kind() == Token::Kind::LeftSquare){
        //Array index
        
        this->expectNext(Token::Kind::LeftSquare);

        std::unique_ptr<BaseNode> E = this->parseExpression();
        
        this->expectNext(Token::Kind::RightSquare);

        return std::make_unique<ArrayExprNode>(std::move(IdName), std::move(E));
    }
    
    // just a variable; 
    return std::make_unique<VariableExprNode>(IdName);
}

std::unique_ptr<BaseNode> Parser::parseIf() {

    if(!this->expectNext(Token::Kind::If).has_value())
        return nullptr;

    auto Cond = parseExpression();
    if (!Cond)
        return nullptr;

    auto Then = parsePrimary();
    if (!Then)
     return nullptr;

    if(!this->expectNext(Token::Kind::Else).has_value())
        return nullptr;

    auto Else = parsePrimary();
    if (!Else)
        return nullptr;

    return std::make_unique<IfNode>(std::move(Cond), std::move(Then),
                                        std::move(Else));
}

std::unique_ptr<BaseNode> Parser::parseFor() {
    this->eat(); // eat the for.

    if (this->_cToken.kind() != Token::Kind::Identifier)
        return LogError("expected identifier after for");

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
    std::unique_ptr<BaseNode> Step;
    if (this->_cToken.kind() != Token::Kind::Comma) {
        this->eat();
        Step = parseExpression();
        if (!Step)
            return nullptr;
    }

    auto Body = parseExpression();
    if (!Body)
        return nullptr;

    return std::make_unique<ForExprNode>(IdName, std::move(Start), std::move(End),
                                        std::move(Step), std::move(Body));
}

llvm::Type * Parser::parseType(){
    if(this->_cToken.kind() == Token::Kind::Asterisk){
        this->eat();
        return llvm::PointerType::get(parseType(), 0);
    }

    if(this->_cToken.kind() != Token::Kind::Identifier){
        std::cout << "expected type name" << std::endl;
        return nullptr;
    }

    std::string s = this->_cToken.lexeme();

    this->eat();


    if(s == "nothing") return llvm::Type::getVoidTy(TheContext);

    if(s == "bool") return llvm::Type::getInt1Ty(TheContext);

    if(s == "u8")  return llvm::Type::getInt8Ty(TheContext);
    if(s == "u16") return llvm::Type::getInt16Ty(TheContext);
    if(s == "u32") return llvm::Type::getInt32Ty(TheContext);
    if(s == "u64") return llvm::Type::getInt64Ty(TheContext);

    if(s == "i8")  return llvm::Type::getInt8Ty(TheContext);
    if(s == "i16") return llvm::Type::getInt16Ty(TheContext);
    if(s == "i32") return llvm::Type::getInt32Ty(TheContext);
    if(s == "i64") return llvm::Type::getInt64Ty(TheContext);

    if(s == "f32") return llvm::Type::getFloatTy(TheContext);
    if(s == "f64") return llvm::Type::getDoubleTy(TheContext);

    auto f = NamedStructures.find(s);

    if(f != NamedStructures.end())
        return NamedStructures[s];

    return nullptr;
}

std::unique_ptr<BaseNode> Parser::parseVar() {
    this->expectNext(Token::Kind::Var);

    std::vector<std::pair<std::string, std::unique_ptr<BaseNode>>> VarNames;

    if (this->_cToken.kind() != Token::Kind::Identifier)
      return LogError("expected identifier after var");

    while (true) {
        std::string Name(this->_cToken.lexeme());
        this->eat(); // eat identifier.

        // Read the optional initializer.
        std::unique_ptr<BaseNode> Init = nullptr;
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
            return LogError("expected identifier list after var");
    }

    return std::make_unique<VarExprNode>(_cBlock.get(), std::move(VarNames));
}

std::unique_ptr<BaseNode> Parser::parseStruct() {
    this->eat();

    if(this->_cToken.kind() != Token::Kind::Identifier)
        return LogError("expected Name after 'struct'");

    std::string name_struct(this->_cToken.lexeme());
    this->eat();

    if(this->_cToken.kind() != Token::Kind::LeftCurly)
        return LogError("expected LeftCurly after 'struct'");

    this->eat();

    std::vector<std::pair<std::string, llvm::Type *>> body;

    while(this->_cToken.kind() != Token::Kind::RightCurly){
        if(this->_cToken.kind() != Token::Kind::Identifier)
            return LogError("expected Identifier in struct");

        std::string field_name(this->_cToken.lexeme());
        this->eat();

        if(!this->expectNext(Token::Kind::Colon).has_value())
            return nullptr;

        
        body.push_back({field_name, parseType()});

        if(!this->expectNext(Token::Kind::Semicolon).has_value())
            return nullptr;
    }

    this->eat();

    return std::make_unique<StructNode>(std::move(name_struct), std::move(body));
}

std::unique_ptr<BaseNode> Parser::parseArray(){
    this->expectNext(Token::Kind::Array);

    std::vector<std::unique_ptr<BaseNode>> body;

    this->expectNext(Token::Kind::LeftCurly);

    while(this->_cToken.kind() != Token::Kind::RightCurly){
        auto E = this->parseExpression();

        body.push_back(std::move(E));

        if(this->_cToken.kind() != Token::Kind::RightCurly)
            this->expectNext(Token::Kind::Comma);
    }

    this->eat();

    return std::make_unique<ArrayInitNode>(std::move(body));
}

std::unique_ptr<BaseNode> Parser::parseChar(){
    
    this->expectNext(Token::Kind::SingleQuote);

    uint8_t c = this->_cToken.lexeme()[0];

    this->eat();

    if(!this->expectNext(Token::Kind::SingleQuote).has_value())
    return nullptr;

    return std::make_unique<NumberExprNode>(std::to_string(int(c)));
}

std::unique_ptr<BaseNode> Parser::parseString(){
    
    this->expectNext(Token::Kind::DoubleQuote);

    std::vector<std::unique_ptr<BaseNode>> body;

    while(this->_cToken.kind() != Token::Kind::DoubleQuote){
        uint8_t c = this->_cToken.lexeme()[0];

        this->eat();

        auto E = std::make_unique<NumberExprNode>(std::to_string(int(c)));

        if(!E){
            std::cout << "Wrong string, body" << std::endl;
            return nullptr;
        }

        body.push_back(std::move(E));
    }

    this->expectNext(Token::Kind::DoubleQuote);

    return std::make_unique<ArrayInitNode>(std::move(body));
}

std::unique_ptr<BaseNode> Parser::parsePrimary() {
    switch (this->_cToken.kind()) {
        default:
        std::cout  << this->_cToken.kind() << "\n" ;
        return LogError("unknown token when expecting an expression");


        case Token::Kind::Identifier:   return parseIdentifier();
        case Token::Kind::SingleQuote:  return parseChar();
        case Token::Kind::DoubleQuote:  return parseString();
        case Token::Kind::LeftCurly:    return parseBlock();
        case Token::Kind::LeftParen:    return parseParen();
        case Token::Kind::Number:       return parseNumber();
        case Token::Kind::Array:        return parseArray();
        case Token::Kind::For:          return parseFor();
        case Token::Kind::Var:          return parseVar();
        case Token::Kind::If:           return parseIf();
    }

    return nullptr;
}

std::unique_ptr<BaseNode> Parser::parseUnary() {
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
        return std::make_unique<UnaryExprNode>(Opc, std::move(Operand));
    return nullptr;
}

int Parser::GetTokPrecedence(Token tok) {
    if (!(tok.kind() >= Token::Kind::LessThan && tok.kind() <= Token::Kind::Slash ||
     tok.kind() == Token::Kind::Colon ||
     tok.kind() == Token::Kind::Dot ||
     tok.kind() == Token::Kind::Equal))
        return -1;

    int TokPrec = this->_bOp[tok.lexeme()];
    if (TokPrec <= 0)
        return -1;
    return TokPrec;
}

 std::unique_ptr<BaseNode> Parser::parseBinOpRHS(int ExprPrec,
                                              std::unique_ptr<BaseNode> LHS) {
  
    while (true) {
        int TokPrec = GetTokPrecedence(this->_cToken);

        if (TokPrec < ExprPrec)
            return LHS;

        auto BinOp = this->_cToken;
        this->eat(); 

        auto RHS = parseUnary();
        if (!RHS)
            return nullptr;

        int NextPrec = GetTokPrecedence(this->_cToken);
        if (TokPrec < NextPrec) {
            RHS = parseBinOpRHS(TokPrec + 1, std::move(RHS));
            if (!RHS)
            return nullptr;
        }

        LHS =
            std::make_unique<BinaryExprNode>(BinOp, std::move(LHS), std::move(RHS));
    }

    // ok
    return nullptr;
}

std::unique_ptr<BaseNode> Parser::parseExpression() {
    auto LHS = parseUnary();
    if (!LHS)
        return nullptr;

    return parseBinOpRHS(0, std::move(LHS));
}

static std::unique_ptr<PrototypeNode> LogErrorP(const char *Str) {
  std::cout << "Error: " << (Str) << std::endl;
  return nullptr;
}

std::unique_ptr<PrototypeNode> Parser::parsePrototype() {

    std::string function_name;

    if(this->_cToken.kind() == Token::Kind::Identifier){
            function_name = std::string(this->_cToken.lexeme());
            this->eat();
    }else if(this->_cToken.kind() == Token::Kind::Master){
        function_name = "main";
        this->eat();
    }else return nullptr;


    if(!this->expectNext(Token::Kind::LeftParen, "in prototype").has_value())
       return nullptr;


    std::vector<std::pair<std::string, llvm::Type *>> ArgNames;
    while (this->_cToken.kind() == Token::Kind::Identifier)
    {

        auto _a = (this->_cToken.lexeme());
        this->eat();

        if(this->expectNext(Token::Kind::Colon).has_value()){

            ArgNames.push_back(
                std::make_pair<std::string, llvm::Type * >(
                    std::string(_a),
                    parseType()
                    ));
        }
    }

    if(!this->expectNext(Token::Kind::RightParen, "in prototype").has_value())
        return nullptr;

    llvm::Type * Ret_Type = nullptr;


    if (this->expectNext(Token::Kind::Cum).has_value())
            Ret_Type = parseType();
    else    Ret_Type = llvm::Type::getVoidTy(TheContext);

    

    return std::make_unique<PrototypeNode>(function_name, ArgNames, Ret_Type);
}

std::unique_ptr<PrototypeNode>  Parser::parseExtern() {
    this->expectNext(Token::Kind::Extern);
    return this->parsePrototype();
}

std::unique_ptr<FunctionNode> Parser::parseDef() {
    this->expectNext(Token::Kind::Def);

    auto Proto = parsePrototype();
    if (!Proto)
     return nullptr;

    std::unique_ptr<BaseNode> E;
    if(this->_cToken.kind() != Token::Kind::Pipe){
      
      E = parseExpression();
      if (!E)  std::cout << "Fucking bad" << std::endl;
         
    }
    return std::make_unique<FunctionNode>(std::move(Proto), std::move(E));
}

std::unique_ptr<FunctionNode> Parser::parseMain() {
    auto Proto = parsePrototype();
    if (!Proto)
     return nullptr;

    std::unique_ptr<BaseNode> E;
    if(this->_cToken.kind() != Token::Kind::Pipe){
      
      E = parseExpression();
      if (!E)  std::cout << "Fucking bad" << std::endl;
         
    }
    return std::make_unique<FunctionNode>(std::move(Proto), std::move(E));
}