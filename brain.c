#include <sys/shm.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/ipc.h>
#include <wait.h>
#include <time.h>
#include "process.h"

// Define the global variables here
Process myProcess;
Process *otherProcesses;
int otherProcessCount = 0;

int sysMsgCnt = 0;
typedef struct node{
  char sid[50],mid[10],msgType[5],filePath[50];
  int i,n,arr[256],cmplt;
  long time_diff_ms;
  struct node *next;
} node;
node *head = NULL;
node* push(char *sid,char *mid,char *msgType,int i,int n)
{
  node *newnode,*temp;
  newnode = (node *)malloc(sizeof(node));
  strcpy(newnode->sid,sid);
  strcpy(newnode->mid,mid);
  newnode->i = i;
  newnode->n = n;
  newnode->cmplt = 0;
  newnode->time_diff_ms = 0;
  strcpy(newnode->msgType,msgType);
  memset(newnode->arr,0,n);
  if(head == NULL)
    head = newnode;
  else
  {
    temp = head;
    while(temp->next != NULL)
      temp = temp->next;
    temp->next = newnode;
  }
  return newnode;
}
node* pushFileName(char *sid,char *mid,char *msgType,char *filePath,int i,int n)
{
  node *newnode,*temp;
  newnode = (node *)malloc(sizeof(node));
  strcpy(newnode->sid,sid);
  strcpy(newnode->mid,mid);
  newnode->i = i;
  newnode->n = n;
  newnode->cmplt = 0;
  newnode->time_diff_ms = 0;
  strcpy(newnode->msgType,msgType);
  strcpy(newnode->filePath,filePath);
  memset(newnode->arr,0,n);
  if(head == NULL)
    head = newnode;
  else
  {
    temp = head;
    while(temp->next != NULL)
      temp = temp->next;
    temp->next = newnode;
  }
  return newnode;
}

void delete(node *temp)
{
  if(head == NULL)
    printf("Error");
  else
  {
    node *t = head,*prev;
    while(t!=NULL)
    {
        if(strcmp(t->sid,temp->sid) == 0 && strcmp(t->mid,temp->mid) == 0)
        {
          if(t != head)
          {
            prev->next = t->next;
          }
          else
          { 
            head = head->next;
          }
          break;
        }
         prev = t;
         t = t->next; 
    }
      
    
  }
}
node* search(char *sid, char *mid)
{
  node *temp = head;
  while(temp != NULL)
  {
      if(strcmp(sid,temp->sid) == 0 && strcmp(mid,temp->mid) == 0)
          return temp;
      temp = temp->next;
  }
  return NULL;
}
int countDigit(int num)
{
    int digit = 0;
    do
    {
    num/=10;
    digit++;
    }while(num);
    return digit;
}


void handle_message(char *msg) {
  struct timespec start, end;
  long time_diff_ms;
  clock_gettime(CLOCK_MONOTONIC, &start);
  node *temp;
  char sid[50],msgType[5],mid[10],fileName[60];
  int i,n,k;
  strcpy(sid,strtok(msg," "));
  strcpy(mid,strtok(msg + strlen(sid)+1," "));
  strcpy(msgType,strtok(msg + strlen(sid)+strlen(mid)+2," "));
  sscanf(msg + strlen(sid)+strlen(mid)+strlen(msgType)+3, "%d" , &i);
  sscanf(msg + strlen(sid)+strlen(mid)+strlen(msgType)+4+countDigit(i), "%d" , &n);
  if(strncmp(mid,"smsg",4)==0)
  { 
    systemMSG(sid,mid,i,n,msg + strlen(sid)+strlen(mid)+strlen(msgType)+7);
    return;
    systemMSG(sid,mid,i,n,msg + strlen(sid)+strlen(mid)+strlen(msgType)+7);
  }
    
  
  if(strcmp(msgType,"T") == 0)
  {
      temp = search(sid,mid);
      if(temp == NULL)
      {
          temp = push(sid,mid,msgType,i,n);
      }
      strcpy(fileName,sid);
      strcat(fileName,mid);
      FILE *file = fopen(fileName, "a");
      if(file == NULL) {
           perror("Failed to open file");
           exit(EXIT_FAILURE);
      }
      fprintf(file, "%s " , sid);
      fprintf(file, "%s " , mid);
      fprintf(file, "%s " , msgType);
      fprintf(file, "%d " , i);
      fprintf(file, "%d " , n);
      fprintf(file, "%s\n" , msg+strlen(sid)+strlen(msgType)+strlen(mid)+5+countDigit(i)+countDigit(n));
      fclose(file);
      temp->arr[i] = 1;
      for(k=0;k<n;k++)
      {
        if(temp->arr[k] == 0)
        {
          temp->cmplt = 0;
          break;
        }
        temp->cmplt = 1;
      }
      if(temp->time_diff_ms > temp->n*20000)
      {
        
        for(int i = 0;i<otherProcessCount;i++)
            {
              if(strcmp(otherProcesses[i].identifier,temp->sid) == 0)
              { 
                  char bufMsg[50];
                  sprintf(bufMsg,"%s smsg%d T %d %d hell",myProcess.identifier,sysMsgCnt++,0,1);
                  Mouth(otherProcesses[i].identifier,bufMsg);
                  break;
              }
            } 
        remove(fileName);  
        delete(temp);fprintf(file, "%s " , mid);
        return;
      }
      if(temp->cmplt == 1)
      {
        appendMSG(fileName);
        remove(fileName);
        clock_gettime(CLOCK_MONOTONIC, &end);
        temp->time_diff_ms += (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
        delete(temp);
        return;
      }
  }
  else
  {
  
    if(strcmp(msgType,"D") == 0)
    { 
      static int c = 1;
      char *token;
      token = strtok(msg + strlen(sid)+strlen(mid)+strlen(msgType)+countDigit(i)+countDigit(n)+5," ");
      size_t bytesRead;
      sscanf(token,"%zu",&bytesRead);
      node *temp = search(sid,mid);
      char fileName[100];
      sprintf(fileName,"./%sFiles/",myProcess.identifier);
      strcat(fileName,temp->filePath);
      FILE *file = fopen(fileName,"ab");
      if(file == NULL)
        printf("Cannot open file");
     
     fwrite(msg+strlen(sid)+strlen(mid)+6+strlen(msgType)+countDigit(i)+countDigit(n)+strlen(token),1,bytesRead,file);
     fwrite(msg+strlen(sid)+strlen(mid)+6+strlen(msgType)+countDigit(i)+countDigit(n)+strlen(token),1,bytesRead,stdout);
     fclose(file);
     c++;
    }
    else if(strcmp(msgType,"F") == 0)
    { 
      char fileName[100];
      sprintf(fileName,"./%sFiles/",myProcess.identifier);
      strcat(fileName,msg + strlen(sid)+strlen(mid)+strlen(msgType)+5+countDigit(i)+countDigit(n));
      FILE *file = fopen(fileName,"wb");
      close(file);
      temp = pushFileName(sid,mid,msgType,msg + strlen(sid)+strlen(mid)+strlen(msgType)+5+countDigit(i)+countDigit(n),i,n);   
    }
  }
  memset(sid, 0, sizeof(sid));
  memset(mid, 0, sizeof(mid));
  memset(msgType, 0, sizeof(msgType)); 
  clock_gettime(CLOCK_MONOTONIC, &end);
  temp->time_diff_ms += (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
}

void appendMSG(char *fileName)
{
  char msg[100],*receiver,*t1,*t2,*t3,*t4;
  FILE *file = fopen(fileName, "r");
  if (file == NULL) {
    perror("Failed to open file");
    exit(EXIT_FAILURE);
  }
  char path[50] = "./";
  strcat(path,myProcess.identifier);
  strcat(path,"Files/");
  strcat(path,myProcess.identifier);
  FILE *file1 = fopen(path, "a");
  if (file == NULL) {
    perror("Failed to open file");
    exit(EXIT_FAILURE);
  }
  fseek(file, 0, SEEK_SET);
  int flag = 0;
  while(fgets(msg, 100, file))
  {
    receiver = strtok(msg," ");
    t1 = strtok(NULL," ");
    t2 = strtok(NULL," ");
    t3 = strtok(NULL," ");
    t4 = strtok(NULL," ");
    
    if(strlen(msg+strlen(receiver)+strlen(t1)+strlen(t2)+strlen(t3)+strlen(t4)+5)>0)
      msg[strlen(receiver)+strlen(t1)+strlen(t2)+strlen(t3)+strlen(t4)+5+strlen(msg+strlen(receiver)+strlen(t1)+strlen(t2)+strlen(t3)+strlen(t4)+5)-1] = '\0';
    if(flag == 0)
    {
      flag = 1;
      fprintf(file1, "%s | %s",receiver,msg+strlen(receiver)+strlen(t1)+strlen(t2)+strlen(t3)+strlen(t4)+5);
    }
    else
      fprintf(file1, "%s",msg+strlen(receiver)+strlen(t1)+strlen(t2)+strlen(t3)+strlen(t4)+5);
  }
  fprintf(file1,"\n");  
  fclose(file);
  fclose(file1);
}


void systemMSG(char *sid, char *mid, int i, int n, char *msg)
{
    Process *otherProcesses;
    int shmid;
    key_t key;
    char path[50] = "./";
    strcat(path,myProcess.identifier);
    strcat(path,"Files/");
    strcat(path,myProcess.identifier);
    key = ftok(path, 'a');
    
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    // Locate the shared memory segment
    if ((shmid = shmget(key, SHM_SIZE, 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    // Attach to the shared memory segment
    if ((otherProcesses = (struct Process *)shmat(shmid, NULL, 0)) == (void *) -1) {
        perror("shmat");
        exit(1);
    }
    int j=0,flag = 0;
    for( j = 0;j<otherProcessCount;j++)
     {
        if(strcmp(otherProcesses[j].identifier,sid) == 0 && strcmp(mid,"smsg0") == 0)
        {   
            flag = 1;
            int bufCap,port;
            sscanf(msg, "%d %d" , &bufCap,&port);
            otherProcesses[j].bufCap = bufCap;
            break;
        }
        else
        {
            if(strcmp(mid,"smsg0") != 0)
            {
                FILE *file1 = fopen(myProcess.identifier, "a");
                if (file1 == NULL) {
                  perror("Failed to open file");
                  exit(EXIT_FAILURE);
                }
                fprintf(file1, "%s | %s\n",sid,msg);
                fclose(file1);
                break;
            }
        }
      }
}


void parseConfigFile(char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    fscanf(file, "%s %d %d", myProcess.identifier, &myProcess.port, &myProcess.bufCap);

    while (fscanf(file, "%s %d", otherProcesses[otherProcessCount].identifier, &otherProcesses[otherProcessCount].port) != EOF) {
        otherProcessCount++;  
     }
    fclose(file);
}


void createDirectory()
{
  char *dirName;
  sprintf(dirName,"./%sFiles/",myProcess.identifier);
  struct stat st;
  if (stat(dirName, &st) == -1) {
        mkdir(dirName, 0700);
  }
}
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <config_file>\n", argv[0]);
        return 1;
    }
    
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    fscanf(file, "%s %d %d", myProcess.identifier, &myProcess.port, &myProcess.bufCap);
    fclose(file);
    
    int shmid;
    key_t key;
    char path[50] = "./";
    strcat(path,myProcess.identifier);
    strcat(path,"Files/");
    strcat(path,myProcess.identifier);
    key = ftok(path,'a');
    if (key == -1) {
        perror("ftok");
        exit(1);
    }
    // Create shared memory segment
    if ((shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT)) < 0) {
        perror("shmget");
        exit(1);
    }
    // Attach to shared memory
    if ((otherProcesses = (struct Process *)shmat(shmid, NULL, 0)) == (void *) -1) {
        perror("shmat");
        exit(1);
    }

    parseConfigFile(argv[1]);
    createDirectory();
    sleep(1);
    pid_t pidEar = fork();
    if (pidEar == 0) {
        prctl(PR_SET_NAME,"ear");
        pthread_t recvThread;
        pthread_create(&recvThread, NULL, Ear, NULL);
        pthread_join(recvThread, NULL);
        exit(0);
    }

    // Add a delay to ensure Ear is ready
    pid_t pidMouth = fork();
    if (pidMouth == 0) {
        prctl(PR_SET_NAME,"mouth");
        char message[1000];
        char target[50],myString[100],mid[10] = "umsg";
        int choice,msgCounter=0;
        
        //sending buffer size to other systems
        for(int i = 0;i<otherProcessCount;i++)
        {
          char bufMsg[50];
          sprintf(bufMsg,"%s smsg%d T %d %d %d %d",myProcess.identifier,sysMsgCnt,0,1,myProcess.bufCap,myProcess.port);
          sleep(1);
          Mouth(otherProcesses[i].identifier,bufMsg);
          
        } 
        sysMsgCnt++;   
        while (1) {
            printf("Process Name : %s \nEnter choice 1 : Send Message \n\t     2 : View Received Message \n\t     3 : View Systems\n==>",myProcess.identifier);
            scanf("%d",&choice);
            
            switch(choice)
            {
                case 1:
                        int choice1;
                        printf("Enter choice 1 : Text Message \n\t     2 : File Message \n==>");
                        scanf("%d",&choice1);
                        switch(choice1)
                        {
                          case 1:
                                  printf("Enter choice");
                                  for(int i = 0; i < otherProcessCount ; i++)
                                  {
                                    if(i!=0)
                                      printf("\t    ");
                                    printf(" %d : %s\n",i+1,otherProcesses[i].identifier);
                                  }
                                  printf("==>");
                                  scanf("%d",&choice);            
                                  printf("Enter message: \n==>");
                                  scanf(" %[^\n]s", message);
                                  int i = 0,flag = 0,extra;
                                  if(strlen(message)%(otherProcesses[choice-1].bufCap-1) == 0)
                                    extra = 0;
                                  else
                                    extra = 1;
                                  
                                  while(strlen(message)/(otherProcesses[choice-1].bufCap-1)+extra != i || flag != 1)
                                  {
                                    strcpy(myString,myProcess.identifier);
                                    strcpy(myString+strlen(myString)," ");
                                    strcpy(myString + strlen(myString),mid);
                                    sprintf(myString+strlen(myString),"%d",msgCounter);
                                    sprintf(myString+strlen(myString)," T");
                                    strcpy(myString + strlen(myString), " ");
                                    sprintf(myString+strlen(myString),"%d",i);
                                    strcpy(myString + strlen(myString), " ");
                                    sprintf(myString+strlen(myString),"%d",strlen(message)/(otherProcesses[choice-1].bufCap-1)+extra);
                                    strcpy(myString + strlen(myString), " ");
                                    strncpy(myString + strlen(myString),message + i*(otherProcesses[choice-1].bufCap-1),otherProcesses[choice-1].bufCap-1);
                                    myString[strlen(myString)] = '\0';
                                    Mouth(otherProcesses[choice-1].identifier, myString);
                                    memset(myString, 0, sizeof(myString));
                                    i++;
                                    flag = 1;
                                  }
                                  msgCounter++;
                                  break;
                      case 2 :
                              for(int i = 0; i < otherProcessCount ; i++)
                                  {
                                    printf("Enter %d : %s\n",i+1,otherProcesses[i].identifier);
                                  }
                                  printf("==>");
                                  scanf("%d",&choice);            
                                  printf("Enter file path: \n==>");
                                  scanf(" %[^\n]s", message);
                                  int i1 = 0;
                                  strcpy(myString,myProcess.identifier);
                                  strcpy(myString+strlen(myString)," ");
                                  strcpy(myString + strlen(myString),mid);
                                  sprintf(myString+strlen(myString),"%d",msgCounter);
                                  sprintf(myString+strlen(myString)," F");
                                  strcpy(myString + strlen(myString), " ");
                                  sprintf(myString+strlen(myString),"%d",1); //i1 is instead of i1
                                  strcpy(myString + strlen(myString), " ");
                                  sprintf(myString+strlen(myString),"%d",1);
                                  strcpy(myString + strlen(myString), " ");
                                  strcpy(myString + strlen(myString),message);
                                  myString[strlen(myString)] = '\0';
                                  
                                  Mouth(otherProcesses[choice-1].identifier, myString);
                                  memset(myString, 0, sizeof(myString));
                                  i1++;
                                  
                                  char buffer[31900],myString1[32024];
                                  size_t bytesRead;
                                  FILE *file = fopen(message,"rb");
                                  if(file == NULL)
                                    printf("File %s not found",message);
                                  int tc =1;
                                  while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                                      snprintf(myString1, sizeof(myString1), "%s %s%d D %d %d %zu ", myProcess.identifier, mid, msgCounter, i1, tc, bytesRead);
                                      memcpy(myString1+strlen(myString1),buffer, bytesRead);
                                      fwrite(buffer,1,bytesRead,stdout);
                                      Mouth(otherProcesses[choice-1].identifier,myString1);
                                      i1++;
                                  }
                                  msgCounter++;
                                  fclose(file);
                                  break;
                }
                break;
                case 2:
                        printf("-----------------Messages-----------------\n");
                        char path[50] = "./";
                        strcat(path,myProcess.identifier);
                        strcat(path,"Files/");
                        strcat(path,myProcess.identifier);
                        FILE *file = fopen(path, "r");
                        if (file == NULL) {
                          perror("Failed to open file");
                          exit(EXIT_FAILURE);
                        }
                        fseek(file, 0, SEEK_SET);
                        while(fgets(myString, 100, file)) {
                          printf("%s", myString);
                        }
                        printf("\n------------------------------------------\n");
                        memset(myString, 0, sizeof(myString));
                        fclose(file);
                        break;
                case 3:
                printf("------------------------------------------\n");
                printf("Sr. No.------Process Name--------Capacity\n");
                for(int i = 0;i<otherProcessCount;i++)
                {
                        printf("  %d   -------   %s   -------   %d\n",i+1,otherProcesses[i].identifier,otherProcesses[i].bufCap);
                }printf("------------------------------------------\n");
              break;
            }
  // -1 to correct the index
        }
        exit(0);
    }
      // Detach from shared memory
       wait(NULL);
       wait(NULL);
    shmdt(otherProcesses);
    shmctl(shmid, IPC_RMID, NULL);
        
    return 0;
}

