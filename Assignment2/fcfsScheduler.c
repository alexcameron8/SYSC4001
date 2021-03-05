#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "assignment2.h"

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
//boolean value if a process has processArrived
int processArrived = false;
int tickStart = 0; //start time of current process
//this array acts as the data structure in memory like a PCB with a memory set to 100 processes
process list_of_processes[MEMORY];
//ready queue which contains queue of processes in ready state
Queue readyQueue;
int totalNumProc = 0; //total number of processes
int numProcSuspended = 0; //increment each time a process is SUSPENDED
//metrics:
//Array of processMetrics structs which holds metrics for each process
processMetrics list_of_procMetrics[MEMORY];
//index number assigned to each process to access specific process metrics
//also acts as size for array
int metricsIndex = 0;
FILE *file;

//func prototypes
void setIOWaitTime(int ioFrequency);
void checkCurProcCPUTime();
int initIOProcesses();
int addIOProcess(process *ioProc);
void removeIOProcess(int i);
int incrementIOProcesses();
void printIOProcs();
void checkArrivalTime();
void calculateTurnAroundTime();
float calculateAverageWaitTime();
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
  totalNumProc++;
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
void swap(process* xp, process* yp)
{
    process temp = *xp;
    *xp = *yp;
    *yp = temp;
}
/*
 * Function which sorts the processes read by the input file from arrival times
 * smallest to greatest for each process
*/
void selectionSort(struct process list_of_processes[MEMORY])
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
 * This function parses a line from the input file and splits the information
 * and adds the process information to the list of processes.
 */
void getProcessData(char *processData){
  const char delim[2] = " ";
  char *process_data;
  process_data = strtok(processData,delim);
  int pid = atoi(process_data);
  process_data = strtok(NULL,delim);
  int arrivalTime = atoi(process_data);
  process_data = strtok(NULL,delim);
  int totalCPUTime = atoi(process_data);
  process_data = strtok(NULL,delim);
  int ioFrequency = atoi(process_data);
  process_data = strtok(NULL,delim);
  int ioDuration = atoi(process_data);
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
    file = fopen(fileName,"r");
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
    file = fopen(fileName,"a+");
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
* Function checks if a process has to arrive yet and if so it is added to ready queue
*/
void checkProcessArrival(){
  for(int i=0; i<MEMORY;i++){
    if(list_of_processes[i].state!=PROCESS_UNDEFINED && list_of_processes[i].arrivalTime == tickCount){
      process *temp = &list_of_processes[i];
      enqueue(&readyQueue,temp);
      processArrived = true;
      //initialize process metrics
      temp->processData = metricsIndex;
      list_of_procMetrics[temp->processData].arrivalTime = tickCount;
      list_of_procMetrics[temp->processData].waitingTime = 0;
      list_of_procMetrics[temp->processData].turnaroundTime = 0;
      list_of_procMetrics[temp->processData].totalCPUTime = temp->totalCPUTime;
      metricsIndex++;
    }
  }
}

/*
 * This function runs the simulation for handling the processes and sending to different states, such as,
 * RUNNING, READY, WAITING and TERMINATED.
 */
void fcfs(int i){
  printf("First Come First Serve Scheduler Simulation # %i  \nState Sequence: \nTIME PID OLDSTATE NEWSTATE\n", i+1);
  //process index in the array
  int processRunning=false; //boolean value if a process is running
  const char* tempOldState; //temp variable used to keep track of processes old states
  file = fopen("outputFCFS.txt","a+");
  fprintf(file,"First Come First Serve Scheduler Simulation # %i  \nState Sequence: \nTIME PID OLDSTATE NEWSTATE\n", i+1);
  fclose(file);
  //ensure that the output file is clear before appending data to it
  initIOProcesses(); //initialize the array of processes in IO (waiting state)
  selectionSort(list_of_processes);
  while(1){
    if(totalNumProc == numProcSuspended){ //all processes complete, end simulation
      break;
    }
    if(tickCount==0){
      checkProcessArrival();
    }

    if(processArrived == true){
      while(readyQueue.head == NULL){
        tickCount++;
        incrementIOProcesses();
      }
    }else{
      while(!processArrived){
        tickCount++;
        checkProcessArrival();
      }
    }

      if(!processRunning){ //no process is currently running then assign a process to run
        if(readyQueue.head!=NULL){ //if ready queue has a process in ready state waiting to run
          currentProcess = dequeue(&readyQueue); //remove first process from readyqueue
          //output data: READY => RUNNING
          tempOldState = getState(currentProcess->state);
          currentProcess->state = PROCESS_RUNNING;
          processRunning = true;
          outputData("outputFCFS.txt",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
          printf("%d %d %s %s \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
          tickStart = tickCount; //tick count the current process begins
        }
      }

      if(currentProcess->ioFrequency == 0){ //if there is no ioFrequency then perform execution until process completes execution
        while(currentProcess->totalCPUTime!=0){
          tickCount++;
          checkProcessArrival();
          currentProcess->totalCPUTime = currentProcess->totalCPUTime - 1; //decreases total CPU execution time from current process
          incrementIOProcesses(); //if a process is in IO then decrement time process in IO
        }
          const char* tempOldState = getState(currentProcess->state);
          currentProcess->state = PROCESS_SUSPENDED;
          outputData("outputFCFS.txt",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
          printf("%d %d %s %s \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
          processRunning = false;
          //calculate metrics
          list_of_procMetrics[currentProcess->processData].finishTime = tickCount;
          calculateTurnAroundTime();
          currentProcess = NULL;
          numProcSuspended++; //counter to track how many processes have finished executing
      }else{ //there is ioFrequency
        while(tickCount < tickStart + currentProcess->ioFrequency){ //execute until process requests IO
          tickCount++;
          checkProcessArrival();
          currentProcess->totalCPUTime = currentProcess->totalCPUTime - 1; //decreases total CPU execution time from current process
          incrementIOProcesses(); //if a process is in IO then decrement time process in IO
          if(currentProcess->totalCPUTime==0){ //if a process has finished executing
            const char* tempOldState = getState(currentProcess->state);
            currentProcess->state = PROCESS_SUSPENDED;
            outputData("outputFCFS.txt",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
            printf("%d %d %s %s \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
            processRunning = false;
            //calculate metrics
            list_of_procMetrics[currentProcess->processData].finishTime = tickCount;
            calculateTurnAroundTime();
            currentProcess = NULL;
            numProcSuspended++; //counter to track how many processes have finished executing
            break;
          }
        }
      }
        if(currentProcess !=NULL){ //if process requires IO
        //send process to IO
        tempOldState = getState(currentProcess->state);
        currentProcess->state = PROCESS_WAITING;
        processRunning = false;
        addIOProcess(currentProcess); //add the process to the array of IO
        //send data (RUNNING => WAITING)
        outputData("outputFCFS.txt",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
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
        outputData("outputFCFS.txt",tickCount, p->process->pid,tempOldState, getState(p->process->state));
        printf("%d %d %s %s \n",tickCount, p->process->pid,tempOldState, getState(p->process->state));
        enqueue(&readyQueue, p->process); //IO has complete add process to ready queue
        removeIOProcess(i); //clears process data at index
      }
    }
  }
  return 0;
}

void printQueue(){
  while(readyQueue.head->next != NULL){
    printf("%i\n", readyQueue.head->process->arrivalTime);
    readyQueue.head->next = readyQueue.head->next->next;
  }
}
/** Metric calculations */
/**
* Process wait time: Wait time = turnaroundTime - burst time
*/
void calculateProcessWaitTime(){
  for(int i=0; i<metricsIndex; i++){
    list_of_procMetrics[i].waitingTime=list_of_procMetrics[i].turnaroundTime - list_of_procMetrics[i].totalCPUTime;
  }
}
/**
* Average Process wait time: Sum of all processes wait times / num processes
*/
float calculateAverageWaitTime(){
  calculateProcessWaitTime();
  int totalWaitTime = 0;
  for(int i=0;i<=metricsIndex;i++){
    totalWaitTime = totalWaitTime + list_of_procMetrics[i].waitingTime;
  }
  return totalWaitTime / (metricsIndex + 1);
}
/**
* Throughput: num of processes/ total simulation time
*/
float calculateThroughput(){
  return (metricsIndex+1)/(double)tickCount;
}
/**
* Turnaroundtime: for each process, turnaroundTime = process completion time - arrival time
*/
void calculateTurnAroundTime(){
  list_of_procMetrics[currentProcess->processData].turnaroundTime = list_of_procMetrics[currentProcess->processData].finishTime - list_of_procMetrics[currentProcess->processData].arrivalTime;
}
/**
* Average Turnaroundtime: SUM of all turnaround times / number of processes
*/
float calculateAverageTurnAroundTime(){
  int totalTurnAroundTime = 0;
  for(int i=0;i<=metricsIndex;i++){
    totalTurnAroundTime = totalTurnAroundTime + list_of_procMetrics[i].turnaroundTime;
  }
  return totalTurnAroundTime / (metricsIndex + 1);
}


void calculateMetrics(int i){
  printf("--------------------------------------------\n");
  printf("METRIC CALCULATIONS SIMULATION: %i\n", i+1);
  printf("Throughput: %f\n",calculateThroughput());
  printf("Average Turnaround Time: %f\n",calculateAverageTurnAroundTime());
  printf("Average Wait Time: %f\n",calculateAverageWaitTime());
  printf("--------------------------------------------\n");
}

void resetVariables(){
  tickCount = 0;
  totalNumProc = 0; //total number of processes
  numProcSuspended = 0; //increment each time a process is SUSPENDED
  memset(list_of_processes,0,sizeof(list_of_processes));
  memset(list_of_procMetrics, 0, sizeof(list_of_procMetrics)); //reset list_of_procMetrics
  //index number assigned to each process to access specific process metrics
  metricsIndex = 0;
}

/*
* FCFS part c test file : "fcfsPartC.txt"
* FCFS part d test file : "fcfsPartD.txt"
*/
int main()
{
  //clear file before execution
  FILE *clearFile = fopen("outputFCFS.txt","w");
  fclose(clearFile);
  for(int i=0;i<10;i++){
    //init list of processes
    init_list_of_processes();
    //read input files 0-9
    char fileName[22];
    sprintf(fileName,"InputFiles/input%d.txt",i);
    readFile(fileName);
    //run simulation
    fcfs(i);
    calculateMetrics(i);
    resetVariables();
  }
}
