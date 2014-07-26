#pragma once
#include "ast.h"
#include "gcc.h"
#include <ostream>
#include <utility>

struct PreLink
{
	gcc::OperationSequence              main_expression;
	std::vector<std::pair<gcc::OperationSequence, std::string>> sub_blocks;

	friend std::ostream& operator<<(std::ostream& os, const PreLink& me) {
		for(auto& op: me.main_expression)
			os << *op << std::endl;
		for(size_t i=0; i<me.sub_blocks.size(); ++i) {
			if(me.sub_blocks[i].second.empty())
				os << i << ":" << std::endl;
			else
				os << i << "<" << me.sub_blocks[i].second << ">:" << std::endl;
			for(auto& op: me.sub_blocks[i].first)
				os << "  " << *op << std::endl;
		}
		return os;
	}
};

PreLink compile_program(const std::vector<ast::AST> defines);
