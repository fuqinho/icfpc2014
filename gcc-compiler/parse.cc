#include "parse.h"
#include <sstream>
#include <vector>
#include <iostream>
using ast::AST;

namespace parse_impl {

void skip_spaces(const char*& p)
{
	while(*p==' '|| *p=='\t' || *p=='\n')
		++p;
}

std::string parse_token(const char*& p)
{
	std::string token;
	while(*p!=' ' && *p!='\t' && *p!='\n' && *p!=')')
		token += *p++;
	return token;
}

// TODO(kinaba) hex?
bool parse_int(const std::string& s, int* value)
{
	int v = 0;
	for(char c: s)
		if('0'<=c && c<='9')
			v = v*10 + (c-'0');
		else
			return false;
	*value = v;
	return true;
}

AST parse_expression(const char*& p)
{
	AST ast(new ast::Impl);

	skip_spaces(p);

	if(*p == '(') {
		++p; // ')'
		std::vector<AST> list;
		while(skip_spaces(p), *p!=')')
			list.emplace_back(parse_expression(p));
		++p; // ')'

		ast->type = ast::LIST;
		ast->list = list;
	} else {
		std::string token = parse_token(p);
		int value;
		if(parse_int(token, &value)) {
			ast->type = ast::VALUE;
			ast->value = value;
		} else {
			ast->type = ast::SYMBOL;
			ast->symbol = token;
		}
	}
	return ast;
}

}  // namespace parse_impl

AST parse_expression(const char* p)
{
	return parse_impl::parse_expression(p);
}

AST parse_expression(std::istream& in)
{
	std::stringstream sin;
	sin << in.rdbuf();
	return parse_expression(sin.str().c_str());
}
