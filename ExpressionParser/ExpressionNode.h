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
    virtual std::shared_ptr<TExpressionNode> Alloc() const { return std::shared_ptr<TExpressionNode>(); }
};

template<typename T, template<typename> typename TBaseNode = TExpressionNode>
struct TOperand : public TBaseNode<T>
{
    using BaseNodeType = TBaseNode<T>;
    
    template<typename T>
    using TTBaseNode = TBaseNode<T>;

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
    template<typename T>
    using TTBaseNode = TBaseNode<T>;

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
    constexpr const static bool bUseCustomOperator = false;

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

template<typename TLeft, typename TRight>
struct TOperatorImplHelper
{
    using TLeftNode = typename std::remove_cvref_t<decltype(ImpiclitConversionExprNode<TRight>(std::declval<TLeft>()))>;
    using TRightNode = typename std::remove_cvref_t<decltype(ImpiclitConversionExprNode<TLeft>(std::declval<TRight>()))>;

    using TLeftBaseNode = typename TLeftNode::BaseNodeType;
    using TRightBaseNode = typename TRightNode::BaseNodeType;

    constexpr static bool bIsSameNodeBase = std::is_same_v<typename TLeftNode::template TTBaseNode<TLeft>, typename TRightNode::template TTBaseNode<TLeft>>;
    static_assert(bIsSameNodeBase, "LeftNode And RightNode is Different Type");

    using TLhs = typename TLeftNode::TValue;
    using TRhs = typename TRightNode::TValue;

    using TLeftOperators = typename TOperatorTraits<typename TLeftNode::BaseNodeType>;
    using TRightOperators = typename TOperatorTraits<typename TRightNode::BaseNodeType>;

    constexpr static bool bAllowImplicitConversion = TLeftOperators::bAllowImplicitConversion && TRightOperators::bAllowImplicitConversion;

    constexpr static bool bIsBothExprNode = (IsExprNode<TLeft> && IsExprNode<TRight>);
    constexpr static bool bIsExclusiveOrExprNode = (IsExprNode<TLeft> && !IsExprNode<TRight>) || (!IsExprNode<TLeft> && IsExprNode<TRight>);;
    
    constexpr static bool bSouldAutoDefinedOperator = (bIsExclusiveOrExprNode ? bAllowImplicitConversion : bIsBothExprNode) 
        && 
        (!TLeftOperators::bUseCustomOperator && !TRightOperators::bUseCustomOperator);
};

template<
    typename TLeft,
    typename TRight
>  requires (TOperatorImplHelper<TLeft, TRight>::bSouldAutoDefinedOperator)
auto operator+(const TLeft& Lhs, const TRight& Rhs)
{ 
    using TImplHelper = TOperatorImplHelper<TLeft, TRight>;
    using Add = TImplHelper::TLeftOperators::template Add<typename TImplHelper::TLhs, typename TImplHelper::TRhs>;
    return Add(ImpiclitConversionExprNode<typename TImplHelper::TRightNode>(Lhs), ImpiclitConversionExprNode<typename TImplHelper::TLeftNode>(Rhs));
}

template<
    typename TLeft,
    typename TRight
>  requires (TOperatorImplHelper<TLeft, TRight>::bSouldAutoDefinedOperator)
auto operator-(const TLeft& Lhs, const TRight& Rhs)
{
    using TImplHelper = TOperatorImplHelper<TLeft, TRight>;

    using Subtract = TImplHelper::TLeftOperators::template Subtract<typename TImplHelper::TLhs, typename TImplHelper::TRhs>;
    return Subtract(ImpiclitConversionExprNode<typename TImplHelper::TRightNode>(Lhs), ImpiclitConversionExprNode<typename TImplHelper::TLeftNode>(Rhs));
}

template<
    typename TLeft,
    typename TRight
>  requires (TOperatorImplHelper<TLeft, TRight>::bSouldAutoDefinedOperator)
auto operator*(const TLeft& Lhs, const TRight& Rhs)
{
    using TImplHelper = TOperatorImplHelper<TLeft, TRight>;

    using Multiply = TImplHelper::TLeftOperators::template Multiply<typename TImplHelper::TLhs, typename TImplHelper::TRhs>;
    return Multiply(ImpiclitConversionExprNode<typename TImplHelper::TRightNode>(Lhs), ImpiclitConversionExprNode<typename TImplHelper::TLeftNode>(Rhs));
}

template<
    typename TLeft,
    typename TRight
>  requires (TOperatorImplHelper<TLeft, TRight>::bSouldAutoDefinedOperator)
auto operator/(const TLeft& Lhs, const TRight& Rhs)
{
    using TImplHelper = TOperatorImplHelper<TLeft, TRight>;

    using Divide = TImplHelper::TLeftOperators::template Divide<typename TImplHelper::TLhs, typename TImplHelper::TRhs>;
    return Divide(ImpiclitConversionExprNode<typename TImplHelper::TRightNode>(Lhs), ImpiclitConversionExprNode<typename TImplHelper::TLeftNode>(Rhs));
}

template<typename TExprNode>
struct TExpressionHolder
{
    const TExprNode& Expr;
    TExpressionHolder(const TExprNode& InExpr) : Expr(InExpr) {}
};