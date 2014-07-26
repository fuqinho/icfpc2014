#pragma once
#include <istream>
#include <vector>
#include "ast.h"

ast::AST parse_expression(std::istream& in);
std::vector<ast::AST> parse_program(std::istream& in);
