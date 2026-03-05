#include "calc.h"
#include "vga.h"
#include "string.h"

/* --- Simple Inline x87 Math Functions --- */
static double m_sin(double x) {
	double res;
	__asm__ volatile ("fsin" : "=t" (res) : "0" (x));
	return res;
}

static double m_cos(double x) {
	double res;
	__asm__ volatile ("fcos" : "=t" (res) : "0" (x));
	return res;
}

static double m_tan(double x) {
	double res;
	/* fptan pushes 1.0 onto the stack after the result, so we pop it */
	__asm__ volatile ("fptan; fstp %%st(0)" : "=t" (res) : "0" (x));
	return res;
}

static double m_sqrt(double x) {
	double res;
	__asm__ volatile ("fsqrt" : "=t" (res) : "0" (x));
	return res;
}

static double m_log(double x) {
	double res;
	/* fyl2x computes y * log2(x). We want ln(x), so y = ln(2) */
	__asm__ volatile ("fldln2; fxch; fyl2x" : "=t" (res) : "0" (x));
	return res;
}

static double m_pow(double base, double exp) {
	double res;
	/* pow(base, exp) = 2^(exp * log2(base)) */
	__asm__ volatile (
		"fyl2x;"
		"fld %%st(0);"
		"frndint;"
		"fsubr %%st, %%st(1);"
		"fxch;"
		"f2xm1;"
		"fld1;"
		"faddp;"
		"fscale;"
		"fstp %%st(1);"
		: "=t" (res) : "0" (base), "u" (exp)
	);
	return res;
}

/* --- Recursive Descent Parser --- */
static const char *parse_ptr;

static double parse_expression(void);

static void skip_whitespace(void) {
	while (*parse_ptr == ' ' || *parse_ptr == '\t') {
		parse_ptr++;
	}
}

static double parse_factor(void) {
	skip_whitespace();

	double result = 0.0;
	int sign = 1;

	/* Unary minus/plus */
	if (*parse_ptr == '-') {
		sign = -1;
		parse_ptr++;
	} else if (*parse_ptr == '+') {
		parse_ptr++;
	}

	skip_whitespace();

	/* Parentheses */
	if (*parse_ptr == '(') {
		parse_ptr++;
		result = parse_expression();
		skip_whitespace();
		if (*parse_ptr == ')') parse_ptr++;
		return sign * result;
	}

	/* Functions */
	if (strncmp(parse_ptr, "sin", 3) == 0) {
		parse_ptr += 3; result = m_sin(parse_factor()); return sign * result;
	} else if (strncmp(parse_ptr, "cos", 3) == 0) {
		parse_ptr += 3; result = m_cos(parse_factor()); return sign * result;
	} else if (strncmp(parse_ptr, "tan", 3) == 0) {
		parse_ptr += 3; result = m_tan(parse_factor()); return sign * result;
	} else if (strncmp(parse_ptr, "sqrt", 4) == 0) {
		parse_ptr += 4; result = m_sqrt(parse_factor()); return sign * result;
	} else if (strncmp(parse_ptr, "log", 3) == 0) {
		parse_ptr += 3; result = m_log(parse_factor()); return sign * result;
	}

	/* Numbers */
	double fraction = 0.0;
	double divisor = 10.0;
	
	while (*parse_ptr >= '0' && *parse_ptr <= '9') {
		result = result * 10.0 + (*parse_ptr - '0');
		parse_ptr++;
	}

	if (*parse_ptr == '.') {
		parse_ptr++;
		while (*parse_ptr >= '0' && *parse_ptr <= '9') {
			fraction += (*parse_ptr - '0') / divisor;
			divisor *= 10.0;
			parse_ptr++;
		}
	}
	
	result += fraction;
	return sign * result;
}

static double parse_power(void) {
	double result = parse_factor();
	skip_whitespace();
	while (*parse_ptr == '^') {
		parse_ptr++;
		result = m_pow(result, parse_factor());
		skip_whitespace();
	}
	return result;
}

static double parse_term(void) {
	double result = parse_power();
	skip_whitespace();

	while (*parse_ptr == '*' || *parse_ptr == '/') {
		char op = *parse_ptr++;
		double rhs = parse_power();
		if (op == '*') result *= rhs;
		else if (op == '/') {
			if (rhs != 0.0) result /= rhs;
			else { vga_print_colored("Error: Division by zero\n", VGA_LIGHT_RED, VGA_BLACK); return 0; }
		}
		skip_whitespace();
	}
	return result;
}

static double parse_expression(void) {
	double result = parse_term();
	skip_whitespace();

	while (*parse_ptr == '+' || *parse_ptr == '-') {
		char op = *parse_ptr++;
		double rhs = parse_term();
		if (op == '+') result += rhs;
		else if (op == '-') result -= rhs;
		skip_whitespace();
	}
	return result;
}

void cmd_calc(const char *expr) {
	if (!expr || !*expr) {
		vga_print_line("Usage: calc <expression>");
		return;
	}

	/* Initialize FPU before arbitrary math ops */
	__asm__ volatile ("finit");

	parse_ptr = expr;
	double result = parse_expression();
	
	char buf[64];
	ftoa(result, buf, 6); /* 6 decimal places */
	
	vga_print_colored("Result: ", VGA_LIGHT_GREEN, VGA_BLACK);
	vga_print_line(buf);
}
