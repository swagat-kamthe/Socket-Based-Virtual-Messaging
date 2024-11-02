#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include "process.h"
#include <time.h>
/*void func(char *msg)
{
}*/

extern void handle_message(char*);  

void *Ear(void *arg) {

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[32024] = {0};

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // Bind to all available interfaces
    address.sin_port = htons(myProcess.port);

    // Bind to the port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    char path[50] = "./";
    strcat(path,myProcess.identifier);
    strcat(path,"Files/");
    strcat(path,myProcess.identifier);
    FILE *file = fopen(path, "w");
    if (file == NULL) {
      perror("Failed to open file");
      exit(EXIT_FAILURE);
    }
    fclose(file);
    
    while ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) >= 0) {
        int valread = read(new_socket, buffer, MAX);
	handle_message(buffer);
        memset(buffer, 0, sizeof(buffer));
    }
    close(server_fd);
    return NULL;
}

