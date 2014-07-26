#pragma once
#include <vector>
#include <string>
#include <memory>
#include <ostream>

namespace ast {

enum Type
{
	VALUE,
	SYMBOL,
	LIST,
};

class Impl;

typedef std::shared_ptr<Impl> AST;

struct Impl
{
	Type type;

	int              value;
	std::string      symbol;
	std::vector<AST> list;

	// For debug
	friend std::ostream& operator<<(std::ostream& os, const Impl& me) {
		switch(me.type){
		case VALUE:  os<<me.value; break;
		case SYMBOL: os<<me.symbol; break;
		case LIST: {
			os<<'('; bool sp=false;
			for(auto& a: me.list) os<<(sp?" ":"")<<*a, sp=true;
			os<<')';
			break;
		}
		}
		return os;
	}
};

} // namespace ast