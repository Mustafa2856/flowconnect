#ifndef LOGGER_INCLUDED
#include "log.cc"
#endif

#ifndef INPUT_HANDLER_INCLUDED
#include "input_handler.cc"
#endif

Node* link_proc_tree(InputConfig* conf, std::string node) {
	if(node == "") {
		node = conf->head;
	}
	Node* headNode = conf->group_mapping[node];
	for (auto v: conf->group_tree[node]) {
		link_proc_tree(conf, v);
		headNode->children.push_back(conf->group_mapping[v]);
	}
  return headNode;
}
