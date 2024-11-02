#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void parseConfigFile(char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    fscanf(file, "%s %d", myProcess.identifier, &myProcess.port);

    while (fscanf(file, "%s %d", otherProcesses[otherProcessCount].identifier, &otherProcesses[otherProcessCount].port) != EOF) {
        otherProcessCount++;
    }

    fclose(file);
}

