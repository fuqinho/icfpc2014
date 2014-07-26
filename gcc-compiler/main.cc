#include "parse.h"
#include "compile.h"
#include "link.h"
#include <iostream>

int main()
{
	ast::AST ast = parse_expression(std::cin) ;
	std::cerr << "PARSED:" << std::endl;
	std::cerr << *ast << std::endl;

	PreLink pl = compile(ast);
	std::cerr << "COMPILED:" << std::endl;
	std::cerr << pl << std::endl;

	std::cerr << "LINKED:" << std::endl;
	gcc::OperationSequence ops = link(pl);
	for(auto& op: ops)
		std::cout << *op << std::endl;
}
