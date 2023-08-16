#include <cstdio>
#include <unistd.h>
#include <wait.h>

#include <vector>

#ifndef LOGGER_INCLUDED
#include "log.cc"
#endif

#define PROCESS_INCLUDED

struct Process {
	enum Mode {SINGLE, GROUP, LOOP, LINEAR};
	// FDs of std streams
	int stdin;
	int stdout;
	int stderr;
	int max_fd;

	char* path;
	char** args;
	int num_args;

	Mode mode;
	std::vector<Process*> siblings;
	std::vector<Process*> child_procs;

	Process() {
		max_fd = 2;
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
			log(" [Error] No Path found for executable\n");
			exit(1);
		}
		int pid = fork();
		if(pid != 0) {
			log(" [Info ] Created process for pid: ", pid, " for file ", path);
			for(int i=0;i<siblings.size();i++) {
				// forking process to unblock parent forker.
				int pid2 = fork();
				if(pid2 != 0)continue;
				siblings[i]->execute();
				exit(0);
			}
			for(int i=3;i<=max_fd;i++) {
				close(i);
			}
			waitpid(pid, NULL, 0);
			// send EOF to stdout of new process to unblock downstream process
			for(int i=0;i<child_procs.size();i++) {
				child_procs[i]->execute();
			}
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

