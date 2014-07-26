#include "link.h"


gcc::OperationSequence link(const PreLink& pl)
{
	std::vector<int> offset;
	offset.push_back(pl.main_expression.size());
	for(const auto& block: pl.sub_blocks)
		offset.push_back(offset.back() + block.size());

	gcc::OperationSequence ops;
	for(auto& op: pl.main_expression)
		ops.push_back(op->resolve(offset));
	for(const auto& block: pl.sub_blocks)
		for(auto& op: block)
			ops.push_back(op->resolve(offset));
	return ops;
}
