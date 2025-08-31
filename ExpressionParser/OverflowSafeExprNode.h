#pragma once
#include "ExpressionNode.h"

template<typename T>
struct TOverflowSafeExprNode : public TExpressionNode<T>
{
	virtual bool CheckOverflow() const = 0;
	virtual T GetValue() const = 0;

	virtual T Eval() const override final
	{
		CheckOverflow();
		return GetValue();
	}
	
	template<typename TOther> requires (std::is_arithmetic_v<T>)
	operator TOther()
	{
		return this->Eval();
	}
};

template<typename T>
struct TOverflowSafeOperand : public TOverflowSafeExprNode<T>
{
	using Super = TOverflowSafeExprNode<T>;
	using Super::Super;

	virtual bool CheckOverflow() const override
	{
		return true;
	}
	virtual EExprNodeType GetNodeType() const final
	{
		return EExprNodeType::Operand;
	}
	virtual T GetValue() const override
	{
		return this->Value;
	}
};

template<typename TLhs, typename TRhs, typename TResult = decltype(std::declval<TLhs>() + std::declval<TRhs>())>
struct TOverflowSafeAdd : public TBinaryExpression<TOverflowSafeExprNode, TLhs, TRhs, TResult>
{

};
