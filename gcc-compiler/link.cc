#include "common.h"
#include "link.h"

void link_and_emit(std::ostream& os, const PreLink& pl, bool show_label)
{
	std::vector<int> offset;
	offset.push_back(pl.main_expression.size());
	for(const auto& block: pl.sub_blocks)
		offset.push_back(offset.back() + block.first.size());

	int line = 0;

	for(auto& op: pl.main_expression) {
		os << *op->resolve(offset) << std::endl;
		++line;
	}

	for(const auto& block: pl.sub_blocks) {
		bool first = true;
		for(auto& op: block.first) {
			os << *op->resolve(offset);
			if(first && show_label) {
				os << "\t\t; [" << line << "] " << block.second;
				first = false;
			}
			os << std::endl;
			++line;
		}
	}
}
