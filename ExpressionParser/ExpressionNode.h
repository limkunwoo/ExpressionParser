#pragma once
#include <iostream>


enum class EExprNodeType
{
    Expression,
    Operand,
};

struct FExpressionNodeBase
{
    virtual EExprNodeType GetNodeType() const = 0;
};

template<typename T>
struct TExpressionNode : FExpressionNodeBase
{
    using TValue = T;

    virtual T Eval() const = 0;
    virtual std::shared_ptr<TExpressionNode> Alloc() const = 0;
};

template<typename T, template<typename> typename TBaseNode = TExpressionNode>
struct TOperand : public TBaseNode<T>
{
    using BaseNodeType = TBaseNode<T>;
    using TValue = TBaseNode<T>::TValue;
protected:
    TValue Value;
public:
    template<typename... TConstructionVar>
    TOperand(TConstructionVar&&... InValue) : Value(std::forward<TConstructionVar>(InValue)...) {}
    
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
protected:
    const TBaseNode<TLhs>& Lhs;
    const TBaseNode<TRhs>& Rhs;

    std::shared_ptr<TBaseNode<TLhs>> LhsPtr;
    std::shared_ptr<TBaseNode<TRhs>> RhsPtr;
public:
    TBinaryExpression(const TBaseNode<TLhs>& InLhs, const TBaseNode<TRhs>& InRhs) : Lhs(InLhs), Rhs(InRhs) {}
    TBinaryExpression(std::shared_ptr<TBaseNode<TLhs>> InLhs, std::shared_ptr<TBaseNode<TRhs>> InRhs) : Lhs(*InLhs), Rhs(*InRhs), LhsPtr(InLhs), RhsPtr(InRhs) {}
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
    constexpr const static bool bAllowImplicitConversion = true;

    template<typename T>
    using Operand = TOperand<T, TExpressionNode>;

    template<typename Lhs, typename Rhs>
    using Add = TAdd<Lhs, Rhs>;

    template<typename Lhs, typename Rhs>
    using Subtract = TSubtract<Lhs, Rhs>;

    template<typename Lhs, typename Rhs>
    using Multiply = TMultiply<Lhs, Rhs>;

    template<typename Lhs, typename Rhs>
    using Divide = TDivide<Lhs, Rhs>;
};
template<typename T>
concept IsExprNode = (std::is_base_of_v<FExpressionNodeBase, T>);
//return LValue;
template<typename TOther, typename TIn> requires (IsExprNode<TIn>)
const TIn& ImpiclitConversionExprNode(const TIn& InValue)
{
    return InValue;
}

//return RValue;
template<typename TOther, typename TIn> requires (!IsExprNode<TIn>)
auto ImpiclitConversionExprNode(const TIn& InValue)
{
    static_assert(TOperatorTraits<typename TOther::BaseNodeType>::bAllowImplicitConversion, "Not Allowed ImplicitConversion");
    return TOperatorTraits<typename TOther::BaseNodeType>::template Operand<TIn>(InValue);
}

template<
    typename TLeft,
    typename TRight
>  requires 
(IsExprNode<TLeft> || IsExprNode<TRight>)
    &&
(requires() { TOperatorTraits<typename TLeft::BaseNodeType>::bAllowImplicitConversion; } || requires() { TOperatorTraits<typename TRight::BaseNodeType>::bAllowImplicitConversion; })
auto operator+(const TLeft& Lhs, const TRight& Rhs)
{   
    using TLeftNode = std::remove_cvref_t<decltype(ImpiclitConversionExprNode<TRight>(Lhs))>;
    using TRightNode = std::remove_cvref_t<decltype(ImpiclitConversionExprNode<TLeft>(Rhs))>;

    using TLhs = TLeftNode::TValue;
    using TRhs = TRightNode::TValue;

    using AddExpr = typename TOperatorTraits<typename TLeftNode::BaseNodeType>::template Add<TLhs, TRhs>;
    using RightAddExpr = typename TOperatorTraits<typename TRightNode::BaseNodeType>::template Add<TLhs, TRhs>;

    static_assert(std::is_same_v<AddExpr, RightAddExpr>, "LeftNode And RightNode is Different Type");
    return AddExpr(ImpiclitConversionExprNode<TRightNode>(Lhs), ImpiclitConversionExprNode<TRightNode>(Rhs));
}

template<
    typename TLeft,
    typename TRight
>  requires
(IsExprNode<TLeft> || IsExprNode<TRight>)
&&
(requires() { TOperatorTraits<typename TLeft::BaseNodeType>::bAllowImplicitConversion; } || requires() { TOperatorTraits<typename TRight::BaseNodeType>::bAllowImplicitConversion; })
auto operator-(const TLeft& Lhs, const TRight& Rhs)
{
    using TLeftNode = std::remove_cvref_t<decltype(ImpiclitConversionExprNode<TRight>(Lhs))>;
    using TRightNode = std::remove_cvref_t<decltype(ImpiclitConversionExprNode<TLeft>(Rhs))>;

    using TLhs = TLeftNode::TValue;
    using TRhs = TRightNode::TValue;

    using SubExpr = typename TOperatorTraits<typename TLeftNode::BaseNodeType>::template Subtract<TLhs, TRhs>;
    using RightSubExpr = typename TOperatorTraits<typename TRightNode::BaseNodeType>::template Subtract<TLhs, TRhs>;

    static_assert(std::is_same_v<SubExpr, RightSubExpr>, "LeftNode And RightNode is Different Type");
    return SubExpr(ImpiclitConversionExprNode<TRightNode>(Lhs), ImpiclitConversionExprNode<TRightNode>(Rhs));
}

template<
    typename TLeft,
    typename TRight
>  requires
(IsExprNode<TLeft> || IsExprNode<TRight>)
&&
(requires() { TOperatorTraits<typename TLeft::BaseNodeType>::bAllowImplicitConversion; } || requires() { TOperatorTraits<typename TRight::BaseNodeType>::bAllowImplicitConversion; })
auto operator*(const TLeft& Lhs, const TRight& Rhs)
{
    using TLeftNode = std::remove_cvref_t<decltype(ImpiclitConversionExprNode<TRight>(Lhs))>;
    using TRightNode = std::remove_cvref_t<decltype(ImpiclitConversionExprNode<TLeft>(Rhs))>;

    using TLhs = TLeftNode::TValue;
    using TRhs = TRightNode::TValue;

    using MulExpr = typename TOperatorTraits<typename TLeftNode::BaseNodeType>::template Multiply<TLhs, TRhs>;
    using RightMulExpr = typename TOperatorTraits<typename TRightNode::BaseNodeType>::template Multiply<TLhs, TRhs>;

    static_assert(std::is_same_v<MulExpr, RightMulExpr>, "LeftNode And RightNode is Different Type");
    return MulExpr(ImpiclitConversionExprNode<TRightNode>(Lhs), ImpiclitConversionExprNode<TRightNode>(Rhs));
}

template<
    typename TLeft,
    typename TRight
>  requires
(IsExprNode<TLeft> || IsExprNode<TRight>)
&&
(requires() { TOperatorTraits<typename TLeft::BaseNodeType>::bAllowImplicitConversion; } || requires() { TOperatorTraits<typename TRight::BaseNodeType>::bAllowImplicitConversion; })
auto operator/(const TLeft& Lhs, const TRight& Rhs)
{
    using TLeftNode = std::remove_cvref_t<decltype(ImpiclitConversionExprNode<TRight>(Lhs))>;
    using TRightNode = std::remove_cvref_t<decltype(ImpiclitConversionExprNode<TLeft>(Rhs))>;

    using TLhs = TLeftNode::TValue;
    using TRhs = TRightNode::TValue;

    using DivideExpr = typename TOperatorTraits<typename TLeftNode::BaseNodeType>::template Divide<TLhs, TRhs>;
    using RightDivideExpr = typename TOperatorTraits<typename TRightNode::BaseNodeType>::template Divide<TLhs, TRhs>;

    static_assert(std::is_same_v<DivideExpr, DivideExpr>, "LeftNode And RightNode is Different Type");
    return DivideExpr(ImpiclitConversionExprNode<TRightNode>(Lhs), ImpiclitConversionExprNode<TRightNode>(Rhs));
}

template<typename TExprNode>
struct TExpressionHolder
{
    const TExprNode& Expr;
    TExpressionHolder(const TExprNode& InExpr) : Expr(InExpr) {}
};