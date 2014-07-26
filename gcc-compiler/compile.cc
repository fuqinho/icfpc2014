#include "compile.h"
#include <algorithm>
#include <cassert>
#include <functional>
#include <map>
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
		if(!parent)
			std::cerr << "!!! VARIABLE " << var << " NOT FOUND !!!" << std::endl;
		assert(parent);
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

enum IsTailPos { NOT_TAIL, TAIL };

std::vector<std::string> verify_lambda_param_node(ast::AST ast) {
	assert(ast->type == ast::LIST);

	std::vector<std::string> vars;
	for(int i=0; i<ast->list.size(); ++i) {
		assert(ast->list[i]->type == ast::SYMBOL);
		vars.push_back(ast->list[i]->symbol);
	}
	return vars;
}

gcc::OperationSequence rec(ast::AST ast, const Context& ctx, IsTailPos tail)
{
	auto compile_op2 = [&](std::shared_ptr<gcc::Op> op) {
		assert(ast->type == ast::LIST);
		assert(ast->list.size() == 3);

		gcc::OperationSequence ops;
		gcc::Append(&ops, rec(ast->list[1], ctx, NOT_TAIL));
		gcc::Append(&ops, rec(ast->list[2], ctx, NOT_TAIL));
		gcc::Append(&ops, op);
		return ops;
	};
	auto compile_op2_rev = [&](std::shared_ptr<gcc::Op> op) {
		assert(ast->type == ast::LIST);
		assert(ast->list.size() == 3);

		gcc::OperationSequence ops;
		gcc::Append(&ops, rec(ast->list[2], ctx, NOT_TAIL)); // note: reversed
		gcc::Append(&ops, rec(ast->list[1], ctx, NOT_TAIL));
		gcc::Append(&ops, op);
		return ops;
	};
	auto compile_op1 = [&](std::shared_ptr<gcc::Op> op) {
		assert(ast->type == ast::LIST);
		assert(ast->list.size() == 2);

		gcc::OperationSequence ops;
		gcc::Append(&ops, rec(ast->list[1], ctx, NOT_TAIL));
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

				// (lambda (vars...) e)
				if(ast->list.front()->symbol == "lambda") {
					assert(ast->list.size() == 3);

					std::vector<std::string> vars = verify_lambda_param_node(ast->list[1]);
					std::shared_ptr<VarMap> neo_varmap = std::make_shared<VarMap>(ctx.varmap, vars);
					Context neo_ctx = {neo_varmap, ctx.codeblocks};

					gcc::OperationSequence body_ops = rec(ast->list[2], neo_ctx, TAIL);
					gcc::Append(&body_ops, std::make_shared<gcc::OpRTN>());
					int id = ctx.AddCodeBlock(body_ops);

					gcc::OperationSequence ops;
					gcc::Append(&ops, std::make_shared<gcc::OpLDF>(id));
					return ops;
				}

				// (let ((x e) (x e)) e)
				if(ast->list.front()->symbol == "let") {
					assert(ast->list.size() == 3);
					assert(ast->list[1]->type == ast::LIST);

					std::vector<std::string> vars;
					std::vector<ast::AST>    args;
					for(auto& kv: ast->list[1]->list) {
						assert(kv->type == ast::LIST);
						assert(kv->list.size() == 2);
						assert(kv->list[0]->type == ast::SYMBOL);
						vars.push_back(kv->list[0]->symbol);
						args.push_back(kv->list[1]);
					}

					std::shared_ptr<VarMap> neo_varmap = std::make_shared<VarMap>(ctx.varmap, vars);
					Context neo_ctx = {neo_varmap, ctx.codeblocks};

					gcc::OperationSequence body_ops = rec(ast->list[2], neo_ctx, TAIL);
					gcc::Append(&body_ops, std::make_shared<gcc::OpRTN>());
					int id = ctx.AddCodeBlock(body_ops);

					gcc::OperationSequence ops;
					for(auto& arg: args)
						gcc::Append(&ops, rec(arg, ctx, NOT_TAIL));
					gcc::Append(&ops, std::make_shared<gcc::OpLDF>(id));
					gcc::Append(&ops, std::make_shared<gcc::OpAP>(args.size()));
					return ops;
				}

				// (if c t e)
				if(ast->list.front()->symbol == "if") {
					assert(ast->list.size() == 4);

					gcc::OperationSequence ops = rec(ast->list[1], ctx, NOT_TAIL);
					gcc::OperationSequence t_ops = rec(ast->list[2], ctx, NOT_TAIL); // TODO
					gcc::Append(&t_ops, std::make_shared<gcc::OpJOIN>());
					int tid = ctx.AddCodeBlock(t_ops);

					gcc::OperationSequence e_ops = rec(ast->list[3], ctx, NOT_TAIL);
					gcc::Append(&e_ops, std::make_shared<gcc::OpJOIN>());
					int eid = ctx.AddCodeBlock(e_ops);

					gcc::Append(&ops, std::make_shared<gcc::OpSEL>(tid, eid));
					return ops;
				}
			}

			// general function applications
			gcc::OperationSequence ops;
			for(int i=1; i<ast->list.size(); ++i)
				gcc::Append(&ops, rec(ast->list[i], ctx, NOT_TAIL));
			gcc::Append(&ops, rec(ast->list[0], ctx, NOT_TAIL));
			if(tail)
				gcc::Append(&ops, std::make_shared<gcc::OpTAP>(ast->list.size() - 1));
			else
				gcc::Append(&ops, std::make_shared<gcc::OpAP>(ast->list.size() - 1));
			return ops;
		}
	}
	assert(false);
}

}  // namespace

PreLink compile_program(const std::vector<ast::AST> defines)
{
	std::map<std::string, std::pair<
		std::vector<std::string>,
		ast::AST
	>> funcs;

	for(auto ast: defines)
	{
		assert(ast->type == ast::LIST);
		assert(ast->list.size() == 3);
		assert(ast->list[0]->type == ast::SYMBOL);
		assert(ast->list[0]->symbol == "define");
		assert(ast->list[1]->type == ast::LIST);
		assert(ast->list[1]->list.size() >= 1);
		assert(ast->list[1]->list[0]->type == ast::SYMBOL);
		std::string name = ast->list[1]->list[0]->symbol;

		std::vector<std::string> params;
		for(size_t i=1; i<ast->list[1]->list.size(); ++i) {
			assert(ast->list[1]->list[i]->type == ast::SYMBOL);
			params.push_back(ast->list[1]->list[i]->symbol);
		}

		assert(funcs.count(name) == 0);
		funcs[name] = std::make_pair(params, ast->list[2]);
	}

	std::shared_ptr<VarMap> nil_varmap;

	std::vector<std::string> init_vars = {"arg1", "arg2"};
	std::shared_ptr<VarMap> init_varmap = std::make_shared<VarMap>(nil_varmap, init_vars);

	std::vector<std::string> global_vars;
	for(auto& kv: funcs)
		global_vars.push_back(kv.first);
	std::shared_ptr<VarMap> global_varmap = std::make_shared<VarMap>(init_varmap, global_vars);

	auto codeblocks = std::make_shared<std::vector<gcc::OperationSequence>>();

	std::vector<int> func_ids;
	int main_offset = -1, i=0;
	for(auto& kv: funcs)
	{
		Context ctx = {
			std::make_shared<VarMap>(global_varmap, kv.second.first),
			codeblocks
		};
		auto body_ops = rec(kv.second.second, ctx, TAIL);
		gcc::Append(&body_ops, std::make_shared<gcc::OpRTN>());
		int id = ctx.AddCodeBlock(body_ops);
		if(kv.first == "main")
			main_offset = i;
		func_ids.push_back(id);
		++i;
	}
	assert(main_offset >= 0);
	// (define (__dummy_main__) (main))
	gcc::OperationSequence dummy_main_ops;
	gcc::Append(&dummy_main_ops, std::make_shared<gcc::OpLD>(0,main_offset));
	gcc::Append(&dummy_main_ops, std::make_shared<gcc::OpAP>(0));
	gcc::Append(&dummy_main_ops, std::make_shared<gcc::OpRTN>());
	Context tmp_ctx = {nil_varmap, codeblocks};
	int dummy_main_id = tmp_ctx.AddCodeBlock(dummy_main_ops);

	gcc::OperationSequence prolog_ops;

	gcc::Append(&prolog_ops, std::make_shared<gcc::OpDUM>(func_ids.size()));
	for(int id: func_ids)
		gcc::Append(&prolog_ops, std::make_shared<gcc::OpLDF>(id));
	gcc::Append(&prolog_ops, std::make_shared<gcc::OpLDF>(dummy_main_id));
	gcc::Append(&prolog_ops, std::make_shared<gcc::OpRAP>(func_ids.size()));
	gcc::Append(&prolog_ops, std::make_shared<gcc::OpRTN>());

	PreLink result = {prolog_ops, *codeblocks};
	return result;
}
