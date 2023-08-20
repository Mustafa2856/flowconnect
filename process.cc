#include <cstdio>
#include <unistd.h>
#include <wait.h>

#include <vector>

#ifndef LOGGER_INCLUDED
#include "log.cc"
#endif

#define PROCESS_INCLUDED

struct Process {
	// FDs of std streams
	int stdin = -1;
	int stdout = -1;
	int stderr = -1;
	int max_fd = 2;

	char* path = NULL;
	char** args = NULL;

  Process();
  int execute();
  void execute_and_exit();
};

Process::Process() {
  stdin = -1;
  stdout = -1;
  stderr = -1;
  max_fd = 2;

  path = NULL;
  args = NULL;
}

int Process::execute() {
		if (path == NULL) {
			log(" [Error] No Path found for executable\n");
			exit(1);
		}
		int pid = fork();
		if(pid != 0) {
			log(" [Info ] Created process for pid: ", pid, " for file ", path);
			for(int i=3;i<=max_fd;i++) {
				close(i);
			}
		  while(waitpid(-1, NULL, 0)>=0);	
			return 0;
		} else {
			for(int i=3;i<=max_fd;i++) {
				if(i == stdout || i == stdin || i == stderr)continue;
				close(i);
			}
			if(stdin != -1) dup2(stdin, STDIN_FILENO);
			if(stdout != -1) dup2(stdout, STDOUT_FILENO);
			if(stderr != -1) dup2(stderr, STDERR_FILENO);
			int status = execvp(path, args);
			if (status == -1) {
				log(" [Error] execvp returned -1 while trying to create process::");
				log(" [Error] Path: ", path);
				log(" [Error] Args: ");
				for(int i=0;true;i++) {
          if(args[i] == NULL)break;
					log(" [Error] ", args[i]);
				}
			}
			exit(0);
		}
		return 0;
	}

void Process::execute_and_exit() {
  execute();
  exit(0);
}

