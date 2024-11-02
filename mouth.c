#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "process.h"
extern Process *otherProcesses;

//message header "<process1> <umsg0 | smsg0> <type_of_msg> <index_number_of_part_of_msg> <total_number_of_parts> <part_of_msg>"
void Mouth(char *identifier, char *message) {

    for (int i = 0; i < otherProcessCount; i++) {
        if (strcmp(otherProcesses[i].identifier, identifier) == 0) {
            int sock = 0;
            struct sockaddr_in serv_addr;

            if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                printf("\n Socket creation error \n");
                return;
            }

            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(otherProcesses[i].port);

            // Assuming communication within the same network or broadcast (no IP needed)
            serv_addr.sin_addr.s_addr = INADDR_ANY;
            
            if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                printf("\nConnection Failed %s\n",otherProcesses[i].identifier);
                return;
            }


            char sender_name[50], msgId[10], type[2];
            int index_of_chunk, total_number_of_chunks;
            size_t bytesRead;

           
            sscanf(message, "%s %s %s %d %d %zu",sender_name, msgId, type, &index_of_chunk, &total_number_of_chunks, &bytesRead);
            if (strcmp(type, "D") == 0)
            {     
                     char header[256];
                    snprintf(header, sizeof(header), "%s %s %s %d %d %zu ", 
                    sender_name, msgId, type, index_of_chunk, total_number_of_chunks, bytesRead);
                    send(sock, message, strlen(header)+bytesRead, 0);
            }
            else 
            send(sock, message, strlen(message), 0);
            
            close(sock);
        }
    }
}

