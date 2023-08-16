#ifndef LOGGER_INCLUDED
#include "log.cc"
#endif

#ifndef INPUT_HANDLER_INCLUDED
#include "input_handler.cc"
#endif

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
