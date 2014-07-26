#include "parse.h"
#include "compile.h"
#include "link.h"
#include <iostream>

int main()
{
	auto asts = parse_program(std::cin) ;
	PreLink pl = compile_program(asts);
	std::cerr << "COMPILED:" << std::endl;
	std::cerr << pl << std::endl;

	std::cerr << "LINKED:" << std::endl;
	gcc::OperationSequence ops = link(pl);
	for(auto& op: ops)
		std::cout << *op << std::endl;
}
