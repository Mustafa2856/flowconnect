#ifndef PROCESS_INCLUDED
#include "process.cc"
#endif

struct NodeConfig {
	Process::Mode mode;
	std::vector<bool> isFile;
	std::vector<char*> paths;
	std::vector<int> num_args;
	std::vector<char**> args;

	NodeConfig() {
		isFile = std::vector<bool>();
		paths = std::vector<char*>();
		num_args = std::vector<int>();
		args = std::vector<char**>();
	}
};
