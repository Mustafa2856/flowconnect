#include <map>
#include <cstring>
#include <iostream>
#include <fstream>

#include "node.cc"
#include "flags.cc"

#define INPUT_HANDLER_INCLUDED

const std::string DEFAULT_GROUP = "def";
const int MAX_FILE_SIZE = 32 * 1024; // 32kB

struct InputConfig {
	int num_input;
	char** input;
	char* file_input;
	std::map<std::string, NodeConfig*> group_mapping;
	std::string head;
	std::map<std::string, std::vector<std::string>> group_tree;

	bool flagList[4] = {0,0,0,0};

	InputConfig(int num_input,char** input) {
		this->num_input = num_input;
		this->input = input;
		set_mode_from_flags();
		process_input();
	}

	void set_mode_from_flags() {
		for (int i = 0; i < num_input; i++) {
			if(input[i][0] == '-'){
				int len = strlen(input[i]);
				for (int j = 0;j < len; j++) {
					if(j==1 && input[i][j] == '-') {
						if(len == 2) break;
						continue;
						// TODO - GNU style flags
					}
					for (int k = 0; k < FLAGS.size(); k++) {
						if (input[i][j] == FLAGS[k]) {
							flagList[k] = 1;
						}
					}
				}
			}
		}
	}

	void process_input() {
		if (flagList[FLAGS::v]) {
			verbose = true;
		}
		if (flagList[FLAGS::f]) {
			std::string path = "";
			for (int i=0;i<num_input;i++) {
				if(strnlen(input[i],2) && input[i][0] == '-' && input[i][1] == FLAGS[FLAGS::f]) {
					if (i < num_input - 1) {
						path = input[i+1];
					}
					break;
				}
			}
			if(path == "") {
				log("No input file specified after -f flag exiting");
				exit(1);
			}
			std::ifstream in(path);
			file_input = (char*)malloc(MAX_FILE_SIZE);
			read_from_istream(in);
		} else if (flagList[FLAGS::i]) {
			log(" [Info ] Reading from stdin");
			file_input = (char*)malloc(MAX_FILE_SIZE);
			read_from_istream(std::cin);
		} else {
			// single process
			int i=0;
			for(i=1;i<num_input;i++){
				if(input[i][0] != '-') break;
			}
			char* path = input[i];
			char **args = input+i;
			NodeConfig* conf = new NodeConfig();
			conf->paths.push_back(path);
			conf->args.push_back(args);
			conf->mode = Process::SINGLE;
			conf->isFile.push_back(false);
			conf->num_args.push_back(num_input - i);
			head = DEFAULT_GROUP;
			group_mapping[DEFAULT_GROUP] = conf;
		}
	}

	void read_from_istream(std::istream& in) {
		in.getline(file_input, MAX_FILE_SIZE, {});	
		std::vector<std::vector<std::string>> lines = get_lines();
		std::string group;
		char* path;
		for(int i=0;i<lines.size();i++) {
			if(lines[i].empty()) continue;
			if(lines[i][0][0] == '#')continue;
			if(lines[i].size()>1 && lines[i][0] == "-") {
				//New group
				group = lines[i][1];
				NodeConfig* node = new NodeConfig();
				group_mapping[group] = node;
				if(lines[i][1][0] == 'l') {
					node->mode = Process::LOOP;
				} else if (lines[i][1][0] == 'g') {
					node->mode = Process::GROUP;
				} else if (lines[i][1][0] == 's') {
					node->mode = Process::SINGLE;
				} else if (lines[i][1][0] == 'i') {
					node->mode = Process::LINEAR;
				}	
			} else if (lines[i].size()>0 && lines[i][0][0] == '\n') {
				continue;
			} else if (lines[i].size() == 2 && lines[i][0] == "=") {
				head = lines[i][1];
			} else if (lines[i].size() == 3 && lines[i][0] == "=") {
				group_tree[lines[i][1]].push_back(lines[i][2]);
			} else {
				path = const_cast<char*>(lines[i][0].c_str());
				char** args = new char*[lines[i].size()+1];
				for(int j=0;j<lines[i].size();j++) {
					args[j] = const_cast<char*>(lines[i][j].c_str());
				}
				args[lines[i].size()] = NULL;
				group_mapping[group]->paths.push_back(path);
				group_mapping[group]->args.push_back(args);
				group_mapping[group]->num_args.push_back(lines[i].size() - 1);
				group_mapping[group]->isFile.push_back(false);	
			}
		}
	}

	std::vector<std::vector<std::string>> get_lines() {
		int len = strnlen(file_input, MAX_FILE_SIZE);
		char* tmp = (char*)malloc(MAX_FILE_SIZE);
		int p = 0;
		std::vector<std::vector<std::string>> result;
		result.push_back(std::vector<std::string>());
		for(int i=0;i<len;i++) {
			if(file_input[i] == ' ') {
				if(i<=p){p=i+1;continue;}
				strncpy(tmp, file_input + p, i-p);
				tmp[i-p]=0;
				result.rbegin()->push_back(std::string(tmp));
				p=i+1;
			} else if (file_input[i] == '\n') {
				if(i<=p){p=i+1;continue;}
				strncpy(tmp, file_input + p, i-p);
				tmp[i-p]=0;
				result.rbegin()->push_back(std::string(tmp));
				result.push_back(std::vector<std::string>());
				p=i+1;
			}
		}
		return result;
	}


};
