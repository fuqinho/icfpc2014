#include "parse.h"
#include "compile.h"
#include "link.h"
#include <iostream>

int main()
{
	auto asts = parse_program(std::cin) ;
	PreLink pl = compile_program(asts);
	link_and_emit(std::cout, pl, true);
}
