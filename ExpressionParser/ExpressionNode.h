#pragma once
#include <iostream>


enum class EExprNodeType
{
    Expression,
    Operand,
};


template<typename T>
struct TExpressionNode
{
    using TValue = T;
    //using BaseNodeType = TExpressionNode<TValue>;

    virtual T Eval() const = 0;
    virtual EExprNodeType GetNodeType() const = 0;
};

template<typename T, template<typename> typename TBaseNode = TExpressionNode>
struct TOperand : public TBaseNode<T>
{
    using BaseNodeType = TBaseNode<T>;

    using TValue = TBaseNode<T>::TValue;

    TValue Value;
    TOperand(TValue&& InValue = TValue{}) : Value(std::forward<TValue>(InValue)) {}

    virtual TValue Eval() const
    {
        return Value;
    }
    virtual EExprNodeType GetNodeType() const final
    {
        return EExprNodeType::Operand;
    }

    template<typename TRhs>
    auto& operator=(const TBaseNode<TRhs>& Rhs)
    {
        Value = Rhs.Eval();
        return *this;
    }
};

template<typename TValue, template<typename> typename TBaseNode>
struct TExpression : public TBaseNode<TValue>
{
    using BaseNodeType = TBaseNode<TValue>;

    virtual EExprNodeType GetNodeType() const final
    {
        return EExprNodeType::Expression;
    }
};

template<template<typename> class TBaseNode, typename TLhs, typename TRhs, typename TResult>
struct TBinaryExpression : public TExpression<TResult, TBaseNode>
{
    const TBaseNode<TLhs>& Lhs;
    const TBaseNode<TRhs>& Rhs;
    TBinaryExpression(const TBaseNode<TLhs>& InLhs, const TBaseNode<TRhs>& InRhs) : Lhs(InLhs), Rhs(InRhs) {}
};

template<typename TValue, typename TResult, template<typename> typename TBaseNode>
struct TPreUnaryExpression : public TExpression<TResult, TBaseNode>
{
    const TBaseNode<TValue>& Operand;
};

template<typename TValue, typename TResult, template<typename> typename TBaseNode>
struct TPostUnaryExpression : public TExpression<TResult, TBaseNode>
{
    const TBaseNode<TValue>& Operand;
};

template<typename TLhs, typename TRhs, typename TResult = decltype(std::declval<TLhs>() + std::declval<TRhs>())>
struct TAdd : public TBinaryExpression<TExpressionNode, TLhs, TRhs, TResult>
{
    using Super = TBinaryExpression<TExpressionNode, TLhs, TRhs, TResult>;
    using Super::Super;

    virtual TResult Eval() const
    {
        return this->Lhs.Eval() + this->Rhs.Eval();
    }
};

template<typename TLhs, typename TRhs, typename TResult = decltype(std::declval<TLhs>() - std::declval<TRhs>())>
struct TSubtract : public TBinaryExpression<TExpressionNode, TLhs, TRhs, TResult>
{
    using Super = TBinaryExpression<TExpressionNode, TLhs, TRhs, TResult>;
    using Super::Super;

    virtual TResult Eval() const
    {
        return this->Lhs.Eval() - this->Rhs.Eval();
    }
};


template<typename TLhs, typename TRhs, typename TResult = decltype(std::declval<TLhs>() * std::declval<TRhs>())>
struct TMultiply : public TBinaryExpression<TExpressionNode, TLhs, TRhs, TResult>
{
    using Super = TBinaryExpression<TExpressionNode, TLhs, TRhs, TResult>;
    using Super::Super;

    virtual TResult Eval() const
    {
        return this->Lhs.Eval() * this->Rhs.Eval();
    }
};

template<typename TLhs, typename TRhs, typename TResult = decltype(std::declval<TLhs>() / std::declval<TRhs>())>
struct TDivide : public TBinaryExpression<TExpressionNode, TLhs, TRhs, TResult>
{
    using Super = TBinaryExpression<TExpressionNode, TLhs, TRhs, TResult>;
    using Super::Super;

    virtual TResult Eval() const
    {
        return this->Lhs.Eval() / this->Rhs.Eval();
    }
};


template<typename T>
struct TOperatorTraits;

template<typename T>
struct TOperatorTraits<TExpressionNode<T>>
{
    template<typename Lhs, typename Rhs>
    using Add = TAdd<Lhs, Rhs>;

    template<typename Lhs, typename Rhs>
    using Subtract = TSubtract<Lhs, Rhs>;

    template<typename Lhs, typename Rhs>
    using Multiply = TMultiply<Lhs, Rhs>;

    template<typename Lhs, typename Rhs>
    using Divide = TDivide<Lhs, Rhs>;
};


template<typename TExprNode>
struct TExprNodeTraits
{
    using TBaseNode = typename TExprNode::BaseNodeType;
    
    using TOperators = TOperatorTraits<TBaseNode>;

    using TValue = typename TBaseNode::TValue;
};

template<
    typename TLeftNode,
    typename TRightNode
> 
auto operator+(const TLeftNode& Lhs, const TRightNode& Rhs)
{   
    using TLhs = TExprNodeTraits<TLeftNode>::TValue;
    using TRhs = TExprNodeTraits<TRightNode>::TValue;

    using AddExpr = typename TExprNodeTraits<TLeftNode>::TOperators::template Add<TLhs, TRhs>;
    using RightAddExpr = typename TExprNodeTraits<TRightNode>::TOperators::template Add<TLhs, TRhs>;

    static_assert(std::is_same_v<AddExpr, RightAddExpr>, "LeftNode And RightNode is Different Type");
    return AddExpr(Lhs, Rhs);
}

template<
    typename TLeftNode,
    typename TRightNode
>
auto operator-(const TLeftNode& Lhs, const TRightNode& Rhs)
{
    using TLhs = TExprNodeTraits<TLeftNode>::TValue;
    using TRhs = TExprNodeTraits<TRightNode>::TValue;

    using SubExpr = typename TExprNodeTraits<TLeftNode>::TOperators::template Subtract<TLhs, TRhs>;
    using RightSubExpr = typename TExprNodeTraits<TRightNode>::TOperators::template Subtract<TLhs, TRhs>;

    static_assert(std::is_same_v<SubExpr, RightSubExpr>, "LeftNode And RightNode is Different Type");
    return SubExpr(Lhs, Rhs);
}

template<
    typename TLeftNode,
    typename TRightNode
>
auto operator*(const TLeftNode& Lhs, const TRightNode& Rhs)
{
    using TLhs = TExprNodeTraits<TLeftNode>::TValue;
    using TRhs = TExprNodeTraits<TRightNode>::TValue;

    using MulExpr = typename TExprNodeTraits<TLeftNode>::TOperators::template Multiply<TLhs, TRhs>;
    using RightMulExpr = typename TExprNodeTraits<TRightNode>::TOperators::template Multiply<TLhs, TRhs>;

    static_assert(std::is_same_v<MulExpr, RightMulExpr>, "LeftNode And RightNode is Different Type");
    return MulExpr(Lhs, Rhs);
}

template<
    typename TLeftNode,
    typename TRightNode
>
auto operator/(const TLeftNode& Lhs, const TRightNode& Rhs)
{
    using TLhs = TExprNodeTraits<TLeftNode>::TValue;
    using TRhs = TExprNodeTraits<TRightNode>::TValue;

    using DivideExpr = typename TExprNodeTraits<TLeftNode>::TOperators::template Divide<TLhs, TRhs>;
    using RightDivideExpr = typename TExprNodeTraits<TRightNode>::TOperators::template Divide<TLhs, TRhs>;

    static_assert(std::is_same_v<DivideExpr, DivideExpr>, "LeftNode And RightNode is Different Type");
    return DivideExpr(Lhs, Rhs);
}