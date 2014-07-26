#include "parse.h"
#include <sstream>
#include <vector>
#include <iostream>
#include <cassert>

namespace parse_impl {

void skip_spaces(const char*& p)
{
	while(*p==' '|| *p=='\t' || *p=='\n')
		++p;
}

bool is_open_paren(char c)
{
	return c=='(' || c=='[' || c=='{';
}

bool is_close_paren(char c)
{
	return c==')' || c==']' || c=='}';
}

std::string parse_token(const char*& p)
{
	std::string token;
	while(*p!=' ' && *p!='\t' && *p!='\n' && !is_close_paren(*p))
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

char paren_match(char op, char cl)
{
	switch(op) {
	case '(': return cl==')';
	case '[': return cl==']';
	case '{': return cl=='}';
	}
	return op==cl;
}

ast::AST parse_expression(const char*& p)
{
	ast::AST ast(new ast::Impl);

	skip_spaces(p);

	if(is_open_paren(*p)) {
		char op = *p;
		++p; // ')'
		std::vector<ast::AST> list;
		while(skip_spaces(p), !is_close_paren(*p))
			list.emplace_back(parse_expression(p));
		char cl = *p;
		assert(paren_match(op,cl));
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

ast::AST parse_expression(const char* p)
{
	return parse_impl::parse_expression(p);
}

ast::AST parse_expression(std::istream& in)
{
	std::stringstream sin;
	sin << in.rdbuf();
	return parse_expression(sin.str().c_str());
}

std::vector<ast::AST> parse_program(std::istream& in)
{
	std::stringstream sin;
	sin << in.rdbuf();
	std::string str = sin.str();
	const char* p = str.c_str();

	std::vector<ast::AST> asts;
	while(parse_impl::skip_spaces(p), *p)
		asts.push_back(parse_impl::parse_expression(p));
	return asts;
}
