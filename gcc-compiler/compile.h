#pragma once
#include "ast.h"
#include "gcc.h"
#include <ostream>

struct PreLink
{
	gcc::OperationSequence              main_expression;
	std::vector<gcc::OperationSequence> sub_blocks;

	friend std::ostream& operator<<(std::ostream& os, const PreLink& me) {
		for(auto& op: me.main_expression)
			os << *op << std::endl;
		for(int i=0; i<me.sub_blocks.size(); ++i) {
			os << i << ":" << std::endl;
			for(auto& op: me.sub_blocks[i])
				os << "  " << *op << std::endl;
		}
		return os;
	}
};

PreLink compile_expression(ast::AST ast);

PreLink compile_program(const std::vector<ast::AST> defines);
