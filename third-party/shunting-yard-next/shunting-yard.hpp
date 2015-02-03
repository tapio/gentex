// Copyright 2011 - 2012, 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

#pragma once

#ifndef MAX_TOKENS
#define MAX_TOKENS 128
#endif

#ifndef OPERAND_STACK_SIZE
#define OPERAND_STACK_SIZE 64
#endif

#ifndef OPERATOR_STACK_SIZE
#define OPERATOR_STACK_SIZE 64
#endif

#ifndef FUNCTION_STACK_SIZE
#define FUNCTION_STACK_SIZE 64
#endif

namespace calc {

typedef enum {
    OK,
    ERROR_SYNTAX,
    ERROR_OPEN_PARENTHESIS,
    ERROR_CLOSE_PARENTHESIS,
    ERROR_UNRECOGNIZED,
    ERROR_NO_INPUT,
    ERROR_UNDEFINED_FUNCTION,
    ERROR_FUNCTION_ARGUMENTS,
    ERROR_UNDEFINED_CONSTANT
} Status;

// Calculates the result of a mathematical expression.
Status shunting_yard(const char *expression, double *result);

} // namespace
