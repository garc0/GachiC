#include "Visitor.h"

#include "../states.h"
#include "../AST/ast.h"


llvm::Function * VisitorFunction::operator()(std::nullptr_t &){
    return nullptr;
}

llvm::Function * VisitorFunction::operator()(PrototypeNode &prot_){
    std::vector<llvm::Type *> f_args;

    auto Name = prot_.getName();
    auto RetType = prot_.getTypeRet();

    std::variant<lvalue, rvalue> r{rvalue()};

    for(auto &i: prot_.Args){
        llvm::Type * ty = reinterpret_cast<llvm::Type*>(std::visit(VisitorExpr{}, *i.second.get(), r));

        f_args.push_back(ty);
    }

    llvm::Type * ret_ty = reinterpret_cast<llvm::Type*>(std::visit(VisitorExpr{}, *RetType, r));

    llvm::FunctionType *FT =
        llvm::FunctionType::get(ret_ty, f_args, false);

    llvm::Function *F =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Name, TheModule.get());

    // Set names for all arguments.
    unsigned Idx = 0;
    for (auto &Arg : F->args())
        Arg.setName(std::get<0>(prot_.Args[Idx++]));

    return F;
}

llvm::Function * VisitorFunction::operator()(FunctionNode & func_){
   
    auto Proto = func_.getProtoPtr();
    auto &P = *Proto.get();

    auto proto_name = std::visit(overload{
        [](PrototypeNode & p) -> std::string{
            return p.getName();
        },
        [](auto &) -> std::string{ return ""; }
    }, P); 

    FunctionProtos[proto_name] = std::move(Proto);
    llvm::Function *TheFunction = getFunction(proto_name);


    if (!TheFunction)
        return nullptr;

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext, "entry", TheFunction);
    Builder.SetInsertPoint(BB);

    std::variant<lvalue, rvalue> r{rvalue{}};

    NamedValues.clear();
    for (auto &Arg : TheFunction->args()) {

        auto arg_type = std::visit(overload{
            [&](PrototypeNode & p){
                return p.getArgType(Arg.getArgNo());
            },
            [](auto &) -> ASTNode*{ return nullptr; }
        }, *FunctionProtos[proto_name].get());

        
        llvm::Type * ty = reinterpret_cast<llvm::Type*>(std::visit(VisitorExpr{}, *arg_type, r));

        llvm::AllocaInst * Alloca = CreateEntryBlockAlloca(TheFunction, Arg.getName(), ty);

        Builder.CreateStore(&Arg, Alloca);

        std::get<0>(NamedValues[std::string(Arg.getName())]) = Alloca;
    }

    auto Body = func_.getBody();

    if (llvm::Value * RetVal = std::visit(VisitorExpr{}, *Body, r)) {
        verifyFunction(*TheFunction);
        return TheFunction;
    }

    TheFunction->eraseFromParent();

    return nullptr;
}


template<class T>
llvm::Value * VisitorExpr::operator()(std::nullptr_t &, T &){
    return nullptr;
}

template<class T>
llvm::Value * VisitorExpr::operator()(ArrayExprNode &node, T & is_l){

    llvm::Value * _v_ptr = nullptr;
    {
        auto _variable = NamedValues.find(node._var_name);

        if(_variable == NamedValues.end())
            return LogErrorV("Not found variable");
        
        _v_ptr = (_variable->second.first);
    }

    std::variant<lvalue, rvalue> r{rvalue{}};
    auto _i = std::visit(*this, *node._index.get(), r);

    llvm::Value * indexList[2] = { llvm::ConstantInt::get(TheContext, llvm::APInt(32, 0)), _i };

    if(!_v_ptr->getType()->isPointerTy())
        return LogErrorV("This is not a pointer, sucker!");

    auto _elem_ptr = Builder.CreateGEP(_v_ptr, indexList);


    if constexpr(std::is_same<lvalue, T>()) return _elem_ptr;
    else return Builder.CreateLoad(_elem_ptr);

}

template<class T>
llvm::Value * VisitorExpr::operator()(ArrayInitNode &node, T &){
    std::vector<llvm::Constant * > elems;

    std::variant<lvalue, rvalue> r{rvalue{}};
    for(int i = 0; i < node._elements.size(); i++){
        llvm::Value* to_push = std::visit(*this, *node._elements[i].get(), r);
        elems.push_back((llvm::Constant *)to_push);
    }

    auto const_array = llvm::ConstantArray::get(llvm::ArrayType::get(elems[0]->getType(), elems.size()), 
                                llvm::ArrayRef<llvm::Constant *>(elems.data(), elems.size()) );
    
    return const_array;
}

template<class T>
llvm::Value * VisitorExpr::operator()(BinaryExprNode &node, T &){

    if (node.Op.kind() == Token::Kind::Equal) {

        std::variant<lvalue, rvalue> l{lvalue{}};
        llvm::Value * Variable = std::visit(*this, *node.LHS.get(), l);    

        if(!Variable){
            std::cout << "lvalue is not valid" << std::endl;
            return nullptr;
        }

        if(!Variable->getType()->isPointerTy()){
            std::cout << "lvalue is not a pointer" << std::endl;
            return nullptr;
        }
        std::variant<lvalue, rvalue> r{rvalue{}};
        llvm::Value * Val = std::visit(*this, *node.RHS.get(), r);
        if (!Val)
            return nullptr;

        Builder.CreateStore(Val, Variable);
        return Variable;
    }


    if(node.Op.kind() == Token::Kind::Dot){
        
        std::variant<lvalue, rvalue> l{lvalue{}};
        llvm::Value * variable = std::visit(*this, *node.LHS.get(), l);

        if (!variable)
            return LogErrorV("Unknown variable name\n");

        llvm::Type * type_struct = variable->getType();

        llvm::Type * type_struct_p = type_struct;

        if(type_struct->isPointerTy()){
            type_struct_p = type_struct->getPointerElementType();
        }

        auto f_type_struct = StructFields.find(type_struct_p);

        if(f_type_struct == StructFields.end())
            return LogErrorV("Not found a struct def");

        auto struct_def = f_type_struct->second;

        auto name_var = std::visit(overload{
            [&](VariableExprNode &n) -> std::string{
                return n.getName();
            },
            [](auto &) -> std::string { return ""; }
        }, *node.RHS.get());

        std::size_t i = 0;

        while(i < struct_def.size() && name_var != struct_def[i].first) i++;

        if(i == struct_def.size())
            return LogErrorV("Not found a field of structure");
        
        llvm::Value * indexList[2] = {llvm::ConstantInt::get(TheContext, llvm::APInt(32, 0)), llvm::ConstantInt::get(TheContext, llvm::APInt(32, i))};

        auto elem_ptr = Builder.CreateGEP(variable, indexList);

        if constexpr(std::is_same<T, lvalue>()) return elem_ptr;
        else return Builder.CreateLoad(elem_ptr);
    }

    std::variant<lvalue, rvalue> r{rvalue{}};

    if(node.Op.kind() == Token::Kind::Ass){
        llvm::Value * L = std::visit(*this, *node.LHS.get(), r);
        llvm::Type * R = reinterpret_cast<llvm::Type*>(std::visit(*this, *node.RHS.get(), r));

        //llvm::Instruction::CastOps::
        return Builder.CreateBitCast(L, R);
    }
    
    llvm::Value * L = std::visit(*this, *node.LHS.get(), r);
    llvm::Value * R = std::visit(*this, *node.RHS.get(), r);
    if (!L || !R)  return nullptr;


    if(L->getType() != R->getType()){
      std::cout << "L->getType() != R->getType()" << std::endl;
    }

    if(!L->getType()->isFloatingPointTy()){
        switch (node.Op.kind()) {
        case Token::Kind::Plus:
            return Builder.CreateAdd(L, R, "addu");
        case Token::Kind::Minus:
            return Builder.CreateSub(L, R, "subu");
        case Token::Kind::Asterisk:
            return Builder.CreateMul(L, R, "mulu");
        case Token::Kind::Slash:
            return Builder.CreateUDiv(L, R, "divu");
        case Token::Kind::Modulo:
            return Builder.CreateURem(L, R, "modu");
        case Token::Kind::LessThan:
            return Builder.CreateICmpSLT(L, R, "cmpu");
        case Token::Kind::GreaterThan:
            return Builder.CreateICmpSLT(R, L, "cmpu");
        default:
            break;
        }
    }else{
        switch (node.Op.kind()) {
        case Token::Kind::Plus:
            return Builder.CreateFAdd(L, R, "addf");
        case Token::Kind::Minus:
            return Builder.CreateFSub(L, R, "subf");
        case Token::Kind::Asterisk:
            return Builder.CreateFMul(L, R, "mulf");
        case Token::Kind::Slash:
            return Builder.CreateFDiv(L, R, "divf");
        case Token::Kind::Modulo:
            return nullptr;
        case Token::Kind::LessThan:
            return Builder.CreateFCmpOLT(L, R, "cmpf");
        case Token::Kind::GreaterThan:
            return Builder.CreateFCmpOLT(R, L, "cmpf");
        default:
            break;
        }
    }

    return LogErrorV("What the hell");

}

template<class T>
llvm::Value * VisitorExpr::operator()(BlockNode &node, T &){
    
    llvm::Value * ValRet = nullptr;

    std::variant<lvalue, rvalue> r{rvalue{}};

    for(int i = 0; i < node.l.size(); i++){
        if(!(ValRet = std::visit(*this, *node.l[i].get(), r)))
            return nullptr;
    }

    for (unsigned i = 0, e = node.VarNames.size(); i != e; ++i)
      std::get<0>(NamedValues[node.VarNames[i].first]) = node.OldBindings[i];

    return ValRet;
}

template<class T>
llvm::Value * VisitorExpr::operator()(CallExprNode &node, T &){

    llvm::Function *CalleeF = getFunction(node.Callee);
    if (!CalleeF)
        return LogErrorV("Unknown function referenced");

    // If argument mismatch error.
    if (CalleeF->arg_size() != node.Args.size())
        return LogErrorV("Incorrect # arguments passed");

    std::variant<lvalue, rvalue> r{rvalue{}};

    std::vector<llvm::Value *> ArgsV;
    for (unsigned i = 0, e = node.Args.size(); i != e; ++i) {
        ArgsV.push_back(std::visit(*this, *node.Args[i].get(), r));
        if (!ArgsV.back())
        return nullptr;
    }

    return Builder.CreateCall(CalleeF, ArgsV);
}

template<typename T>
inline bool is_in_range(T v){
    return (v >= std::numeric_limits<T>::lowest()) && (v <= std::numeric_limits<T>::max());
}
template<class T>
llvm::Value * VisitorExpr::operator()(NumberExprNode &node, T &){

    bool is_float = node._val.find('.') != std::string::npos;

    if(is_float){
        auto d = std::stod(node._val);

        if(is_in_range(float(d))) return llvm::ConstantFP::get(TheContext, llvm::APFloat(float(d)));
        if(is_in_range(double(d))) return llvm::ConstantFP::get(TheContext, llvm::APFloat(double(d)));

        return nullptr;
    }
    
    auto i = std::stoll(node._val);
    //if(is_in_range<uint8_t>(uint8_t(i))) return llvm::ConstantInt::get(TheContext, llvm::APInt(8, i));
    //if(is_in_range<uint16_t>(uint16_t(i))) return llvm::ConstantInt::get(TheContext, llvm::APInt(16, i));
    if(is_in_range<uint32_t>(uint32_t(i))) return llvm::ConstantInt::get(TheContext, llvm::APInt(32, i));
    if(is_in_range<uint64_t>(uint64_t(i))) return llvm::ConstantInt::get(TheContext, llvm::APInt(64, i));
    return nullptr;
}


template<class T>
llvm::Value * VisitorExpr::operator()(CharNode &node, T &){
    return llvm::ConstantInt::get(TheContext, llvm::APInt(8, node._val));
}

template<class T>
llvm::Value * VisitorExpr::operator()(StructNode &node, T &){
    auto struct_val = llvm::StructType::create(TheContext, node._struct_name.c_str());

    NamedStructures[node._struct_name] = (llvm::Type * )struct_val;
 
    std::vector<std::pair<std::string, llvm::Type*>> type_elements;

    std::variant<lvalue, rvalue> r{rvalue()};

    for(auto &i: node._elements)
    {
        type_elements.push_back({i.first,
        reinterpret_cast<llvm::Type*>(
            std::visit(*this, *i.second.get(), r)
        )});
    }

    StructFields[struct_val] = type_elements;


    std::vector<llvm::Type * > struct_body;

    for(auto & i: type_elements){
        struct_body.push_back(i.second);
    }

    struct_val->setBody(struct_body);

    return (llvm::Value * )struct_val;
}

template<class T>
llvm::Value * VisitorExpr::operator()(StructExprNode &node, T &){

    auto struct_type = NamedStructures.find(node.Name);

    if(struct_type == NamedStructures.end()){
        std::cout << "Invalid structure name" << std::endl;
        return nullptr;
    }

    std::vector<llvm::Constant *> fields;

    std::variant<lvalue, rvalue> r{rvalue{}};
    for(auto & i : node._elements){
        fields.push_back((llvm::Constant *)(std::visit(*this, *i.second.get(), r)));
    }

    auto const_struct = llvm::ConstantStruct::get(
        (llvm::StructType*)struct_type->second, 
        llvm::ArrayRef<llvm::Constant *>(fields.data(), fields.size())
        );

    return const_struct;
}

template<class T>
llvm::Value * VisitorExpr::operator()(UnaryExprNode &node, T &){


    if(node.Opcode.is_kind(Token::Kind::Return)){
        std::variant<lvalue, rvalue> r{rvalue{}};
        llvm::Value * OperandV = std::visit(*this, *node.Operand.get(), r);
        return Builder.CreateRet(OperandV);
    }

    if(node.Opcode.is_kind(Token::Kind::Ampersand)){
        std::variant<lvalue, rvalue> l{lvalue{}};
        return std::visit(*this, *node.Operand.get(), l);
    }

    if(node.Opcode.is_kind(Token::Kind::Asterisk)){
        std::variant<lvalue, rvalue> r{rvalue{}};
        llvm::Value * OperandV = std::visit(*this, *node.Operand.get(), r);

        if(!OperandV) return nullptr;

        if constexpr (std::is_same<T, lvalue>())
             return OperandV;
        else return Builder.CreateLoad(OperandV);
    }

    if(node.Opcode.is_kind(Token::Kind::Minus)){
        std::variant<lvalue, rvalue> r{rvalue{}};
        llvm::Value * OperandV = std::visit(*this, *node.Operand.get(), r);

        if(!OperandV) return nullptr;

        if(OperandV->getType()->isFloatingPointTy())
        return Builder.CreateFNeg(OperandV);
        else return Builder.CreateNeg(OperandV);
    }

    if(node.Opcode.is_kind(Token::Kind::Not)){
        std::variant<lvalue, rvalue> r{rvalue{}};
        llvm::Value * OperandV = std::visit(*this, *node.Operand.get(), r);

        if(!OperandV) return nullptr;

        return Builder.CreateNot(OperandV);
    }

    return nullptr;
}

template<class T>
llvm::Value * VisitorExpr::operator()(VarExprNode &node, T &){
    llvm::Function * TheFunction = Builder.GetInsertBlock()->getParent();

    llvm::Value * ValToRet;

    for (unsigned i = 0, e = node.VarNames.size(); i != e; ++i) {
        const std::string &VarName = node.VarNames[i].first;
        ASTNode * Init = node.VarNames[i].second.get();

        llvm::Value * InitVal;
        if (Init) {
            if (!InitVal)
            return nullptr;
    } else 
        InitVal = llvm::ConstantInt::get(TheContext, llvm::APInt(64, (uint64_t)0));

    std::variant<lvalue, rvalue> r{rvalue{}};
    InitVal = std::visit(*this, *Init, r);

    llvm::AllocaInst * Alloca = CreateEntryBlockAlloca(TheFunction, "", InitVal->getType());

    Builder.CreateStore(InitVal, Alloca);

    std::visit(overload{
        [&](BlockNode &node){
            node.OldBindings.push_back(std::get<0>(NamedValues[VarName]));
        },
        [](auto &){ return; }
        }, *node.l_block);


    ValToRet = std::get<0>(NamedValues[VarName]) = Alloca;
    }

    std::visit(overload{
    [&](BlockNode &bl){
        bl.VarNames = std::move(node.VarNames);
    },
    [](auto &){ return; }
    }, *node.l_block);  


    return ValToRet;
}

template<class T>
llvm::Value * VisitorExpr::operator()(VariableExprNode &node, T &){
    auto f = NamedValues.find(node.Name);
    if(f == NamedValues.end()){
        std::cerr << "Unknown variable name" << std::endl;
        return nullptr;
    }

    llvm::Value * V = std::get<0>(f->second);

    if (!V){
        std::cerr << "Variable not defined" << std::endl;
        return nullptr;
    }

    if constexpr (std::is_same<T, lvalue>()) return V;
    else return Builder.CreateLoad(V);
}

template<class T>
llvm::Value * VisitorExpr::operator()(WhileExpr &node, T &){
    llvm::Function * TheFunction = 
        Builder.GetInsertBlock()->getParent();

    llvm::BasicBlock * loop_bb = 
        llvm::BasicBlock::Create(TheContext, "loop", TheFunction);

    llvm::BasicBlock * after_bb =
        llvm::BasicBlock::Create(TheContext, "after_loop", TheFunction);

    {
        std::variant<lvalue, rvalue> r{rvalue{}};
        auto * cond_val = std::visit(*this, *node._cond.get(), r);
        if(!cond_val){
            return nullptr;
        }
        Builder.CreateCondBr(cond_val, loop_bb, after_bb);
    }

    Builder.SetInsertPoint(loop_bb);

    std::variant<lvalue, rvalue> r{rvalue{}};

    if(!std::visit(*this, *node._body.get(), r))
        return nullptr;
    
    if(node._step.has_value() && node._step.value())
        if(!std::visit(*this, *node._step.value().get(), r))
            return nullptr;

    auto * cond_val = std::visit(*this, *node._cond.get(), r);
    if(!cond_val){
        return nullptr;
    }


    Builder.CreateCondBr(cond_val, loop_bb, after_bb);

    Builder.SetInsertPoint(after_bb);

    return llvm::Constant::getNullValue(llvm::Type::getInt64Ty(TheContext));
}

template<class T>
llvm::Value * VisitorExpr::operator()(ForExpr &node, T &){
    llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();

    std::variant<lvalue, rvalue> r{rvalue{}};

    llvm::Value * StartVal = std::visit(*this, *node.Start.get(), r);
    if (!StartVal)
        return nullptr;

    llvm::AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, node.VarName, StartVal->getType());

    Builder.CreateStore(StartVal, Alloca);
    llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(TheContext, "loop", TheFunction);

    Builder.CreateBr(LoopBB);

    Builder.SetInsertPoint(LoopBB);

    llvm::AllocaInst * OldVal = std::get<0>(NamedValues[node.VarName]);
    std::get<0>(NamedValues[node.VarName]) = Alloca;


    if (std::visit(*this, *node.Body.get(), r))
        return nullptr;

    llvm::Value * StepVal = nullptr;
    if (node.Step) {
        StepVal = std::visit(*this, *node.Step.get(), r);
        if (!StepVal)
         return nullptr;
    } else 
        StepVal = llvm::ConstantInt::get(TheContext, llvm::APInt(64, 1));

    llvm::Value * EndCond = std::visit(*this, *node.End.get(), r);
    if (!EndCond)
        return nullptr;


    llvm::Value *CurVar = Builder.CreateLoad(Alloca, node.VarName.c_str());
    llvm::Value *NextVar = Builder.CreateAdd(CurVar, StepVal, "next_var");
    Builder.CreateStore(NextVar, Alloca);

    llvm::BasicBlock *AfterBB =
    llvm::BasicBlock::Create(TheContext, "after_loop", TheFunction);

    Builder.CreateCondBr(EndCond, LoopBB, AfterBB);

    Builder.SetInsertPoint(AfterBB);

    if (OldVal)
    std::get<0>(NamedValues[node.VarName]) = OldVal;
    else
    NamedValues.erase(node.VarName);

    return llvm::Constant::getNullValue(llvm::Type::getInt64Ty(TheContext));
}

template<class T>
llvm::Value * VisitorExpr::operator()(IfExpr &node, T &){

    std::variant<lvalue, rvalue> r{rvalue{}};
    llvm::Value *CondV = std::visit(*this, *node.Cond.get(), r);
    if (!CondV)
    return nullptr;


    llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(TheContext, "then", TheFunction);

    llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(TheContext, "else");

    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(TheContext, "if_cont");

    if(node.Else)
        Builder.CreateCondBr(CondV, ThenBB, ElseBB);
    else Builder.CreateCondBr(CondV, ThenBB, MergeBB);

    Builder.SetInsertPoint(ThenBB);

    llvm::Value *ThenV = std::visit(*this, *node.Then.get(), r);

    if (!ThenV)
        return nullptr;

    if(node.Else){
        Builder.CreateBr(MergeBB);

        TheFunction->getBasicBlockList().push_back(ElseBB);
        Builder.SetInsertPoint(ElseBB);
    
        llvm::Value *ElseV = std::visit(*this, *node.Else.get(), r);
        if (!ElseV) return nullptr;
    }

    
    Builder.CreateBr(MergeBB);

    TheFunction->getBasicBlockList().push_back(MergeBB);
    Builder.SetInsertPoint(MergeBB);


    return llvm::Constant::getNullValue(llvm::Type::getInt64Ty(TheContext));
}

#include <functional>

template<class T> llvm::Value * VisitorExpr::operator()(TypeNode &node, T &){

    using it = std::vector<TypeNode::type_pair>::iterator;

    std::function<llvm::Type*(it, it)> _helper = [&](auto tv, auto end) -> llvm::Type*{
        if(tv == end) return nullptr;
        if(tv->first == TypeNode::type_id::pointer){
            return llvm::PointerType::get(_helper(++tv, end), 0);
        }

        auto ty = tv->first;

        if(ty == TypeNode::type_id::nothing) return llvm::Type::getVoidTy(TheContext);

        if(ty == TypeNode::type_id::u1) return llvm::Type::getInt8Ty(TheContext);

        if(ty == TypeNode::type_id::u8)  return llvm::Type::getInt8Ty(TheContext);
        if(ty == TypeNode::type_id::u16) return llvm::Type::getInt16Ty(TheContext);
        if(ty == TypeNode::type_id::u32) return llvm::Type::getInt32Ty(TheContext);
        if(ty == TypeNode::type_id::u64) return llvm::Type::getInt64Ty(TheContext);

        if(ty == TypeNode::type_id::i8)  return llvm::Type::getInt8Ty(TheContext);
        if(ty == TypeNode::type_id::i16) return llvm::Type::getInt16Ty(TheContext);
        if(ty == TypeNode::type_id::i32) return llvm::Type::getInt32Ty(TheContext);
        if(ty == TypeNode::type_id::i64) return llvm::Type::getInt64Ty(TheContext);

        if(ty == TypeNode::type_id::f32) return llvm::Type::getFloatTy(TheContext);
        if(ty == TypeNode::type_id::f64) return llvm::Type::getDoubleTy(TheContext);

        auto f = NamedStructures.find(tv->second);

        if(f != NamedStructures.end())
            return f->second;

        return nullptr;
    };

    //dick move
    return reinterpret_cast<llvm::Value*>(_helper(node._t.begin(), node._t.end()));
}