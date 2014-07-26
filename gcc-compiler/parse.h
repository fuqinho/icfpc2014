#pragma once
#include <istream>
#include <vector>
#include "ast.h"

std::vector<ast::AST> parse_program(std::istream& in);
