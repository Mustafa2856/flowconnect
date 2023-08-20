#include <sys/wait.h>
#include <unistd.h>
#ifndef PROCESS_INCLUDED
#include "process.cc"
#endif

#define NODE_INCLUDED

struct Node {
  enum Mode {EMPTY, SINGLE, GROUP, LOOP, LINEAR};

  std::string name;
  Mode mode;
  std::vector<Process*> processes;
  std::vector<Node*> children;

  Node();
  void execute();
  void execute_and_exit();
};

Node::Node() {
  mode = EMPTY;
  processes = std::vector<Process*>();
  children = std::vector<Node*>();
}

void Node::execute() {
  if (mode == EMPTY) {
    log(" [Info] Executed node: ", name);
  }
  else if (mode == SINGLE) {
    if (processes.size() != 1) {
      log(" [Error] Length of processes expected to be equal to 1");
    }
    int pid = fork();
    if (pid == 0) {
      processes[0]->execute_and_exit();
    }
    while(waitpid(-1, NULL, 0)>=0);
  }
  else if (mode == GROUP) {
    for (int i=0;i<processes.size();i++) {
      int pid = fork();
      if (pid != 0) continue;
      processes[i]->execute_and_exit();
    }
    while(waitpid(-1, NULL, 0)>=0);
  }
  else if (mode == LOOP) {
    int pid = fork();
    if (pid != 0) {
      // creating pipes and executing from a fork instead of directly to avoid polluting fd space
      while(waitpid(-1, NULL, 0)>=0);
    }
    else {
      //creating pipes
      std::vector<int[2]> pipes(processes.size());
      int max_fd = pipes.size() * 2 + 2;
      for (int i=0;i<pipes.size();i++) {
        if(pipe(pipes[i]) == -1) {
				  log(" [Error] Error occured creating pipe");
          exit(1);
			  }
      }
      for (int i=0;i<processes.size();i++) {
        processes[i]->max_fd = max_fd;
        processes[i]->stdin = pipes[(i-1 + processes.size())%processes.size()][0];
        processes[i]->stdout = pipes[i][1];
      }
      for (int i=0;i<processes.size();i++) {
        int pid = fork();
        if (pid != 0) continue;
        processes[i]->execute_and_exit();
      }
      for(int i=3;i<max_fd;i++) {
        close(i);
      }
      while(waitpid(-1, NULL, 0)>=0);
      exit(0);
    }
  }
  else if (mode == LINEAR) {
    int pid = fork();
    if (pid != 0) {
      // creating pipes and executing from a fork instead of directly to avoid polluting fd space
      while(waitpid(-1, NULL, 0)>=0);
    }
    else {
      //creating pipes
      std::vector<int[2]> pipes(processes.size());
      int max_fd = pipes.size() * 2 + 2;
      for (int i=0;i<pipes.size();i++) {
        if(pipe(pipes[i]) == -1) {
				  log(" [Error] Error occured creating pipe");
          exit(1);
			  }
      }
      processes[0]->max_fd = max_fd;
      for (int i=1;i<processes.size();i++) {
        processes[i]->max_fd = max_fd;
        processes[i]->stdin = pipes[i-1][0];
        processes[i-1]->stdout = pipes[i-1][1];
      }
      for (int i=0;i<processes.size();i++) {
        int pid = fork();
        if (pid != 0) {
          continue;
        }
        processes[i]->execute_and_exit();
      }
      for(int i=3;i<max_fd;i++) {
        close(i);
      }
      while(waitpid(-1, NULL, 0)>=0);
      exit(0);
    }
  }

  for (int i=0;i<children.size();i++) {
    int pid = fork();
    if (pid != 0) continue;
    children[i]->execute_and_exit();
  }
  while(waitpid(-1, NULL, 0)>=0);
}

void Node::execute_and_exit() {
  execute();
  exit(0);
}
