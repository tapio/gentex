// Copyright 2011 - 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

#include "shunting-yard.hpp"

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

namespace calc {

typedef enum {
	TOKEN_NONE,
	TOKEN_UNKNOWN,
	TOKEN_OPEN_PARENTHESIS,
	TOKEN_CLOSE_PARENTHESIS,
	TOKEN_OPERATOR,
	TOKEN_NUMBER,
	TOKEN_IDENTIFIER
} TokenType;

typedef struct {
	TokenType type;
	double num;
	char *value;
	char op;
} Token;

typedef enum {
	OPERATOR_OTHER,
	OPERATOR_UNARY,
	OPERATOR_BINARY
} OperatorArity;

typedef enum {
	OPERATOR_NONE,
	OPERATOR_LEFT,
	OPERATOR_RIGHT
} OperatorAssociativity;

typedef struct {
	char symbol;
	int precedence;
	OperatorArity arity;
	OperatorAssociativity associativity;
} Operator;

typedef struct {
	int size;
	double values[OPERAND_STACK_SIZE];
} OperandStack;

typedef struct {
	int size;
	const Operator* values[OPERATOR_STACK_SIZE];
} OperatorStack;

typedef struct {
	int size;
	const char* values[FUNCTION_STACK_SIZE];
} FunctionStack;

#define STACK_PUSH(stack, value) (stack)->values[(stack)->size++] = (value)
#define STACK_POP(stack) (stack)->values[--(stack)->size]
#define STACK_TOP(stack) (stack)->values[(stack)->size-1]

static const Token NO_TOKEN = {TOKEN_NONE, 0, NULL, 0};

static const Operator OPERATORS[] = {
	{'!', 1, OPERATOR_UNARY,  OPERATOR_LEFT},
	{'^', 2, OPERATOR_BINARY, OPERATOR_RIGHT},
	{'+', 3, OPERATOR_UNARY,  OPERATOR_RIGHT},
	{'-', 3, OPERATOR_UNARY,  OPERATOR_RIGHT},
	{'*', 4, OPERATOR_BINARY, OPERATOR_LEFT},
	{'/', 4, OPERATOR_BINARY, OPERATOR_LEFT},
	{'%', 4, OPERATOR_BINARY, OPERATOR_LEFT},
	{'+', 5, OPERATOR_BINARY, OPERATOR_LEFT},
	{'-', 5, OPERATOR_BINARY, OPERATOR_LEFT},
	{'(', 6, OPERATOR_OTHER,  OPERATOR_NONE}
};

// Parses a tokenized expression.
static Status eval(const Token *tokens, OperandStack *operands, OperatorStack *operators, FunctionStack *functions);

// Pushes an operator to the stack after applying operators with a higher
// precedence.
static Status push_operator(const Operator *op, OperandStack *operands, OperatorStack *operators);

// Pushes the multiplication operator to the stack.
static Status push_multiplication(OperandStack *operands, OperatorStack *operators);

// Converts a constant identifier into its value and pushes it to the stack.
static Status push_constant(const char *value, OperandStack *operands);

// Applies an operator to the top one or two operands, depending on if the
// operator is unary or binary.
static Status apply_operator(const Operator *op, OperandStack *operands);

// Applies a unary operator to the top operand.
static Status apply_unary_operator(const Operator *op, OperandStack *operands);

// Applies a function to the top operand.
static Status apply_function(const char *function, OperandStack *operands);

// Returns the arity of an operator, using the previous token for context.
static OperatorArity get_arity(char symbol, const Token *previous);

// Returns a matching operator.
static const Operator *get_operator(char symbol, OperatorArity arity);

Status shunting_yard(const char *expression, double *result) {
	Token tokens[MAX_TOKENS];
	shunting_yard_parse(expression, tokens);
	return shunting_yard_eval(tokens, result);
}

void shunting_yard_parse(const char *expression, void *mem) {
	Token* tokens = (Token*)mem;
	int length = 0;
	const char *c = expression;
	while (*c) {
		Token token = {TOKEN_UNKNOWN, 0, NULL, 0};
		size_t tokenLength = 1;
		if (*c == '(')
			token.type = TOKEN_OPEN_PARENTHESIS;
		else if (*c == ')')
			token.type = TOKEN_CLOSE_PARENTHESIS;
		else if (strchr("!^*/%+-", *c)) {
			token.type = TOKEN_OPERATOR;
			token.op = *c;
		} else if (isdigit(*c) || *c == '.') {
			token.type = TOKEN_NUMBER;
			token.num = std::stod(c, &tokenLength);
		} else if (sscanf(c, "%m[A-Za-z]", &token.value)) {
			token.type = TOKEN_IDENTIFIER;
			tokenLength = strlen(token.value);
		}
		if (!isspace(*c)) {
			tokens[length++] = token;
		}
		c += tokenLength;
	}
	tokens[length] = NO_TOKEN;
}

Status shunting_yard_eval(void *mem, double *result) {
	Token* tokens = (Token*)mem;
	OperandStack operands; operands.size = 0;
	OperatorStack operators; operators.size = 0;
	FunctionStack functions; functions.size = 0;
	Status status = eval(tokens, &operands, &operators, &functions);
	if (operands.size)
		*result = round(STACK_POP(&operands) * 10e14) / 10e14;
	else if (status == OK)
		status = ERROR_NO_INPUT;
	//for (Token *token = tokens; token->type != TOKEN_NONE; token++)
	//	free(token->value);
	return status;
}

Status eval(const Token *tokens, OperandStack *operands, OperatorStack *operators, FunctionStack *functions) {
	Status status = OK;
	for (const Token *token = tokens, *previous = &NO_TOKEN, *next = token + 1;
		 token->type != TOKEN_NONE; previous = token, token = next++) {
		switch (token->type) {
			case TOKEN_OPEN_PARENTHESIS:
				// Implicit multiplication: "(2)(2)".
				if (previous->type == TOKEN_CLOSE_PARENTHESIS)
					status = push_multiplication(operands, operators);

				STACK_PUSH(operators, get_operator('(', OPERATOR_OTHER));
				break;

			case TOKEN_CLOSE_PARENTHESIS: {
				// Apply operators until the previous open parenthesis is found.
				bool found_parenthesis = false;
				while (operators->size && status == OK && !found_parenthesis) {
					const Operator *op = STACK_POP(operators);
					if (op->symbol == '(')
						found_parenthesis = true;
					else
						status = apply_operator(op, operands);
				}
				if (!found_parenthesis)
					status = ERROR_CLOSE_PARENTHESIS;
				else if (functions->size)
					status = apply_function(STACK_POP(functions), operands);
				break;
			}

			case TOKEN_OPERATOR:
				status = push_operator(get_operator(token->op, get_arity(token->op, previous)), operands, operators);
				break;

			case TOKEN_NUMBER:
				if (previous->type == TOKEN_CLOSE_PARENTHESIS ||
						previous->type == TOKEN_NUMBER ||
						previous->type == TOKEN_IDENTIFIER)
					status = ERROR_SYNTAX;
				else {
					STACK_PUSH(operands, token->num);

					// Implicit multiplication: "2(2)" or "2a".
					if (next->type == TOKEN_OPEN_PARENTHESIS || next->type == TOKEN_IDENTIFIER)
						status = push_multiplication(operands, operators);
				}
				break;

			case TOKEN_IDENTIFIER:
				// The identifier could be either a constant or function.
				status = push_constant(token->value, operands);
				if (status == ERROR_UNDEFINED_CONSTANT && next->type == TOKEN_OPEN_PARENTHESIS) {
					STACK_PUSH(functions, token->value);
					status = OK;
				} else if (next->type == TOKEN_OPEN_PARENTHESIS ||
						   next->type == TOKEN_IDENTIFIER) {
					// Implicit multiplication: "a(2)" or "a b".
					status = push_multiplication(operands, operators);
				}
				break;

			default:
				status = ERROR_UNRECOGNIZED;
		}
		if (status != OK)
			return status;
	}

	// Apply all remaining operators.
	while (operators->size && status == OK) {
		const Operator *op = STACK_POP(operators);
		if (op->symbol == '(')
			status = ERROR_OPEN_PARENTHESIS;
		else
			status = apply_operator(op, operands);
	}
	return status;
}

Status push_operator(const Operator *op, OperandStack *operands, OperatorStack *operators) {
	if (!op)
		return ERROR_SYNTAX;

	Status status = OK;
	while (operators->size && status == OK) {
		const Operator *stack_op = STACK_TOP(operators);
		if (op->arity == OPERATOR_UNARY ||
				op->precedence < stack_op->precedence ||
				(op->associativity == OPERATOR_RIGHT &&
				 op->precedence == stack_op->precedence))
			break;

		status = apply_operator(STACK_POP(operators), operands);
	}
	STACK_PUSH(operators, op);
	return status;
}

Status push_multiplication(OperandStack *operands, OperatorStack *operators) {
	return push_operator(get_operator('*', OPERATOR_BINARY), operands,
						 operators);
}

Status push_constant(const char *value, OperandStack *operands) {
	double x = 0.0;
	if (strcasecmp(value, "e") == 0)
		x = M_E;
	else if (strcasecmp(value, "pi") == 0)
		x = M_PI;
	else if (strcasecmp(value, "tau") == 0)
		x = M_PI * 2;
	else
		return ERROR_UNDEFINED_CONSTANT;
	STACK_PUSH(operands, x);
	return OK;
}

Status apply_operator(const Operator *op, OperandStack *operands) {
	if (!op || !operands->size)
		return ERROR_SYNTAX;
	if (op->arity == OPERATOR_UNARY)
		return apply_unary_operator(op, operands);

	double y = STACK_POP(operands);
	if (!operands->size)
		return ERROR_SYNTAX;
	double x = STACK_POP(operands);
	Status status = OK;
	switch (op->symbol) {
		case '^':
			x = pow(x, y);
			break;
		case '*':
			x = x * y;
			break;
		case '/':
			x = x / y;
			break;
		case '%':
			x = fmod(x, y);
			break;
		case '+':
			x = x + y;
			break;
		case '-':
			x = x - y;
			break;
		default:
			return ERROR_UNRECOGNIZED;
	}
	STACK_PUSH(operands, x);
	return status;
}

Status apply_unary_operator(const Operator *op, OperandStack *operands) {
	double x = STACK_POP(operands);
	switch (op->symbol) {
		case '+':
			break;
		case '-':
			x = -x;
			break;
		case '!':
			x = tgamma(x + 1);
			break;
		default:
			return ERROR_UNRECOGNIZED;
	}
	STACK_PUSH(operands, x);
	return OK;
}

Status apply_function(const char *function, OperandStack *operands) {
	if (!operands->size)
		return ERROR_FUNCTION_ARGUMENTS;

	double x = STACK_POP(operands);
	if (strcasecmp(function, "abs") == 0)
		x = fabs(x);
	else if (strcasecmp(function, "sqrt") == 0)
		x = sqrt(x);
	else if (strcasecmp(function, "ln") == 0)
		x = log(x);
	else if (strcasecmp(function, "lb") == 0)
		x = log2(x);
	else if (strcasecmp(function, "lg") == 0 ||
			 strcasecmp(function, "log") == 0)
		x = log10(x);
	else if (strcasecmp(function, "cos") == 0)
		x = cos(x);
	else if (strcasecmp(function, "sin") == 0)
		x = sin(x);
	else if (strcasecmp(function, "tan") == 0)
		x = tan(x);
	else
		return ERROR_UNDEFINED_FUNCTION;
	STACK_PUSH(operands, x);
	return OK;
}

OperatorArity get_arity(char symbol, const Token *previous) {
	if (symbol == '!' || previous->type == TOKEN_NONE ||
			previous->type == TOKEN_OPEN_PARENTHESIS ||
			(previous->type == TOKEN_OPERATOR && *previous->value != '!'))
		return OPERATOR_UNARY;
	return OPERATOR_BINARY;
}

const Operator *get_operator(char symbol, OperatorArity arity) {
	for (size_t i = 0; i < sizeof OPERATORS / sizeof OPERATORS[0]; i++) {
		if (OPERATORS[i].symbol == symbol && OPERATORS[i].arity == arity)
			return &OPERATORS[i];
	}
	return NULL;
}

} // namespace
