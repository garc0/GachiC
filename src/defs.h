#ifndef DEFS_H
#define DEFS_H
#pragma once

#include <variant>

class Node;
class ArrayExprNode;
class ArrayInitNode;
class BinaryExprNode;
class BlockNode;
class ForExpr;
class IfExpr;
class CallExprNode;
class NumberExprNode;
class StructNode;
class UnaryExprNode;
class VarExprNode;
class StructExprNode;
class VariableExprNode;
class WhileExpr;
class TypeNode;
class CharNode;
class StringNode;

class PrototypeNode;
class FunctionNode;


// copypasta
template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

using ASTNode = std::variant<
            //std::nullptr_t, 
            ArrayExprNode, 
            ArrayInitNode,
            BinaryExprNode,
            BlockNode,
            ForExpr,
            IfExpr,
            CallExprNode,
            NumberExprNode,
            CharNode,
            StringNode,
            StructNode,
            StructExprNode,
            UnaryExprNode,
            VarExprNode,
            VariableExprNode,
            WhileExpr,
            TypeNode
            >;

using DefNode = std::variant<
            //std::nullptr_t,
            PrototypeNode, FunctionNode>;


#endif