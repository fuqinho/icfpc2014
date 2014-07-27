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

int main()
{
	std::map<std::string, size_t> label;
	std::vector<std::string> cmd;

	for(std::string str; std::getline(std::cin, str); )
	{
		str = clean(str);
		if(str.empty())
			continue;
		size_t i = str.find(':');
		if(i != std::string::npos) {
			label[clean(str.substr(0, i))] = cmd.size();
			str = clean(str.substr(i+1));
			if(!str.empty())
				cmd.push_back(str);
		}
		else
			cmd.push_back(str);
	}

	for(std::string str: cmd) {
		size_t i = str.find(' ');
		size_t k = str.find(',');
		if(i!=std::string::npos && k!=std::string::npos && i<k) {
			auto targ = str.substr(i+1, k-i-1);
			if(label.count(targ))
				str.replace(i+1, k-i-1, lexical_cast(label[targ]));
		}
		std::cout << str << std::endl;
	}
}
