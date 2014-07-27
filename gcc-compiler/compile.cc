#include "compile.h"
#include <algorithm>
#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

#define compiler_assert(cond, ast, msg) \
	do { \
		if(!(cond)) {std::cerr << "ERROR [" << ast->pos() << "]: " << msg << std::endl << std::endl; } \
		assert(cond); \
	} while(0)

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
			return false;
		bool b = parent->resolve(var, depth, index);
		if(b)
			++*depth;
		return b;
	}

private:
	const std::shared_ptr<VarMap> parent;
	std::vector<std::string> vars;
};

typedef std::map<std::string, std::pair<
	std::vector<std::string>,
	ast::AST
>> MacroMap;

struct Context
{
	const std::shared_ptr<VarMap> varmap;
	const std::shared_ptr<std::vector<std::pair<gcc::OperationSequence, std::string>>> codeblocks;
	const std::string name;
	const std::shared_ptr<MacroMap> macros;

	int AddCodeBlock(const gcc::OperationSequence& ops) const {
		return AddCodeBlock(ops, "");
	}

	int AddCodeBlock(const gcc::OperationSequence& ops, const std::string& label) const {
		int id = codeblocks->size();
		codeblocks->emplace_back(ops, label);
		return id;
	}
};

enum IsTailPos { NOT_TAIL, TAIL };

std::vector<std::string> verify_lambda_param_node(ast::AST ast) {
	compiler_assert(ast->type == ast::LIST, ast, "lambda param is not a list");

	std::vector<std::string> vars;
	for(size_t i=0; i<ast->list.size(); ++i) {
		compiler_assert(ast->list[i]->type == ast::SYMBOL, ast, "lambda param not a symbol");
		vars.push_back(ast->list[i]->symbol);
	}
	return vars;
}

ast::AST substitute(ast::AST orig, std::map<std::string, ast::AST>& sub)
{
	switch(orig->type) {
	case ast::VALUE:
		break;
	case ast::SYMBOL:
		if(sub.count(orig->symbol))
			return sub[orig->symbol];
		break;
	case ast::LIST: {
		ast::AST after = std::make_shared<ast::Impl>();
		after->line = orig->line;
		after->column = orig->column;
		after->type = ast::LIST;
		for(auto& e: orig->list)
			after->list.push_back(substitute(e, sub));
		return after;
		}
	}
	return orig;
}

gcc::OperationSequence compile(ast::AST ast, const Context& ctx, IsTailPos tail)
{
	auto compile_op2 = [&](std::shared_ptr<gcc::Op> op) {
		compiler_assert(ast->type == ast::LIST, ast, "ICE1");
		compiler_assert(ast->list.size() >= 3, ast, "too few arguments");

		gcc::OperationSequence ops;
		gcc::Append(&ops, compile(ast->list[1], ctx, NOT_TAIL));
		if(op->assoc_left()) {
			for(size_t i=2; i<ast->list.size(); ++i) {
				gcc::Append(&ops, compile(ast->list[i], ctx, NOT_TAIL));
				gcc::Append(&ops, op);
			}
		} else {
			for(size_t i=2; i<ast->list.size(); ++i)
				gcc::Append(&ops, compile(ast->list[i], ctx, NOT_TAIL));
			for(size_t i=2; i<ast->list.size(); ++i)
				gcc::Append(&ops, op);
		}
		if(tail)
			gcc::Append(&ops, std::make_shared<gcc::OpRTN>());
		return ops;
	};
	auto compile_op2_rev = [&](std::shared_ptr<gcc::Op> op) {
		compiler_assert(ast->type == ast::LIST, ast, "ICE2");
		compiler_assert(ast->list.size() == 3, ast, "argument count mismatch");

		gcc::OperationSequence ops;
		gcc::Append(&ops, compile(ast->list[2], ctx, NOT_TAIL)); // reversed
		gcc::Append(&ops, compile(ast->list[1], ctx, NOT_TAIL));
		gcc::Append(&ops, op);
		if(tail)
			gcc::Append(&ops, std::make_shared<gcc::OpRTN>());
		return ops;
	};
	auto compile_op1 = [&](std::shared_ptr<gcc::Op> op) {
		compiler_assert(ast->type == ast::LIST, ast, "ICE3");
		compiler_assert(ast->list.size() == 2, ast, "argument count mismatch");

		gcc::OperationSequence ops;
		gcc::Append(&ops, compile(ast->list[1], ctx, NOT_TAIL));
		gcc::Append(&ops, op);
		if(tail)
			gcc::Append(&ops, std::make_shared<gcc::OpRTN>());
		return ops;
	};

	switch(ast->type) {
		case ast::VALUE: {
			// constant
			gcc::OperationSequence ops;
			gcc::Append(&ops, std::make_shared<gcc::OpLDC>(ast->value));
			if(tail)
				gcc::Append(&ops, std::make_shared<gcc::OpRTN>());
			return ops;
		}
		case ast::SYMBOL: {
			// variable
			int depth, index;
			if(!ctx.varmap->resolve(ast->symbol, &depth, &index))
				compiler_assert(false, ast, "variable not found: " << ast->symbol);
			gcc::OperationSequence ops;
			gcc::Append(&ops, std::make_shared<gcc::OpLD>(depth, index));
			if(tail)
				gcc::Append(&ops, std::make_shared<gcc::OpRTN>());
			return ops;
		}
		case ast::LIST: {
			if(ast->list.empty()) {
				// use as nil
				gcc::OperationSequence ops;
				gcc::Append(&ops, std::make_shared<gcc::OpLDC>(0));
				if(tail)
					gcc::Append(&ops, std::make_shared<gcc::OpRTN>());
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

				// (dbg! e kont)
				if(ast->list.front()->symbol == "dbg!") {
					compiler_assert(ast->list.size() == 3, ast, "dbg! takes 2 args");

					gcc::OperationSequence ops;
					gcc::Append(&ops, compile(ast->list[1], ctx, NOT_TAIL));
					gcc::Append(&ops, std::make_shared<gcc::OpDBUG>());
					gcc::Append(&ops, compile(ast->list[2], ctx, tail));
					return ops;
				}

				// (set! x e kont)
				if(ast->list.front()->symbol == "set!") {
					compiler_assert(ast->list.size() == 4, ast, "set! takes 3 args");
					compiler_assert(ast->list[1]->type == ast::SYMBOL, ast->list[1], "set! to non-variable");

					int depth, index;
					if(!ctx.varmap->resolve(ast->list[1]->symbol, &depth, &index))
						compiler_assert(false, ast->list[1], "variable not found: " << ast->list[1]->symbol);

					gcc::OperationSequence ops;
					gcc::Append(&ops, compile(ast->list[2], ctx, NOT_TAIL));
					gcc::Append(&ops, std::make_shared<gcc::OpST>(depth, index));
					gcc::Append(&ops, compile(ast->list[3], ctx, tail));
					return ops;
				}

				// (lambda (vars...) e)
				if(ast->list.front()->symbol == "lambda") {
					compiler_assert(ast->list.size() == 3, ast, "lambda takes 2 args");

					std::vector<std::string> vars = verify_lambda_param_node(ast->list[1]);
					std::shared_ptr<VarMap> neo_varmap = std::make_shared<VarMap>(ctx.varmap, vars);
					Context neo_ctx = {neo_varmap, ctx.codeblocks, ctx.name+"L", ctx.macros};

					gcc::OperationSequence body_ops = compile(ast->list[2], neo_ctx, TAIL);
					int id = ctx.AddCodeBlock(body_ops, ctx.name+"L");

					gcc::OperationSequence ops;
					gcc::Append(&ops, std::make_shared<gcc::OpLDF>(id));
					if(tail)
						gcc::Append(&ops, std::make_shared<gcc::OpRTN>());
					return ops;
				}

				// (let ((x e) (x e)) e)
				if(ast->list.front()->symbol == "let") {
					compiler_assert(ast->list.size() == 3, ast, "let takes 2 args");
					compiler_assert(ast->list[1]->type == ast::LIST, ast->list[1], "let takes binding LIST");

					gcc::OperationSequence ops;

					std::vector<std::string> vars;
					for(auto& kv: ast->list[1]->list) {
						compiler_assert(kv->type == ast::LIST, kv, "let binding not a list");
						compiler_assert(kv->list.size() >= 2, kv, "let binding too short");
						for(size_t i=0; i+1<kv->list.size(); ++i) {
							compiler_assert(kv->list[i]->type == ast::SYMBOL, kv->list[i], "let bind to non-variable");
							vars.push_back(kv->list[i]->symbol);
						}
						gcc::Append(&ops, compile(kv->list.back(), ctx, NOT_TAIL));

						// tuple decomposition
						int depth, index;
						if(!ctx.varmap->resolve("__compiler_dummy__", &depth, &index))
							compiler_assert(false, ast, "ICE4");
						for(int tuple=kv->list.size()-1; tuple>=2; --tuple) {
							gcc::Append(&ops, std::make_shared<gcc::OpST>(depth, index));
							gcc::Append(&ops, std::make_shared<gcc::OpLD>(depth, index));
							gcc::Append(&ops, std::make_shared<gcc::OpCAR>());
							gcc::Append(&ops, std::make_shared<gcc::OpLD>(depth, index));
							gcc::Append(&ops, std::make_shared<gcc::OpCDR>());
						}
					}

					std::shared_ptr<VarMap> neo_varmap = std::make_shared<VarMap>(ctx.varmap, vars);
					Context neo_ctx = {neo_varmap, ctx.codeblocks, ctx.name, ctx.macros};

					gcc::OperationSequence body_ops = compile(ast->list[2], neo_ctx, TAIL);
					int id = ctx.AddCodeBlock(body_ops, "("+ctx.name+"-let)");

					gcc::Append(&ops, std::make_shared<gcc::OpLDF>(id));
					if(tail)
						gcc::Append(&ops, std::make_shared<gcc::OpTAP>(vars.size()));
					else
						gcc::Append(&ops, std::make_shared<gcc::OpAP>(vars.size()));
					return ops;
				}

				// (if c t e)
				if(ast->list.front()->symbol == "if") {
					compiler_assert(ast->list.size() == 4, ast, "if takes 3 args");

					gcc::OperationSequence ops = compile(ast->list[1], ctx, NOT_TAIL);
					gcc::OperationSequence t_ops = compile(ast->list[2], ctx, tail);
					if(!tail)
						gcc::Append(&t_ops, std::make_shared<gcc::OpJOIN>());
					int tid = ctx.AddCodeBlock(t_ops, "("+ctx.name+"-then)");

					gcc::OperationSequence e_ops = compile(ast->list[3], ctx, tail);
					if(!tail)
						gcc::Append(&e_ops, std::make_shared<gcc::OpJOIN>());
					int eid = ctx.AddCodeBlock(e_ops, "("+ctx.name+"-else)");

					if(tail)
						gcc::Append(&ops, std::make_shared<gcc::OpTSEL>(tid, eid));
					else
						gcc::Append(&ops, std::make_shared<gcc::OpSEL>(tid, eid));
					return ops;
				}

				// (macro ...)
				if(ctx.macros->count(ast->list.front()->symbol)) {
					std::vector<std::string> params = ctx.macros->at(ast->list.front()->symbol).first;
					ast::AST mbody = ctx.macros->at(ast->list.front()->symbol).second;
					compiler_assert(params.size() == ast->list.size()-1, ast, "macro argnum mismatch: "
						<< ast->list.front()->symbol);

					std::map<std::string, ast::AST> sub;
					for(size_t i=0; i<params.size(); ++i)
						sub[params[i]] = ast->list[i+1];
					ast::AST mbody_subst = substitute(mbody, sub);
					return compile(mbody_subst, ctx, tail);
				}
			}

			// general function applications
			gcc::OperationSequence ops;
			for(size_t i=1; i<ast->list.size(); ++i)
				gcc::Append(&ops, compile(ast->list[i], ctx, NOT_TAIL));
			gcc::Append(&ops, compile(ast->list[0], ctx, NOT_TAIL));
			if(tail)
				gcc::Append(&ops, std::make_shared<gcc::OpTAP>(ast->list.size() - 1));
			else
				gcc::Append(&ops, std::make_shared<gcc::OpAP>(ast->list.size() - 1));
			return ops;
		}
	}
	compiler_assert(false, ast, "ICE5");
	return gcc::OperationSequence();
}

}  // namespace

PreLink compile_program(const std::vector<ast::AST> defines)
{
	typedef std::pair<std::string, std::pair<
		std::vector<std::string>,
		ast::AST
	>> funcdef;
	std::vector<funcdef> funcs;
	std::map<std::string, std::pair<
		std::vector<std::string>,
		ast::AST
	>> macros;

	for(auto ast: defines)
	{
		compiler_assert(ast->type == ast::LIST, ast, "top-level must be define or defmacro");
		compiler_assert(ast->list.size() == 3, ast, "top-level must be define or defmacro");
		compiler_assert(ast->list[0]->type == ast::SYMBOL, ast, "top-level must be define or defmacro");
		compiler_assert(ast->list[0]->symbol == "define" || ast->list[0]->symbol == "defmacro",
			ast, "top-level must be define or defmacro");
		compiler_assert(ast->list[1]->type == ast::LIST, ast, "top-level must be define or defmacro");
		compiler_assert(ast->list[1]->list.size() >= 1, ast, "top-level must be define or defmacro");
		compiler_assert(ast->list[1]->list[0]->type == ast::SYMBOL, ast,
			"top-level must be define or defmacro");

		std::string name = ast->list[1]->list[0]->symbol;
		std::vector<std::string> params;
		for(size_t i=1; i<ast->list[1]->list.size(); ++i) {
			assert(ast->list[1]->list[i]->type == ast::SYMBOL);
			params.push_back(ast->list[1]->list[i]->symbol);
		}

		if(ast->list[0]->symbol == "define") {
			assert(count_if(funcs.begin(), funcs.end(), [&](const funcdef& fd) {
				return fd.first == name;
			}) == 0);
			funcs.emplace_back(name, std::make_pair(params, ast->list[2]));
		} else {
			macros[name] = std::make_pair(params, ast->list[2]);
		}
	}

	std::shared_ptr<VarMap> nil_varmap;

	std::vector<std::string> init_vars = {"arg1", "arg2"};
	std::shared_ptr<VarMap> init_varmap = std::make_shared<VarMap>(nil_varmap, init_vars);

	std::vector<std::string> global_vars;
	for(auto& kv: funcs)
		global_vars.push_back(kv.first);
	global_vars.push_back("__compiler_dummy__");
	std::shared_ptr<VarMap> global_varmap = std::make_shared<VarMap>(init_varmap, global_vars);

	auto codeblocks = std::make_shared<std::vector<std::pair<gcc::OperationSequence,std::string>>>();

	std::vector<int> func_ids;
	int main_offset = -1, i=0;
	for(auto& kv: funcs)
	{
		Context ctx = {
			std::make_shared<VarMap>(global_varmap, kv.second.first),
			codeblocks,
			kv.first,
			std::make_shared<MacroMap>(macros)
		};
		auto body_ops = compile(kv.second.second, ctx, TAIL);
		int id = ctx.AddCodeBlock(body_ops, kv.first);
		if(kv.first == "main")
			main_offset = i;
		func_ids.push_back(id);
		++i;
	}
	assert(main_offset >= 0);
	// (define (__dummy_main__) (main))
	gcc::OperationSequence dummy_main_ops;
	gcc::Append(&dummy_main_ops, std::make_shared<gcc::OpLD>(0,main_offset));
	gcc::Append(&dummy_main_ops, std::make_shared<gcc::OpTAP>(0));
	Context tmp_ctx = {nil_varmap, codeblocks, "__dummy_main__"};
	int dummy_main_id = tmp_ctx.AddCodeBlock(dummy_main_ops, "__dummy_main__");

	gcc::OperationSequence prolog_ops;

	gcc::Append(&prolog_ops, std::make_shared<gcc::OpDUM>(func_ids.size()+1));
	for(int id: func_ids)
		gcc::Append(&prolog_ops, std::make_shared<gcc::OpLDF>(id));
	gcc::Append(&prolog_ops, std::make_shared<gcc::OpLDC>(12345678));  // dummy variable "__compiler_dummy__"
	gcc::Append(&prolog_ops, std::make_shared<gcc::OpLDF>(dummy_main_id));
	gcc::Append(&prolog_ops, std::make_shared<gcc::OpTRAP>(func_ids.size()+1));

	PreLink result = {prolog_ops, *codeblocks};
	return result;
}
