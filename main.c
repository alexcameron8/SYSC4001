#include<stdio.h>
#include <string.h>
#include<stdlib.h>
#include "assignment1.h"

#define MEMORY 100

//Limiting total number of processes to 100 (Change?)
//Note: This array acts as the data structure in memory like a PCB
struct process list_of_processes[MEMORY];

void init_list_of_processes(){
//initializes all processes in memory to be Undefined.
  for(int i=0; i<MEMORY; i++){
    list_of_processes[i].state= PROCESS_UNDEFINED;
  }

}

void list_add(int pid, int arrivalTime, int totalCPUTime, int ioFrequency, int ioDuration){
  for(int i=0;i<MEMORY;i++){
    if(list_of_processes[i].state == PROCESS_UNDEFINED){
      list_of_processes[i].pid = pid;
      list_of_processes[i].arrivalTime = arrivalTime;
      list_of_processes[i].totalCPUTime = totalCPUTime;
      list_of_processes[i].ioFrequency = ioFrequency;
      list_of_processes[i].ioDuration = ioDuration;
      list_of_processes[i].state = PROCESS_WAITING;
      return;
    }
  }
  printf("There is no more allocated memory.");
}

//NOTE: Delete all print statements later
void getProcessData(char *processData){
  const char delim[2] = " ";
  char *process_data;
  process_data = strtok(processData,delim);
  int pid = atoi(process_data);
  printf("pid: %s \n",process_data);
  process_data = strtok(NULL,delim);
  int arrivalTime = atoi(process_data);
  printf("Arrival Time: %s \n",process_data);
  process_data = strtok(NULL,delim);
  int totalCPUTime = atoi(process_data);
  printf("Total CPU Time: %s \n",process_data);
  process_data = strtok(NULL,delim);
  int ioFrequency = atoi(process_data);
  printf("I/O Frequency: %s \n",process_data);
  process_data = strtok(NULL,delim);
  int ioDuration = atoi(process_data);
  printf("I/O Duration: %s \n",process_data);
  list_add(pid, arrivalTime, totalCPUTime, ioFrequency, ioDuration);
}

const char* getState(enum process_state state)
{
   switch (state)
   {
      case PROCESS_SUSPENDED: return "SUSPENDED";
      case PROCESS_STOPPED: return "STOPPED";
      case PROCESS_WAITING: return "WAITING";
      case PROCESS_RUNNING: return "RUNNING";
      case PROCESS_UNDEFINED: return "UNDEFINED";
   }
   return 0;
}

int readFile(const char *fileName){
    char text[1000];
    //Read file
    FILE *file = fopen(fileName,"r");
    if (!file)
        return 1;

    while (fgets(text,1000, file)!=NULL)
        //printf("%s",text);
        getProcessData(text);

fclose(file);
    return 0;
  }

//Note: writes one line at a time
int outputData(const char *fileName, int time, int pid, enum process_state oldState, enum process_state newState){
    FILE *file = fopen(fileName,"w");
    if(file == NULL){
      printf("File Exception Error.");
      return 0;
    }
      fprintf(file,"%d %d %s %s",time, pid, getState(oldState), getState(newState));
      fclose(file);
      return 0;
}

int main()
{
  init_list_of_processes();
  readFile("input.txt");

  //testing purposes
  for(int i=0;i<10;i++){
    printf("%i \n",list_of_processes[i].pid);
  }
  outputData("output.txt", 5, 1111, PROCESS_WAITING, PROCESS_RUNNING );
}
