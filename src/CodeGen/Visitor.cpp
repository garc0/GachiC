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

    for(auto &i: prot_.Args){
        llvm::Type * ty = reinterpret_cast<llvm::Type*>(VisitorExpr{}.expr_visit(*i.second.get()));

        f_args.push_back(ty);
    }

    llvm::Type * ret_ty = reinterpret_cast<llvm::Type*>(VisitorExpr{}.expr_visit(*RetType));

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

    NamedValues.clear();
    for (auto &Arg : TheFunction->args()) {

        auto arg_type = std::visit(overload{
            [&](PrototypeNode & p){
                return p.getArgType(Arg.getArgNo());
            },
            [](auto &) -> ASTNode*{ return nullptr; }
        }, *FunctionProtos[proto_name].get());

        
        llvm::Type * ty = reinterpret_cast<llvm::Type*>(VisitorExpr{}.expr_visit(*arg_type));

        llvm::AllocaInst * Alloca = CreateEntryBlockAlloca(TheFunction, Arg.getName(), ty);

        Builder.CreateStore(&Arg, Alloca);

        std::get<0>(NamedValues[std::string(Arg.getName())]) = Alloca;
    }

    auto Body = func_.getBody();

    if (llvm::Value * RetVal = VisitorExpr{}.expr_visit(*Body)) {
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

    std::vector<llvm::Value *> indexList = { llvm::ConstantInt::get(TheContext, llvm::APInt(32, 0)) };

    for(auto &i: node._indexes){
        indexList.push_back(this->expr_visit(*i.get()));
    }

    if(!_v_ptr->getType()->isPointerTy())
        return LogErrorV("This is not a pointer, sucker!");

    auto _elem_ptr = Builder.CreateGEP(_v_ptr, indexList);


    if constexpr(std::is_same<lvalue, T>()) return _elem_ptr;
    else return Builder.CreateLoad(_elem_ptr);

}

template<class T>
llvm::Value * VisitorExpr::operator()(ArrayInitNode &node, T &){
    std::vector<llvm::Constant * > elems;

    for(int i = 0; i < node._elements.size(); i++){
        llvm::Value* to_push = this->expr_visit(*node._elements[i].get());
        elems.push_back((llvm::Constant *)to_push);
    }

    auto const_array = llvm::ConstantArray::get(llvm::ArrayType::get(elems[0]->getType(), elems.size()), 
                                llvm::ArrayRef<llvm::Constant *>(elems.data(), elems.size()) );
    
    return const_array;
}

template<class T>
llvm::Value * VisitorExpr::operator()(StringNode &node, T &){
    return Builder.CreateGlobalStringPtr(node._val);
}

template<class T>
llvm::Value * VisitorExpr::operator()(BinaryExprNode &node, T &){

    if (node.Op.is_kind(Token::Kind::Equal)) {
        llvm::Value * Variable = this->expr_visit<lvalue>(*node.LHS.get());    

        if(!Variable){
            std::cerr << "lvalue is not valid" << std::endl;
            return nullptr;
        }

        if(!Variable->getType()->isPointerTy()){
            std::cerr << "lvalue is not a pointer" << std::endl;
            return nullptr;
        }
        llvm::Value * Val = this->expr_visit(*node.RHS.get());
        if (!Val)
            return nullptr;

        Builder.CreateStore(Val, Variable);
        return Variable;
    }


    if(node.Op.is_kind(Token::Kind::Dot)){
        
        llvm::Value * variable = this->expr_visit<lvalue>(*node.LHS.get());

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
        
        llvm::Value * indexList[2] = {
            llvm::ConstantInt::get(TheContext, llvm::APInt(32, 0)), 
            llvm::ConstantInt::get(TheContext, llvm::APInt(32, i))
            };

        auto elem_ptr = Builder.CreateGEP(variable, indexList);

        if constexpr(std::is_same<T, lvalue>()) return elem_ptr;
        else return Builder.CreateLoad(elem_ptr);
    }

    if(node.Op.is_kind(Token::Kind::Ass)){
        llvm::Value * L = this->expr_visit(*node.LHS.get());
        llvm::Type * R = reinterpret_cast<llvm::Type*>(this->expr_visit(*node.RHS.get()));

        if(L->getType()->isIntegerTy()){
            if(R->isPointerTy())
                return Builder.CreateCast(llvm::Instruction::CastOps::IntToPtr, L, R);
            if(R->isFloatTy() || R->isDoubleTy())
                return Builder.CreateCast(llvm::Instruction::CastOps::SIToFP, L, R);
            if(R->isIntegerTy())
                return Builder.CreateCast(llvm::Instruction::CastOps::SExt, L, R);
        }

        if(L->getType()->isFloatTy() || L->getType()->isDoubleTy()){
            if(R->isFloatTy() || R->isDoubleTy())
                return Builder.CreateCast(llvm::Instruction::CastOps::SExt, L, R);
            if(R->isIntegerTy())
                return Builder.CreateCast(llvm::Instruction::CastOps::FPToSI, L, R);
        }

        if(L->getType()->isPointerTy()) {
            if(R->isIntegerTy())
                return Builder.CreateCast(llvm::Instruction::CastOps::PtrToInt, L, R);
        }
        return Builder.CreateBitCast(L, R);
    }
    
    llvm::Value * L = this->expr_visit(*node.LHS.get());
    llvm::Value * R = this->expr_visit(*node.RHS.get());
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
            return Builder.CreateCmp(llvm::CmpInst::Predicate::ICMP_SLT, L, R, "cmpu");
        case Token::Kind::GreaterThan:
            return Builder.CreateCmp(llvm::CmpInst::Predicate::ICMP_SGT, L, R, "cmpu");
        case Token::Kind::LessEqual:
            return Builder.CreateCmp(llvm::CmpInst::Predicate::ICMP_SLE, L, R, "cmpu");
        case Token::Kind::GreaterEqual:
            return Builder.CreateCmp(llvm::CmpInst::Predicate::ICMP_SGE, L, R, "cmpu");
        case Token::Kind::DoubleEqual:
            return Builder.CreateCmp(llvm::CmpInst::Predicate::ICMP_EQ, L, R, "equ");
        case Token::Kind::DoublePipe:
            return Builder.CreateOr(L, R, "oru");
        case Token::Kind::DoubleAmpersand:
            return Builder.CreateAnd(L, R, "andu");
        case Token::Kind::NotEqual:
            return Builder.CreateCmp(llvm::CmpInst::Predicate::ICMP_NE, L, R, "neu");
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
            return Builder.CreateCmp(llvm::CmpInst::Predicate::FCMP_OLT, L, R, "cmpf");
        case Token::Kind::GreaterThan:
            return Builder.CreateCmp(llvm::CmpInst::Predicate::FCMP_OGT, L, R, "cmpf");
        case Token::Kind::LessEqual:
            return Builder.CreateCmp(llvm::CmpInst::Predicate::FCMP_OLE, L, R, "cmpf");
        case Token::Kind::GreaterEqual:
            return Builder.CreateCmp(llvm::CmpInst::Predicate::FCMP_OGE, L, R, "cmpf");
        case Token::Kind::DoubleEqual:
            return Builder.CreateCmp(llvm::CmpInst::Predicate::FCMP_OEQ, L, R, "eqf");
        case Token::Kind::NotEqual:
            return Builder.CreateCmp(llvm::CmpInst::Predicate::FCMP_ONE, L,  R, "nef");
        default:
            break;
        }
    }

    return LogErrorV("What the hell");

}

template<class T>
llvm::Value * VisitorExpr::operator()(BlockNode &node, T &){
    
    llvm::Value * ValRet = nullptr;

    for(int i = 0; i < node.l.size(); i++){
        if(!(ValRet = this->expr_visit(*node.l[i].get())))
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

    std::vector<llvm::Value *> ArgsV;
    for (unsigned i = 0, e = node.Args.size(); i != e; ++i) {
        ArgsV.push_back(this->expr_visit(*node.Args[i].get()));
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

        if(is_in_range(float(d))) return llvm::ConstantFP::get(llvm::Type::getFloatTy(TheContext), d);
        if(is_in_range(double(d))) return llvm::ConstantFP::get(llvm::Type::getDoubleTy(TheContext), d);

    }else{
        auto i = std::stoll(node._val);
        //if(is_in_range<uint8_t>(uint8_t(i))) return llvm::ConstantInt::get(TheContext, llvm::APInt(8, i));
        //if(is_in_range<uint16_t>(uint16_t(i))) return llvm::ConstantInt::get(TheContext, llvm::APInt(16, i));
        if(is_in_range<uint32_t>(uint32_t(i))) return llvm::ConstantInt::get(TheContext, llvm::APInt(32, i));
        if(is_in_range<uint64_t>(uint64_t(i))) return llvm::ConstantInt::get(TheContext, llvm::APInt(64, i));
    }
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

    for(auto &i: node._elements)
    {
        type_elements.push_back({i.first,
        reinterpret_cast<llvm::Type*>(
            this->expr_visit(*i.second.get())
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

    for(auto & i : node._elements){
        fields.push_back((llvm::Constant *)(this->expr_visit(*i.second.get())));
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
        llvm::Value * OperandV = this->expr_visit(*node.Operand.get());

        if(!OperandV) return nullptr;

        return Builder.CreateRet(OperandV);
    }

    if(node.Opcode.is_kind(Token::Kind::Ampersand)){
        return this->expr_visit<lvalue>(*node.Operand.get());
    }

    if(node.Opcode.is_kind(Token::Kind::Asterisk)){
        llvm::Value * OperandV = this->expr_visit(*node.Operand.get());

        if(!OperandV) return nullptr;

        if constexpr (std::is_same<T, lvalue>())
             return OperandV;
        else return Builder.CreateLoad(OperandV);
    }

    if(node.Opcode.is_kind(Token::Kind::Minus)){
        llvm::Value * OperandV = this->expr_visit(*node.Operand.get());

        if(!OperandV) return nullptr;

        if(OperandV->getType()->isFloatingPointTy())
        return Builder.CreateFNeg(OperandV);
        else return Builder.CreateNeg(OperandV);
    }

    if(node.Opcode.is_kind(Token::Kind::Not)){
        llvm::Value * OperandV = this->expr_visit(*node.Operand.get());

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

        InitVal = this->expr_visit(*Init);

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
        for(auto &i : node.VarNames)
            bl.VarNames.push_back(std::move(i));
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
llvm::Value * VisitorExpr::operator()(StickNode &node, T &){
    
    llvm::Value * ValToRet = nullptr;

    llvm::Function * TheFunction = Builder.GetInsertBlock()->getParent();

    const std::string &VarName = node._val.first;

    if(!node._to_alloc){
        llvm::Function * CalleeF = getFunction("free");

        if (!CalleeF){
            std::vector<llvm::Type*> f_args = {
                llvm::Type::getInt8PtrTy(TheContext)
            };

            llvm::Type * ret_ty = llvm::Type::getVoidTy(TheContext);

            llvm::FunctionType *FT =
                llvm::FunctionType::get(ret_ty, f_args, false);

            CalleeF =
                llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "free", TheModule.get());
        }

        std::vector<llvm::Value *> ArgsV{
           Builder.CreateBitCast(std::get<0>(NamedValues[VarName]), llvm::Type::getInt8PtrTy(TheContext))
        };

        Builder.CreateCall(CalleeF, ArgsV);

        return llvm::Constant::getNullValue(llvm::Type::getInt64Ty(TheContext));
    }

    auto ty = reinterpret_cast<llvm::Type*>(this->expr_visit(*node._val.second.get()));

    auto ty_size = TheModule->getDataLayout().getTypeAllocSize(ty);

    llvm::Function * CalleeF = getFunction("malloc");
    if (!CalleeF){
        std::vector<llvm::Type*> f_args = {
            llvm::Type::getInt64Ty(TheContext)
        };

        llvm::Type * ret_ty = llvm::Type::getInt8PtrTy(TheContext);

        llvm::FunctionType *FT =
            llvm::FunctionType::get(ret_ty, f_args, false);

        CalleeF =
            llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "malloc", TheModule.get());

    }

    llvm::Value * alloc_size = llvm::ConstantInt::get(TheContext, llvm::APInt(sizeof(std::size_t) * 8, ty_size.getFixedSize()));

    if(node._size_arr != nullptr){
        auto szArr = this->expr_visit(*node._size_arr.get());
        alloc_size = (llvm::Value *)Builder.CreateMul(alloc_size, szArr);
    }

    std::vector<llvm::Value *> ArgsV{
        alloc_size
        };

    llvm::AllocaInst * Alloca = (llvm::AllocaInst *)Builder.CreateCall(CalleeF, ArgsV);

    if(node._size_arr != nullptr)
        ty = llvm::ArrayType::get(ty, 0);

    Alloca = (llvm::AllocaInst *)Builder.CreateBitCast(Alloca, ty->getPointerTo());

    llvm::Value * InitVal = Builder.CreateLoad(Alloca);

    std::visit(overload{
        [&](BlockNode &node){
            node.OldBindings.push_back(std::get<0>(NamedValues[VarName]));
        },
        [](auto &){ return; }
        }, *node._block);


    ValToRet = std::get<0>(NamedValues[VarName]) = Alloca;

    std::visit(overload{
        [&](BlockNode &bl){
            bl.VarNames.push_back(std::move(node._val));
        },
        [](auto &){ return; }
    }, *node._block);  

    return ValToRet;
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
        auto * cond_val = this->expr_visit(*node._cond.get());
        if(!cond_val){
            return nullptr;
        }
        Builder.CreateCondBr(cond_val, loop_bb, after_bb);
    }

    Builder.SetInsertPoint(loop_bb);

    if(!this->expr_visit(*node._body.get()))
        return nullptr;
    
    if(node._step.has_value() && node._step.value())
        if(!this->expr_visit(*node._step.value().get()))
            return nullptr;

    auto * cond_val = this->expr_visit(*node._cond.get());
    if(!cond_val)
        return nullptr;

    Builder.CreateCondBr(cond_val, loop_bb, after_bb);

    Builder.SetInsertPoint(after_bb);

    return llvm::Constant::getNullValue(llvm::Type::getInt64Ty(TheContext));
}

template<class T>
llvm::Value * VisitorExpr::operator()(ForExpr &node, T &){
    llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();

    llvm::Value * StartVal = this->expr_visit(*node.Start.get());
    if (!StartVal)
        return nullptr;

    llvm::AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, node.VarName, StartVal->getType());

    Builder.CreateStore(StartVal, Alloca);
    llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(TheContext, "loop", TheFunction);

    Builder.CreateBr(LoopBB);

    Builder.SetInsertPoint(LoopBB);

    llvm::AllocaInst * OldVal = std::get<0>(NamedValues[node.VarName]);
    std::get<0>(NamedValues[node.VarName]) = Alloca;


    if (this->expr_visit(*node.Body.get()))
        return nullptr;

    llvm::Value * StepVal = nullptr;
    if (node.Step) {
        StepVal = this->expr_visit(*node.Step.get());
        if (!StepVal)
         return nullptr;
    } else 
        StepVal = llvm::ConstantInt::get(TheContext, llvm::APInt(64, 1));

    llvm::Value * EndCond = this->expr_visit(*node.End.get());
    if (!EndCond)
        return nullptr;


    llvm::Value * CurVar = Builder.CreateLoad(Alloca, node.VarName.c_str());
    llvm::Value * NextVar = Builder.CreateAdd(CurVar, StepVal, "next_var");
    Builder.CreateStore(NextVar, Alloca);

    llvm::BasicBlock * AfterBB =
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

    llvm::Value *CondV = this->expr_visit(*node.Cond.get());
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

    llvm::Value *ThenV = this->expr_visit(*node.Then.get());

    if (!ThenV)
        return nullptr;

    if(node.Else){
        Builder.CreateBr(MergeBB);

        TheFunction->getBasicBlockList().push_back(ElseBB);
        Builder.SetInsertPoint(ElseBB);
    
        llvm::Value *ElseV = this->expr_visit(*node.Else.get());
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

        if(tv->first == TypeNode::type_id::array){
            std::size_t sz = std::stoll(tv->second.c_str());
            return llvm::ArrayType::get(_helper(++tv, end), sz);
        }

        switch(tv->first){
            case TypeNode::type_id::nothing: return llvm::Type::getVoidTy(TheContext);
            case TypeNode::type_id::u1: return llvm::Type::getInt8Ty(TheContext);

            case TypeNode::type_id::u8:  return llvm::Type::getInt8Ty(TheContext);
            case TypeNode::type_id::u16: return llvm::Type::getInt16Ty(TheContext);
            case TypeNode::type_id::u32: return llvm::Type::getInt32Ty(TheContext);
            case TypeNode::type_id::u64: return llvm::Type::getInt64Ty(TheContext); 

            case TypeNode::type_id::i8:  return llvm::Type::getInt8Ty(TheContext);
            case TypeNode::type_id::i16: return llvm::Type::getInt16Ty(TheContext);
            case TypeNode::type_id::i32: return llvm::Type::getInt32Ty(TheContext);
            case TypeNode::type_id::i64: return llvm::Type::getInt64Ty(TheContext); 
            case TypeNode::type_id::f32: return llvm::Type::getFloatTy(TheContext);
            case TypeNode::type_id::f64: return llvm::Type::getDoubleTy(TheContext);
        }

        auto f = NamedStructures.find(tv->second);

        if(f != NamedStructures.end())
            return f->second;
        return nullptr;
    };

    //dick move
    return reinterpret_cast<llvm::Value*>(_helper(node._t.begin(), node._t.end()));
}