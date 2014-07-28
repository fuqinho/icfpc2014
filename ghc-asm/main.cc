#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

std::string clean(const std::string& s)
{
	size_t i = s.find_first_not_of(" \t\n", 0);
	if(i==std::string::npos) i = s.size();
	size_t e = s.find(';', i);
	if(e==std::string::npos) e = s.size();
	return s.substr(i, e-i);
}

std::string lexical_cast(int n)
{
	std::stringstream sin;
	sin << n;
	return sin.str();
}

struct lengthcmp {
	bool operator()(const std::string& lhs, const std::string& rhs) {
		if(lhs.size() != rhs.size())
			return lhs.size()>rhs.size();
		return lhs<rhs;
	}
};

int main()
{
	std::map<std::string, size_t, lengthcmp> defs = {
	// direction
		{"UP", 0},
		{"RIGHT", 1},
		{"DOWN", 2},
		{"LEFT", 3},
	// vitality
		{"STANDARD", 0},
		{"FRIGHT", 1},
		{"INVISIBLE", 2},
	// map
		{"WALL", 0},
		{"EMPTY", 1},
		{"PILL", 2},
		{"POWERPILL", 3},
		{"FRUIT", 4},
		{"LAMBDASTART", 5},
		{"GHOSTSTART", 6},
	// int
		{"SetGhostDirection", 0},
		{"GetLambdaXy", 1},
		{"Get2ndLambdaXy", 2},
		{"GetGhostIndex", 3},
		{"GetGhostStartXy", 4},
		{"GetGhostCurrentXy", 5},
		{"GetGhostState", 6},
		{"GetMap", 7},
		{"Debug", 8},
	};
	std::vector<std::string> cmd;

	for(std::string str; std::getline(std::cin, str); )
	{
		str = clean(str);
		if(str.empty())
			continue;
		size_t i = str.find(':');
		if(i != std::string::npos) {
			defs[clean(str.substr(0, i))] = cmd.size();
			str = clean(str.substr(i+1));
			if(!str.empty())
				cmd.push_back(str);
		}
		else
			cmd.push_back(str);
	}

	for(std::string str: cmd) {
		for(auto kv: defs) {
			size_t i = str.find(kv.first);
			if(i != std::string::npos)
				str.replace(i, kv.first.size(), lexical_cast(kv.second));
		}
		std::cout << str << std::endl;
	}
}
