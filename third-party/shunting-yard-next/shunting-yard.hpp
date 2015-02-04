// Copyright 2011 - 2012, 2014 Brian Marshall. All rights reserved.
// Copyright 2015 Tapio Vierros.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

#pragma once

#include <string>
#include <vector>

namespace calc {

enum Status {
	OK,
	ERROR_SYNTAX,
	ERROR_OPEN_PARENTHESIS,
	ERROR_CLOSE_PARENTHESIS,
	ERROR_UNRECOGNIZED,
	ERROR_NO_INPUT,
	ERROR_UNDEFINED_FUNCTION,
	ERROR_FUNCTION_ARGUMENTS,
	ERROR_UNDEFINED_CONSTANT
};

// std::function is much slower...
typedef double (*MathFunc)(double);

struct Function {
	const char* name;
	MathFunc func;
};

struct Constant {
	const char* name;
	double value;
};

enum TokenType {
	TOKEN_NONE,
	TOKEN_UNKNOWN,
	TOKEN_OPEN_PARENTHESIS,
	TOKEN_CLOSE_PARENTHESIS,
	TOKEN_OPERATOR,
	TOKEN_NUMBER,
	TOKEN_IDENTIFIER
};

struct Token {
	TokenType type;
	MathFunc func;
	double num;
	char op;
	char var;
};

struct MathExpression
{
	static const int MAX_TOKENS = 128;

	MathExpression(const std::string& expr);
	void setVar(char var, double value);
	double eval(Status* status = 0);

	static std::vector<Function> funcs;
	static std::vector<Constant> consts;

private:
	Token tokens[MAX_TOKENS];
};

} // namespace
