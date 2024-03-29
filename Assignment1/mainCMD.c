#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "assignment1.h"

//predetermined memory for array of processes
#define MEMORY 100

//tick counter
int tickCount = 0;
//current running process
process *currentProcess;
//array of processes in the IO waiting state
processIO *ioProcesses[MEMORY];
//boolean value if process is currently running
int processRunning=false;
//this array acts as the data structure in memory like a PCB with a memory set to 100 processes
process list_of_processes[MEMORY];
//ready queue which contains queue of processes in ready state
Queue readyQueue;

//func prototypes
void setIOWaitTime(int ioFrequency);
void checkCurProcCPUTime();
int initIOProcesses();
int addIOProcess(process *ioProc);
void removeIOProcess(int i);
int incrementIOProcesses();
void printIOProcs();

/*
 * Initialize all indexes of the list of processes (PCB) to be undefined.
 */
void init_list_of_processes(){
//initializes all processes in memory to be Undefined.
  for(int i=0; i<MEMORY; i++){
    list_of_processes[i].state= PROCESS_UNDEFINED;
  }
}
/*
 * Add a process to the array of processes and assign the process information
 * (pid,arrival time, total CPU execution time, IO Frequency, IO Duration)
 */
void list_add(int pid, int arrivalTime, int totalCPUTime, int ioFrequency, int ioDuration){
  for(int i=0;i<MEMORY;i++){
    if(list_of_processes[i].state == PROCESS_UNDEFINED){
      list_of_processes[i].pid = pid;
      list_of_processes[i].arrivalTime = arrivalTime;
      list_of_processes[i].totalCPUTime = totalCPUTime;
      list_of_processes[i].ioFrequency = ioFrequency;
      list_of_processes[i].ioDuration = ioDuration;
      list_of_processes[i].state = PROCESS_READY;
      return;
    }
  }
  printf("There is no more allocated memory.");
}

/*
 * This function parses a line from the input file and splits the information
 * and adds the process information to the list of processes.
 */
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
/*
 * Getter function to convert a process_state enum type to a char* to be able
 *  to print the states to the output.
 */
const char* getState(enum process_state state)
{
   switch (state)
   {
      case PROCESS_SUSPENDED: return "SUSPENDED";
      case PROCESS_STOPPED: return "STOPPED";
      case PROCESS_WAITING: return "WAITING";
      case PROCESS_RUNNING: return "RUNNING";
      case PROCESS_UNDEFINED: return "UNDEFINED";
      case PROCESS_READY: return "READY";
   }
   return 0;
}
/*
 * The readFile function takes a file name as a parameter and opens such file. This function
 *  calls the getProcessData() function to convert the input from the .txt file to a process struct.
 */
int readFile(const char *fileName){
    char text[1000];
    //Read file
    FILE *file = fopen(fileName,"r");
    if (!file)
        return 1;

    while (fgets(text,1000, file)!=NULL)
        getProcessData(text);

fclose(file);
    return 0;
  }
  /*
   * This function outputs data and adds to the output file each time this function is called
   *  It outputs data for a state transition each time a process changes states.
   */
int outputData(const char *fileName, int time, int pid, const char *oldState, const char * newState){
    FILE *file = fopen(fileName,"a+");
    if(file == NULL){
      printf("File Exception Error.");
      return 0;
    }
      fprintf(file,"%d %d %s %s\n",time, pid, oldState, newState);
      fclose(file);
      return 0;
}
/*
 * Adds a process to the queue.
 */
void enqueue(Queue *q, process *p) {
   //create a new link and link the process to the process passed through the argument
  node *att = (node*) malloc(sizeof(node));
  att->process = p;
  att->next = NULL; //last node of queue
  if(q->head==NULL){
      q->head = att;
  } else{
      node *temp = q->head;
      //point head to new first node
      while(temp->next!=NULL){
         temp=temp->next;
      }
      temp->next = att;
      temp=att;
  }
}

/*
 * Remove a process to the queue.
 */
process * dequeue(Queue *q) {
   //create temp node and set reference to head link
   node *tempLink = q->head;
   //mark next node as new head
   q->head = q->head->next;
   //return the deleted node
   return tempLink->process;
}
/*
 * This function runs the simulation for handling the processes and sending to different states, such as,
 * RUNNING, READY, WAITING and TERMINATED.
 */
void testProcess(){
  printf("\nProcess State Sequence: \nTIME PID OLDSTATE NEWSTATE\n");
  //process index in the array
  int i=0;
  int processRunning=false; //boolean value if a process is running
  process *currentProcess; //pointer to the current process running
  const char* tempOldState; //temp variable used to keep track of processes old states
  int tickStart; //start time of current process
  int processesComplete = false; //boolean value to end simulation if all processes have finished execution
  int processSuspended = false; //boolean value if a process is suspended

  //ensure that the output file is clear before appending data to it
  FILE *clearFile = fopen("output.txt","w");
  fclose(clearFile);

  initIOProcesses(); //initialize the array of processes in IO (waiting state)
  while(1){
    if(processesComplete == true){ //all processes complete, end simulation
      break;
    }
    while(list_of_processes[i].state != PROCESS_UNDEFINED){
      process *temp = &list_of_processes[i];
      enqueue(&readyQueue, temp); //add all processes in memory to the readyQueue
      i++;
    }

      //if the arrival time of process is less than total ticks, wait for first process arrival time
      if(!processSuspended){
        while(tickCount< readyQueue.head->process->arrivalTime ){
          tickCount++; //increase ticks
          incrementIOProcesses(); //if a process is in IO then decrement time process in IO
          checkCurProcCPUTime(); //check if the current process has finished executing
        }
      }

      if(!processRunning){
        if(readyQueue.head!=NULL){ //if ready queue has a process in ready state waiting to run
          currentProcess = dequeue(&readyQueue); //remove first process from readyqueue
          //output data: READY => RUNNING
          tempOldState = getState(currentProcess->state);
          currentProcess->state = PROCESS_RUNNING;
          processRunning = true;
          outputData("output.txt",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
          printf("%d %d %s %s \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
          tickStart = tickCount; //tick count the current process begins
        }
      }

        while(tickCount < tickStart + currentProcess->ioFrequency){ //execute until process requests IO
          tickCount++;
          currentProcess->totalCPUTime = currentProcess->totalCPUTime - 1; //decreases total CPU execution time from current process
          incrementIOProcesses(); //if a process is in IO then decrement time process in IO
          if(currentProcess->totalCPUTime==0){ //if a process has finished executing
            const char* tempOldState = getState(currentProcess->state);
            currentProcess->state = PROCESS_SUSPENDED;
            outputData("output.txt",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
            printf("%d %d %s %s \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
            processRunning = false;
            currentProcess = NULL;
            processesComplete = true;
            processSuspended = true;
            break;
          }
        }

        if(currentProcess !=NULL){
        //send process to IO
        tempOldState = getState(currentProcess->state);
        currentProcess->state = PROCESS_WAITING;
        processRunning = false;
        addIOProcess(currentProcess); //add the process to the array of IO
        //send data (RUNNING => WAITING)
        outputData("output.txt",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
        printf("%d %d %s %s \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
      }
    }
  }
  /*
   * function used for TESTING purposes to view processes in the IO data structure
   */
void printIOProcs(){
  int i;
  printf("test\n");
  for(i=0; i<MEMORY;i++){
    if(ioProcesses[i]->process!=NULL){
      printf("%i\n", ioProcesses[i]->process->pid);
    }
  }
}
/*
 * Initializes the array of processes currently in IO waiting state
 */
int initIOProcesses(){
  int i;
  for(i=0;i<MEMORY;i++){
    ioProcesses[i] = (processIO*) malloc(sizeof(processIO));
    ioProcesses[i]->process = NULL;
  }
  return 0;
}
/*
 * Add a process to the IO waiting Structure
 * Note: This data structure is required because there can be several different processes accessing IO
 * at the same time. Therefore, the easiest way to keep track of the processes in IO is to use an array
 */
int addIOProcess(process *ioProc){
  int i;
  for(i=0;i<MEMORY;i++){
    if(ioProcesses[i]->process == NULL){
      ioProcesses[i]->process = ioProc;
      //the ioDuration will be decremented each tick to keep track of how long process is in IO
      ioProcesses[i]->ioDuration= ioProc->ioDuration;
      return 0;
    }
  }
  return -1;
}

/*
 * Remove a process from the IO array once complete waiting duration with specified index
 */
void removeIOProcess(int i){
  int j;
  for(j=0;i<MEMORY;i++){
    if(j==i){
      ioProcesses[i]->process = NULL;
    }
  }
}
/*
 * This method will update each process in IO each time a tick is increased to keep track
 * of how long a process has been in IO
 */
int incrementIOProcesses(){
  int i;
  for(i=0;i<MEMORY;i++){
    if(ioProcesses[i]->process != NULL && ioProcesses[i]->process->state == PROCESS_WAITING){
      ioProcesses[i]->ioDuration--; //decrement ioDuration by 1 tick
      if(ioProcesses[i]->ioDuration == 0){ //check if IO for process has complete
        processIO *p = ioProcesses[i];
        const char* tempOldState = getState(p->process->state);
        p->process->state= PROCESS_READY;
        outputData("output.txt",tickCount, p->process->pid,tempOldState, getState(p->process->state));
        printf("%d %d %s %s \n",tickCount, p->process->pid,tempOldState, getState(p->process->state));
        enqueue(&readyQueue, p->process); //IO has complete add process to ready queue
        removeIOProcess(i); //clears process data at index
      }
    }
  }
  return 0;
}
/*
 * Check if the current process has finished executing
 */
void checkCurProcCPUTime(){
  if(currentProcess!= NULL && currentProcess->totalCPUTime==0){
    const char* tempOldState = getState(currentProcess->state);
    currentProcess->state = PROCESS_SUSPENDED;
    printf("%d %d %s %s \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
    processRunning = false;
  }
}

int main(int argc, char* argv[])
{
  init_list_of_processes();
  readFile(argv[1]);

  testProcess();
}
