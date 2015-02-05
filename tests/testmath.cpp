// This is adapted from Brian Marshall's shunting-yard test suite.
// Copyright 2012 - 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.

#include <string>
#include <iostream>
#include <chrono>
#include <cmath>

#include "shunting-yard-next/shunting-yard.hpp"
using namespace calc;

static int tests = 0;
static int fails = 0;

#define ASSERT_RESULT(expression, result) do { \
	tests++; \
	Status status; \
	MathExpression expr(expression); \
	double res = expr.eval(&status); \
	if (status != OK) { \
		std::cout << "Expression " << #expression << " failed with code " << status << std::endl; \
		fails++; \
	} else if (std::fabs(res - result) > 1e-6) { \
		std::cout << "Expression " << #expression << " failed: got " << res << ", expected " << result << std::endl; \
		fails++; \
	} \
} while(0);

int main(int, char*[]) {
	auto t0 = std::chrono::steady_clock::now();

	// Test addition
	ASSERT_RESULT("2+2", 4);
	ASSERT_RESULT("2  +  2", 4);
	ASSERT_RESULT("2+2.", 4);
	ASSERT_RESULT("3 + (5 + 1 + (2 + 2))", 13);
	ASSERT_RESULT("1+2+4+8+16 + 11", 42);
	ASSERT_RESULT("2.1+2.1", 4.2);

	// Test subtraction
	ASSERT_RESULT("8-4", 4);
	ASSERT_RESULT("15-10", 5);
	ASSERT_RESULT("27 - (10 - 11)", 28);
	ASSERT_RESULT("-5-11", -16);
	ASSERT_RESULT("-(2-3.6)", 1.6);
	ASSERT_RESULT("(-5-7)", -12);

	// Test multiplication
	ASSERT_RESULT("13 * 2", 26);
	ASSERT_RESULT("3.2*2", 6.4);
	ASSERT_RESULT("20*2*1.375", 55);
	ASSERT_RESULT("0.75*((2*-4)*1.5)", -9);
	ASSERT_RESULT("27*0.5", 13.5);
	ASSERT_RESULT("2(3)", 6);
	ASSERT_RESULT("(2)(3)", 6);

	// Test division
	ASSERT_RESULT("1/2", 0.5);
	ASSERT_RESULT("3.885 / 7", 0.555);
	ASSERT_RESULT("(140/2)/0.5/2", 70);
	ASSERT_RESULT("((517/4)/2/.25/.25)/22", 47);
	ASSERT_RESULT("2987898/34743", 86);

	// Test modulus
	ASSERT_RESULT("10 % 6", 4);
	ASSERT_RESULT("2+3 % 3", 2);
	ASSERT_RESULT("6*5%21", 9);
	ASSERT_RESULT("10%11", 10);
	ASSERT_RESULT("5 %5", 0);
	ASSERT_RESULT("5.7%3", 2.7);
	ASSERT_RESULT("pi%2", 1.1415926535898);

	// Test exponentiation
	ASSERT_RESULT("3^2", 9);
	ASSERT_RESULT("10^-2", 0.01);
	ASSERT_RESULT("4^2", 16);
	ASSERT_RESULT("2^8", 256);
	ASSERT_RESULT("5^(2^3)", 390625);

	// Test factorials
	ASSERT_RESULT("1!", 1);
	ASSERT_RESULT("2!", 2);
	ASSERT_RESULT("3!", 6);
	ASSERT_RESULT("4!", 24);
	ASSERT_RESULT("5!", 120);
	ASSERT_RESULT("3!+1", 7);

	// Test comparison
	ASSERT_RESULT("1 < 2", 1);
	ASSERT_RESULT("1 > 2", 0);
	ASSERT_RESULT("2.5 < 1.6", 0);
	ASSERT_RESULT("2.5 > 1.6", 1);
	ASSERT_RESULT("2 * 1 < 1.5", 0);
	ASSERT_RESULT("2 * (1 < 1.5)", 2);

	// Test functions
	ASSERT_RESULT("abs(-32)", 32);
	ASSERT_RESULT("abs(-5-7)", 12);
	ASSERT_RESULT("abs(-1.1)", 1.1);
	ASSERT_RESULT("sqrt(100)", 10);
	ASSERT_RESULT("sqrt(sqrt(10000))", 10);
	ASSERT_RESULT("sqrt(sqrt(10000) + 800)", 30);
	ASSERT_RESULT("42 * cos(0)", 42);
	ASSERT_RESULT("(sin(0)*cos(0)*40*tan(0))-1", -1);
	ASSERT_RESULT("log(10)", 1);
	ASSERT_RESULT("lb(8)", 3);
	ASSERT_RESULT("ln(e)", 1);
	ASSERT_RESULT("log(10^42)", 42);
	ASSERT_RESULT("lb(2^123)", 123);

	// Test constant
	ASSERT_RESULT("sin(pi)", 0);
	ASSERT_RESULT("cos(pi)", -1);
	ASSERT_RESULT("tan(pi)", 0);
	ASSERT_RESULT("cos(tau)", 1);
	ASSERT_RESULT("cos(2pi)", 1);
	ASSERT_RESULT("((2pi/tau)+(10pi))/(1+10pi)", 1);
	ASSERT_RESULT("2pi", 6.2831853071796);
	ASSERT_RESULT("pi(2)", 6.2831853071796);
	ASSERT_RESULT("pi pi", 9.8696044010894);
	ASSERT_RESULT("2pi pi", 19.739208802179);

	// Precedence
	ASSERT_RESULT("6/3*5", 10);
	ASSERT_RESULT("6+3*2", 12);
	ASSERT_RESULT("-10^2", -100);
	ASSERT_RESULT("(-10)^2", 100);
	ASSERT_RESULT("10^2+1", 101);
	ASSERT_RESULT("10^-2+1", 1.01);
	ASSERT_RESULT("-10^-2+1", 0.99);
	ASSERT_RESULT("10^-2*2", 0.02);
	ASSERT_RESULT("2+6/2*5+10/3-2/6", 20);
	ASSERT_RESULT("10^3!", 1000000);
	ASSERT_RESULT("10^-2pi", M_PI / 100);
	ASSERT_RESULT("2^2^3", 256);

	auto t1 = std::chrono::steady_clock::now();
	auto dtms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

	std::cout << (tests - fails) << "/" << tests << " tests succeeded, " << fails << " failed";
	std::cout << std::endl;
	std::cout << "tests took " << dtms << " ms" << std::endl;

	return fails;
}
