#pragma once

#include <iostream>
#include "ExpressionNode.h"

template<typename T>
struct TLogExpressionNode : public TExpressionNode<T>//, public TOperatorImpl<TLogExpressionNode>
{
	virtual void Log() const = 0;
};


template<typename T>
struct TLogOperand : public TOperand<T, TLogExpressionNode>
{
	using Super = TOperand<T, TLogExpressionNode>;
	using Super::Super;

	virtual void Log() const
	{
		std::cout << this->Value;
	}
	virtual std::shared_ptr<TExpressionNode<T>> Alloc() const override
	{
		return std::make_shared<TLogOperand<T>>(this->Value);
	}
	template<typename TRhs>
	TLogOperand<T>& operator=(const TLogExpressionNode<TRhs>& Rhs)
	{
		T Value = Rhs.Eval();
		std::cout << Value;
		std::cout << " = ";
		Rhs.Log();
		std::cout << std::endl;

		this->Value = Rhs.Eval();
		return *this;
	}
};

template<typename TLhs, typename TRhs, typename TResult = decltype(std::declval<TLhs>() + std::declval<TRhs>())>
struct TLogBinaryOperation : public TBinaryExpression<TLogExpressionNode, TLhs, TRhs, TResult>
{
	using Super = TBinaryExpression<TLogExpressionNode, TLhs, TRhs, TResult>;
	using Super::Super;

	virtual void LogOp() const = 0;
	virtual void Log() const
	{
		if (this->Lhs.GetNodeType() == EExprNodeType::Expression)
		{
			std::cout << "(";
		}
		this->Lhs.Log();
		if (this->Lhs.GetNodeType() == EExprNodeType::Expression)
		{
			std::cout << ")";
		}

		LogOp();

		if (this->Rhs.GetNodeType() == EExprNodeType::Expression)
		{
			std::cout << "(";
		}
		this->Rhs.Log();
		if (this->Rhs.GetNodeType() == EExprNodeType::Expression)
		{
			std::cout << ")";
		}
	}
};

template<typename TLhs, typename TRhs, typename TResult = decltype(std::declval<TLhs>() + std::declval<TRhs>())>
struct TLogAdd : public TLogBinaryOperation<TLhs, TRhs>
{
	using Super = TLogBinaryOperation<TLhs, TRhs>;
	using Super::Super;

	virtual void LogOp() const
	{
		std::cout << " + ";
	}
	virtual TResult Eval() const
	{
		return this->Lhs.Eval() + this->Rhs.Eval();
	}
	virtual std::shared_ptr<TExpressionNode<TResult>> Alloc() const override
	{
		std::shared_ptr<TLogExpressionNode<TLhs>> LhsPtr = std::static_pointer_cast<TLogExpressionNode<TLhs>>(this->Lhs.Alloc());
		std::shared_ptr<TLogExpressionNode<TRhs>> RhsPtr = std::static_pointer_cast<TLogExpressionNode<TRhs>>(this->Rhs.Alloc());
		return std::make_shared<TLogAdd>(LhsPtr, RhsPtr);
	}
};

template<typename TLhs, typename TRhs, typename TResult = decltype(std::declval<TLhs>() + std::declval<TRhs>())>
struct TLogSubtract : public TLogBinaryOperation<TLhs, TRhs>
{
	using Super = TLogBinaryOperation<TLhs, TRhs>;
	using Super::Super;

	virtual void LogOp() const
	{
		std::cout << " - ";
	}
	virtual TResult Eval() const
	{
		return this->Lhs.Eval() - this->Rhs.Eval();
	}
	virtual std::shared_ptr<TExpressionNode<TResult>> Alloc() const override
	{
		std::shared_ptr<TLogExpressionNode<TLhs>> LhsPtr = std::static_pointer_cast<TLogExpressionNode<TLhs>>(this->Lhs.Alloc());
		std::shared_ptr<TLogExpressionNode<TRhs>> RhsPtr = std::static_pointer_cast<TLogExpressionNode<TRhs>>(this->Rhs.Alloc());
		return std::make_shared<TLogSubtract>(LhsPtr, RhsPtr);
	}
};

template<typename TLhs, typename TRhs, typename TResult = decltype(std::declval<TLhs>() + std::declval<TRhs>())>
struct TLogMultiply : public TLogBinaryOperation<TLhs, TRhs>
{
	using Super = TLogBinaryOperation<TLhs, TRhs>;
	using Super::Super;

	virtual void LogOp() const
	{
		std::cout << " * ";
	}
	virtual TResult Eval() const
	{
		return this->Lhs.Eval() * this->Rhs.Eval();
	}

	virtual std::shared_ptr<TExpressionNode<TResult>> Alloc() const override
	{
		std::shared_ptr<TLogExpressionNode<TLhs>> LhsPtr = std::static_pointer_cast<TLogExpressionNode<TLhs>>(this->Lhs.Alloc());
		std::shared_ptr<TLogExpressionNode<TRhs>> RhsPtr = std::static_pointer_cast<TLogExpressionNode<TRhs>>(this->Rhs.Alloc());
		return std::make_shared<TLogMultiply>(LhsPtr, RhsPtr);
	}
};

template<typename TLhs, typename TRhs, typename TResult = decltype(std::declval<TLhs>() + std::declval<TRhs>())>
struct TLogDivide : public TLogBinaryOperation<TLhs, TRhs>
{
	using Super = TLogBinaryOperation<TLhs, TRhs>;
	using Super::Super;

	virtual void LogOp() const
	{
		std::cout << " / ";
	}
	virtual TResult Eval() const
	{
		return this->Lhs.Eval() / this->Rhs.Eval();
	}

	virtual std::shared_ptr<TExpressionNode<TResult>> Alloc() const override
	{
		std::shared_ptr<TLogExpressionNode<TLhs>> LhsPtr = std::static_pointer_cast<TLogExpressionNode<TLhs>>(this->Lhs.Alloc());
		std::shared_ptr<TLogExpressionNode<TRhs>> RhsPtr = std::static_pointer_cast<TLogExpressionNode<TRhs>>(this->Rhs.Alloc());
		return std::make_shared<TLogDivide>(LhsPtr, RhsPtr);
	}
};

template<typename T>
struct TOperatorTraits<TLogExpressionNode<T>>
{
	constexpr const static bool bAllowImplicitConversion = false;
	constexpr const static bool bUseCustomOperator = false;

	template<typename T>
	using Operand = TLogOperand<T>;

	template<typename Lhs, typename Rhs>
	using Add = TLogAdd<Lhs, Rhs>;

	template<typename Lhs, typename Rhs>
	using Subtract = TLogSubtract<Lhs, Rhs>;

	template<typename Lhs, typename Rhs>
	using Multiply = TLogMultiply<Lhs, Rhs>;

	template<typename Lhs, typename Rhs>
	using Divide = TLogDivide<Lhs, Rhs>;
};

