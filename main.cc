#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <map>

// FLAGS :
enum FLAGS {f=0,g,i,v};
// getConfigFromFile; | -f
// singleNodeGroup;   | -g
// getFromStdInput;   | -i
// verbose;           | -v
const int NUM_FLAGS = 4;
const char flags[NUM_FLAGS + 1] = "fgiv";

const int MAX_FILE_SIZE = 32 * 1024; // 32kB

void print_help_msg_and_exit() {

}

bool verbose = false;
void log(){
	if(!verbose)return;
	std::cout<<"\n";
}
template<typename First, typename ...Rest>
void log(First && first, Rest && ...rest) {
	if(!verbose)return;
	std::cout << std::forward<First>(first)<<" ";
	log(std::forward<Rest>(rest)...);
}

struct Process {
	enum Mode {SINGLE, GROUP, LOOP, LINEAR};
	// FDs of std streams
	int stdin;
	int stdout;
	int stderr;

	char* path;
	char** args;
	int num_args;

	Mode mode;
	std::vector<Process*> siblings;
	std::vector<Process*> child_procs;

	Process() {
		stdin = -1;
		stdout = -1;
		stderr = -1;	
		path = NULL;
		num_args = 0;
		args = NULL;
		mode = Mode::SINGLE;
		siblings = std::vector<Process*>();
		child_procs = std::vector<Process*>();
	}

	int execute() {
		if (path == NULL) {
			std::cerr << " [Error] No Path found for executable\n";
			exit(1);
		}
		int pid = fork();
		if(pid != 0) {
			log(" [Info ] Created process for pid: ", pid, " for file ", path);
			for(int i=0;i<siblings.size();i++) {
				siblings[i]->execute();
			}	
			waitpid(pid, NULL, 0);
			for(int i=0;i<child_procs.size();i++) {
				child_procs[i]->execute();
			}
			return 0;
		} else {
			if(stdin != -1) dup2(stdin, STDIN_FILENO);
			if(stdout != -1) dup2(stdout, STDOUT_FILENO);
			if(stderr != -1) dup2(stderr, STDERR_FILENO);
			int status = execvp(path, args);
			if (status == -1) {
				log(" [Error] execvp returned -1 while trying to create process::");
				log(" [Error] Path: ", path);
				log(" [Error] Args: ", num_args);
				for(int i=0;i<num_args;i++) {
					log(args[i]);
				}
			}	
			exit(0);
		}
		return 0;
	}
};

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
					for (int k = 0; k < NUM_FLAGS; k++) {
						if (input[i][j] == flags[k]) {
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
			// TODO
			std::istream in(0);
			file_input = (char*)malloc(MAX_FILE_SIZE);
			read_from_istream(in);
		} else if (flagList[FLAGS::i]) {
			log(" [Info ] Reading from stdin");
			file_input = (char*)malloc(MAX_FILE_SIZE);
			read_from_istream(std::cin);
		} else {
			//TODO
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

void link_proc_tree(Process* head, InputConfig* conf, std::string node) {
	if(node == "") {
		node = conf->head;
	}
	NodeConfig* headNode = conf->group_mapping[node];
	head->mode = headNode->mode;
	// Create pipes and stuff and link the tree to process struct tree.
	if (headNode->mode == Process::SINGLE) {
		head->args = headNode->args.front();
		head->path = headNode->paths.front();
		head->num_args = headNode->num_args.front();
	} else if (headNode->mode == Process::GROUP) {
		head->args = headNode->args.front();
		head->path = headNode->paths.front();
		head->num_args = headNode->num_args.front();
		for(int i=1;i<headNode->paths.size();i++) {
			Process *p = new Process();
			p->args = headNode->args[i];
			p->path = headNode->paths[i];
			p->num_args = headNode->num_args[i];
			head->siblings.push_back(p);
		}
	} else if (headNode->mode == Process::LOOP) {
		int pipes[headNode->paths.size()][2];
		for (int i=0;i<headNode->paths.size();i++) {
			if(pipe(pipes[i]) == -1) {
				log(" [Error] Error occured creating pipe");
			}
		}
		head->args = headNode->args.front();
		head->path = headNode->paths.front();
		head->num_args = headNode->num_args.front();
		head->stdin = pipes[headNode->paths.size() - 1][0];
		head->stdout = pipes[0][1];
		Process* prev = head;
		for(int i=1;i<headNode->paths.size();i++) {
			Process *p = new Process();
			p->args = headNode->args[i];
			p->path = headNode->paths[i];
			p->num_args = headNode->num_args[i];
			p->stdin = pipes[i-1][0];
			p->stdout = pipes[i][1];
			prev->siblings.push_back(p);
			prev = p;
		}
	} else if (headNode->mode == Process::LINEAR) {
		int pipes[headNode->paths.size()][2];
		for (int i=0;i<headNode->paths.size();i++) {
			if(pipe(pipes[i]) == -1) {
				log(" [Error] Error occured creating pipe");
			}
		}
		head->args = headNode->args.front();
		head->path = headNode->paths.front();
		head->num_args = headNode->num_args.front();
		head->stdout = pipes[0][1];
		Process* prev = head;
		for(int i=1;i<headNode->paths.size();i++) {
			Process *p = new Process();
			p->args = headNode->args[i];
			p->path = headNode->paths[i];
			p->num_args = headNode->num_args[i];
			p->stdin = pipes[i-1][0];
			if(i != headNode->paths.size() -1) {
				p->stdout = pipes[i][1];
			}
			prev->siblings.push_back(p);
			prev = p;
		}
	}
	for (auto v: conf->group_tree[node]) {
		Process *p = new Process();
		link_proc_tree(p, conf, v);
		head->child_procs.push_back(p);
	}
}



int main(int argc, char* argv[]) {
	log(" [Info ] Recieved ", argc, " argument(s) ::"); 
	for(int i=0;i<argc;i++){
		log(" [Debug] -> \"", argv[i], "\"");
	}
	log();
	// get control logic
	InputConfig config(argc, argv);
	// create process graph
	Process *head = new Process();
	// link proc tree into head
	link_proc_tree(head, &config, "");
	head->execute();
	return 0;
}


