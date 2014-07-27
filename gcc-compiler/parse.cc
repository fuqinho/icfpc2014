#include "common.h"
#include "parse.h"

namespace {

class Cursor
{
public:
	Cursor(const char* p, int line=1, int column=1) : p_(p), line_(line), column_(column) {}

	char operator*() const { return *p_; }
	Cursor& operator++() { incr(); return *this; }
	Cursor operator++(int) { Cursor prev(p_, line_, column_); incr(); return prev; }

	int line() const { return line_; }
	int column() const { return column_; }

private:
	void incr() {
		if(*p_=='\n') { line_++; column_=1; } else { column_++; }
		p_++;
	}
	const char* p_;
	int line_, column_;
};

void skip_spaces(Cursor& p)
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

std::string parse_token(Cursor& p)
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

ast::AST parse_expression(Cursor& p)
{
	skip_spaces(p);

	ast::AST ast(new ast::Impl);
	ast->line = p.line();
	ast->column = p.column();
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

std::string read_skipping_comments(std::istream& in)
{
	std::string all;
	for(std::string str; getline(in, str); ) {
		size_t i = str.find(';');
		if(i != std::string::npos)
			str.resize(i);
		all += str;
		all += '\n';
	}
	return all;
}

}  // namespace

std::vector<ast::AST> parse_program(std::istream& in)
{
	std::string str = read_skipping_comments(in);
	Cursor p(str.c_str());

	std::vector<ast::AST> asts;
	while(skip_spaces(p), *p)
		asts.push_back(parse_expression(p));
	return asts;
}
