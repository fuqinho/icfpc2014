#include "compile.h"
#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

namespace {

class VarMap
{
public:
	VarMap(std::shared_ptr<VarMap> parent, const std::vector<std::string>& vars)
		: parent(parent), vars(vars) {}

	bool resolve(const std::string& var, int* depth, int* index) const {
		auto it = std::find(vars.begin(), vars.end(), var);
		if(it != vars.end()) {
			*depth = 0;
			*index = it - vars.begin();
			return true;
		}
		bool b = parent->resolve(var, depth, index);
		if(b)
			++*depth;
		return b;
	}

private:
	const std::shared_ptr<VarMap> parent;
	std::vector<std::string> vars;
};

struct Context
{
	const std::shared_ptr<VarMap> varmap;
	const std::shared_ptr<std::vector<gcc::OperationSequence>> codeblocks;

	int AddCodeBlock(const gcc::OperationSequence& ops) const {
		int id = codeblocks->size();
		codeblocks->push_back(ops);
		return id;
	}
};

std::vector<std::string> verify_lambda_param_node(ast::AST ast) {
	assert(ast->type == ast::LIST);

	std::vector<std::string> vars;
	for(int i=0; i<ast->list.size(); ++i) {
		assert(ast->list[i]->type == ast::SYMBOL);
		vars.push_back(ast->list[i]->symbol);
	}
	return vars;
}

gcc::OperationSequence rec(ast::AST ast, const Context& ctx)
{
	auto compile_op2 = [&](std::shared_ptr<gcc::Op> op) {
		assert(ast->type == ast::LIST);
		assert(ast->list.size() == 3);

		gcc::OperationSequence ops;
		gcc::Append(&ops, rec(ast->list[2], ctx));
		gcc::Append(&ops, rec(ast->list[1], ctx));
		gcc::Append(&ops, op);
		return ops;
	};
	auto compile_op2_rev = [&](std::shared_ptr<gcc::Op> op) {
		assert(ast->type == ast::LIST);
		assert(ast->list.size() == 3);

		gcc::OperationSequence ops;
		gcc::Append(&ops, rec(ast->list[1], ctx));  // note: reversed
		gcc::Append(&ops, rec(ast->list[2], ctx));
		gcc::Append(&ops, op);
		return ops;
	};
	auto compile_op1 = [&](std::shared_ptr<gcc::Op> op) {
		assert(ast->type == ast::LIST);
		assert(ast->list.size() == 2);

		gcc::OperationSequence ops;
		gcc::Append(&ops, rec(ast->list[1], ctx));
		gcc::Append(&ops, op);
		return ops;
	};

	switch(ast->type) {
		case ast::VALUE: {
			// constant
			gcc::OperationSequence ops;
			gcc::Append(&ops, std::make_shared<gcc::OpLDC>(ast->value));
			return ops;
		}
		case ast::SYMBOL: {
			// variable
			int depth, index;
			if(!ctx.varmap->resolve(ast->symbol, &depth, &index))
				assert(false);
			gcc::OperationSequence ops;
			gcc:Append(&ops, std::make_shared<gcc::OpLD>(depth, index));
			return ops;
		}
		case ast::LIST: {
			if(ast->list.empty()) {
				// use as nil
				gcc::OperationSequence ops;
				gcc::Append(&ops, std::make_shared<gcc::OpLDC>(0));
				return ops;
			} else if(ast->list.front()->type==ast::SYMBOL) {
				// special forms
				if(ast->list.front()->symbol == "+")
					return compile_op2(std::make_shared<gcc::OpADD>());
				if(ast->list.front()->symbol == "-")
					return compile_op2(std::make_shared<gcc::OpSUB>());
				if(ast->list.front()->symbol == "*")
					return compile_op2(std::make_shared<gcc::OpMUL>());
				if(ast->list.front()->symbol == "/")
					return compile_op2(std::make_shared<gcc::OpDIV>());
				if(ast->list.front()->symbol == "=")
					return compile_op2(std::make_shared<gcc::OpCEQ>());
				if(ast->list.front()->symbol == ">")
					return compile_op2(std::make_shared<gcc::OpCGT>());
				if(ast->list.front()->symbol == ">=")
					return compile_op2(std::make_shared<gcc::OpCGTE>());
				if(ast->list.front()->symbol == "<")
					return compile_op2_rev(std::make_shared<gcc::OpCGT>());
				if(ast->list.front()->symbol == "<=")
					return compile_op2_rev(std::make_shared<gcc::OpCGTE>());
				if(ast->list.front()->symbol == "int?")
					return compile_op1(std::make_shared<gcc::OpATOM>());
				if(ast->list.front()->symbol == "cons")
					return compile_op2(std::make_shared<gcc::OpCONS>());
				if(ast->list.front()->symbol == "car")
					return compile_op1(std::make_shared<gcc::OpCAR>());
				if(ast->list.front()->symbol == "cdr")
					return compile_op1(std::make_shared<gcc::OpCDR>());

				// lambda
				if(ast->list.front()->symbol == "lambda") {
					assert(ast->list.size() == 3);

					std::vector<std::string> vars = verify_lambda_param_node(ast->list[1]);
					std::shared_ptr<VarMap> neo_varmap = std::make_shared<VarMap>(ctx.varmap, vars);
					Context neo_ctx = {neo_varmap, ctx.codeblocks};

					gcc::OperationSequence body_ops = rec(ast->list[2], neo_ctx);
					gcc::Append(&body_ops, std::make_shared<gcc::OpRTN>());
					int id = ctx.AddCodeBlock(body_ops);

					gcc::OperationSequence ops;
					gcc::Append(&ops, std::make_shared<gcc::OpLDF>(id));
					return ops;
				}

				// if
				if(ast->list.front()->symbol == "if") {
					assert(ast->list.size() == 4);

					gcc::OperationSequence ops = rec(ast->list[1], ctx);
					gcc::OperationSequence t_ops = rec(ast->list[2], ctx);
					gcc::Append(&t_ops, std::make_shared<gcc::OpJOIN>());
					int tid = ctx.AddCodeBlock(t_ops);

					gcc::OperationSequence e_ops = rec(ast->list[3], ctx);
					gcc::Append(&e_ops, std::make_shared<gcc::OpJOIN>());
					int eid = ctx.AddCodeBlock(e_ops);

					gcc::Append(&ops, std::make_shared<gcc::OpSEL>(tid, eid));
					return ops;
				}
			}

			// general function applications
			gcc::OperationSequence ops;
			for(int i=ast->list.size()-1; i>=0; --i)
				gcc::Append(&ops, rec(ast->list[i], ctx));
			gcc::Append(&ops, std::make_shared<gcc::OpAP>(ast->list.size() - 1));
			return ops;
		}
	}
	assert(false);
}

}  // namespace

PreLink compile(ast::AST ast)
{
	std::shared_ptr<VarMap> nil_varmap;
	auto codeblocks = std::make_shared<std::vector<gcc::OperationSequence>>();
	Context ctx = {nil_varmap, codeblocks};
	auto mainblock = rec(ast, ctx);
	PreLink result = {mainblock, *codeblocks};
	return result;
}
