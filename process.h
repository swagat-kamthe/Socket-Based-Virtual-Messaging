
#ifndef PROCESS_H
#define PROCESS_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <wait.h>

#define MAX 32024
#define HMAX 19
#define MSGMAX 5
typedef struct {
    char identifier[50];
    int port;
    int bufCap;
} Process;
#define SHM_SIZE 1024
extern Process myProcess;
//extern Process *otherProcesses;
extern int otherProcessCount;

void *Ear(void *arg);
void Mouth(char *identifier, char *message);


#endif

