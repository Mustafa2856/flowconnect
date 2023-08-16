#include "log.cc"
#include "process.cc"
#include "input_handler.cc"
#include "linker.cc"

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


