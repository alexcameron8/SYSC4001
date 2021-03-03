#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "assignment2.h"

//predetermined memory for array of processes
#define MEMORY 100
#define QUANTUM 3
//Quantum value (time slice) Assignment 2: 100 ms NOTE:CHANGE
//int quantum = 3;
//tick counter
int tickCount = 0;
//burst time of current process remaining
int burstRemaining = QUANTUM;
//current running process
processRR *currentProcess;
//array of processes in the IO waiting state
processIORR *ioProcesses[MEMORY];
//boolean value if process is currently running
int processRunning=false;
//boolean value if a process has processArrived
int processArrived = false;

int calledThisTick = false;
//start time of current process
int tickStart = 0;
//this array acts as the data structure in memory like a PCB with a memory set to 100 processes
processRR list_of_processes[MEMORY];
//ready queue which contains queue of processes in ready state
RRQueue readyQueue;

//func prototypes
void setIOWaitTime(int ioFrequency);
void checkCurProcCPUTime();
int initIOProcesses();
int addIOProcess(processRR *ioProc);
void removeIOProcess(int i);
int incrementIOProcesses();
void printIOProcs();
void checkArrivalTime();
void checkCurProcBurstTime();
void printQueue();
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
      list_of_processes[i].ioFrequencyRemaining = ioFrequency;
      printf("PID: %i Frequency:%i  remaining: %i\n", list_of_processes[i].pid, list_of_processes[i].ioFrequency, list_of_processes[i].ioFrequencyRemaining); //test
      return;
    }
  }
  printf("There is no more allocated memory.");
}


/*
 * function used for TESTING purposes to view processes in the IO data structure
 */
void printIOProcsArrival(){
int i;
printf("------------TEST------------\n");
for(i=0; i<MEMORY;i++){
  if(list_of_processes[i].state!=PROCESS_UNDEFINED){
    printf("pid: %i , arrival: %i at position %i\n", list_of_processes[i].pid, list_of_processes[i].arrivalTime, i);
  }
}
printf("------------TEST------------\n");
}
/*
* Helper function to swap position of 2 int
*/
void swap(processRR* xp, processRR* yp)
{
    processRR temp = *xp;
    *xp = *yp;
    *yp = temp;
}
/*
 * Function which sorts the processes read by the input file from arrival times
 * smallest to greatest for each process
*/
void selectionSort(struct processRR list_of_processes[MEMORY])
{
    int i, j, min_idx;
    int n = 0;
    while(list_of_processes[n].state != PROCESS_UNDEFINED){
      n++;
    }
    // One by one move boundary of unsorted subarray
    for (i = 0; i < n - 1; i++) {

        // Find the minimum element in unsorted array
        min_idx = i;
        for (j = i + 1; j < n; j++)
            if (list_of_processes[j].arrivalTime < list_of_processes[min_idx].arrivalTime)
                min_idx = j;

        // Swap the found minimum element
        // with the first element
        swap(&list_of_processes[min_idx], &list_of_processes[i]);
    }
}

/*
* Function to iterate through ready queue and find process with lowest arrival time.
*/
processRR* smallestArrProcess(RRQueue q)
{
    processRR* min = NULL;
    RRQueue temp = q;
    // Check loop while head not equal to NULL
    while (temp.head != NULL) {

        // If min is greater then head->data then
        // assign value of head->data to min
        // otherwise node point to next node.
        if(min == NULL){
          min = temp.head->process;
        }
        if (min->arrivalTime > temp.head->process->arrivalTime)
            min = temp.head->process;

        temp.head = temp.head->next;
    }
    printf("pid: %i , arrival: %i\n", min->pid, min->arrivalTime);
    return min;
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
void enqueue(RRQueue *q, processRR *p) {
   //create a new link and link the process to the process passed through the argument
  nodeRR *att = (nodeRR*) malloc(sizeof(node));
  att->process = p;
  att->next = NULL; //last node of queue
  if(q->head==NULL){
      q->head = att;
  } else{
      nodeRR *temp = q->head;
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
processRR * dequeue(RRQueue *q) {
   //create temp node and set reference to head link
   nodeRR *tempLink = q->head;
   //mark next node as new head
   q->head = q->head->next;
   //return the deleted node
   return tempLink->process;
}

/*
* Function checks if a process has to arrive yet and if so it is added to ready queue
*/
void checkProcessArrival(){
  for(int i=0; i<MEMORY;i++){
    if(list_of_processes[i].state!=PROCESS_UNDEFINED && list_of_processes[i].arrivalTime == tickCount){
      processRR *temp = &list_of_processes[i];
    //  printf("Added to ReadyQueue: %i @ tickCount: %i\n", temp->pid, tickCount); testing purposes
      enqueue(&readyQueue,temp);
      processArrived = true;
      calledThisTick = true;
    }
  }
}

/*
 * This function runs the simulation for handling the processes and sending to different states, such as,
 * RUNNING, READY, WAITING and TERMINATED.
 */
 void roundrobin(){
   printf("\nProcess State Sequence: \nTIME PID OLDSTATE NEWSTATE\n");
   //process index in the array
   int processRunning=false; //boolean value if a process is running
   const char* tempOldState; //temp variable used to keep track of processes old states
   int processesComplete = false; //boolean value to end simulation if all processes have finished execution
   int processSuspended = false; //boolean value if a process is suspended

   //ensure that the output file is clear before appending data to it
   FILE *clearFile = fopen("output.txt","w");
   fclose(clearFile);
   initIOProcesses(); //initialize the array of processes in IO (waiting state)
   selectionSort(list_of_processes);
   while(1){
     if(processesComplete == true){ //all processes complete, end simulation
       break;
     }
     if(tickCount==0){
       checkProcessArrival();
     }

     if(processArrived == true){ //case where the only process that has arrived is in IO
       while(readyQueue.head == NULL){
         tickCount++;
         incrementIOProcesses();
       }
     }else{ //no processes have arrived
       while(!processArrived){
         tickCount++;
         checkProcessArrival();
       }
     }

       if(!processRunning){
         if(readyQueue.head!=NULL){ //if ready queue has a process in ready state waiting to run
           currentProcess = dequeue(&readyQueue); //remove first process from readyqueue
  //         printf("PID: %i Frequency remaining: %i\n",currentProcess->pid, currentProcess->ioFrequencyRemaining);
           //output data: READY => RUNNING
           tempOldState = getState(currentProcess->state);
           currentProcess->state = PROCESS_RUNNING;
           processRunning = true;
           outputData("output.txt",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
           printf("%d %d %s %s \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
           tickStart = tickCount; //tick count the current process begins
         }
       }

       if(currentProcess->ioFrequency ==0){ //if there is no ioFrequency then perform execution until process completes execution
         while(currentProcess->totalCPUTime!=0){
           tickCount++;
           checkProcessArrival();
           currentProcess->totalCPUTime--; //decreases total CPU execution time from current process
           burstRemaining--;
  //         printf(" (shouldnt reach here) Frequency remaining: %i\n", currentProcess->ioFrequencyRemaining);
           //printf("%i\n", burstRemaining); test
           incrementIOProcesses(); //if a process is in IO then decrement time process in IO
           if(burstRemaining==0){
             break;
           }
         }
          //check if current process quantum completed
          if(currentProcess !=NULL){
            if(burstRemaining == 0 && currentProcess->totalCPUTime != 0){ //if burst time is 0 then process shall be preemted
              const char* tempOldState = getState(currentProcess->state); //RUNNING
              currentProcess->state = PROCESS_READY;
              printf("%d %d %s %s \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
              processRunning = false;
              enqueue(&readyQueue,currentProcess); //put process back in ready queue
              burstRemaining = QUANTUM; //reset burst remaining
              currentProcess = NULL;
            }
          }

          if(processRunning){
            burstRemaining = QUANTUM;
            const char* tempOldState = getState(currentProcess->state);
            currentProcess->state = PROCESS_SUSPENDED;
            outputData("output.txt",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
            printf("%d %d %s %s \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
            processRunning = false;
            currentProcess = NULL;
            processSuspended = true;
          }
       }else{
         while(currentProcess->ioFrequencyRemaining>0){ //execute until process requests IO
           tickCount++;
           checkProcessArrival();
           currentProcess->ioFrequencyRemaining--;
//           printf("PID: %i Frequency remaining: %i\n",currentProcess->pid, currentProcess->ioFrequencyRemaining);
           currentProcess->totalCPUTime--; //decreases total CPU execution time from current process
           burstRemaining--;
           incrementIOProcesses(); //if a process is in IO then decrement time process in IO
           if(currentProcess->totalCPUTime==0){ //if a process has finished executing
             const char* tempOldState = getState(currentProcess->state);
             currentProcess->state = PROCESS_SUSPENDED;
             outputData("output.txt",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
             printf("%d %d %s %s \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
             processRunning = false;
             currentProcess = NULL;
             processSuspended = true;
             burstRemaining = QUANTUM;
             break;
           }
//           printf("burstRemaining: %i reached?\n", burstRemaining); //test
           if(burstRemaining==0){
             if(currentProcess->ioFrequencyRemaining >0){ //case where process has executed max quantum time but not terminated or request IO yet
               tempOldState = getState(currentProcess->state);
               currentProcess->state = PROCESS_READY;
               outputData("output.txt",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
               printf("%d %d %s %s \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
               enqueue(&readyQueue, currentProcess);
               currentProcess = NULL;
               processRunning = false;
               burstRemaining = QUANTUM;
             }
             break;
           }
         }

         //check if current process quantum completed
         if(currentProcess !=NULL){
           if(burstRemaining == 0 && currentProcess->totalCPUTime != 0){ //if burst time is 0 then process shall be preemted
             const char* tempOldState = getState(currentProcess->state); //RUNNING
             currentProcess->state = PROCESS_READY;
             printf("%d %d %s %s \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
             processRunning = false;
             enqueue(&readyQueue,currentProcess); //put process back in ready queue
             burstRemaining = QUANTUM; //reset burst remaining
             currentProcess = NULL;
           }
         }
       }
         if(currentProcess !=NULL && currentProcess->ioFrequencyRemaining == 0){
//           printf("\t(waiting)PID: %i Frequency remaining: %i\n",currentProcess->pid, currentProcess->ioFrequencyRemaining);
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
    ioProcesses[i] = (processIORR*) malloc(sizeof(processIO));
    ioProcesses[i]->process = NULL;
  }
  return 0;
}
/*
 * Add a process to the IO waiting Structure
 * Note: This data structure is required because there can be several different processes accessing IO
 * at the same time. Therefore, the easiest way to keep track of the processes in IO is to use an array
 */
int addIOProcess(processRR *ioProc){
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
        processIORR *p = ioProcesses[i];
        const char* tempOldState = getState(p->process->state);
        p->process->state= PROCESS_READY;
        outputData("output.txt",tickCount, p->process->pid,tempOldState, getState(p->process->state));
        printf("%d %d %s %s \n",tickCount, p->process->pid,tempOldState, getState(p->process->state));
        p->process->ioFrequencyRemaining = p->process->ioFrequency; //reset amount until IO runs again
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

void printQueue(){
  printf("------------TEST------------\n");
  while(readyQueue.head->next != NULL){
    printf("%i\n", readyQueue.head->process->pid);
    readyQueue.head->next = readyQueue.head->next->next;
  }
  printf("------------TEST------------\n");
}

/**
* Function that checks to make sure that process has not surpassed time splice /
* quantum.
* Note: Run after CheckCurProcTime()
*/
void checkCurProcBurstTime(){
  printf("%i\n", currentProcess ==  NULL);
  if(currentProcess !=NULL){
    if(burstRemaining == 0 && currentProcess->totalCPUTime != 0){ //if burst time is 0 then process shall be preemted
      const char* tempOldState = getState(currentProcess->state); //RUNNING
      currentProcess->state = PROCESS_READY;
      printf("%d %d %s %s \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
      processRunning = false;
      enqueue(&readyQueue,currentProcess); //put process back in ready queue
      burstRemaining = QUANTUM; //reset burst remaining
      currentProcess = NULL;
    }else{
      printf("PID: %i total execution time complete \n", currentProcess->pid);
    }
  }
}
/*
* Round Robin part c test file : "roundRobin.txt"
* FCFS part d test file : "fcfsPartD.txt"
*/
int main()
{
  //init list of processes
  init_list_of_processes();
  //read input file
  readFile("roundRobinIO.txt");
  //readFile("roundRobin.txt");
  //run Round Robin simulation
  roundrobin();

}
