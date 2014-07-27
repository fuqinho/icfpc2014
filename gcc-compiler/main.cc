#include "common.h"
#include "parse.h"
#include "compile.h"
#include "link.h"

int main()
{
	auto asts = parse_program(std::cin) ;
	PreLink pl = compile_program(asts);
	link_and_emit(std::cout, pl, true);
}
