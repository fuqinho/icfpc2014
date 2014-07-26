#pragma once
#include <istream>
#include "ast.h"

ast::AST parse_expression(const char* p);
ast::AST parse_expression(std::istream& in);
