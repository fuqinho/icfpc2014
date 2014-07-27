#pragma once
#include "common.h"
#include "ast.h"

std::vector<ast::AST> parse_program(std::istream& in);
