#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "assignment2.h"

//predetermined memory for array of processes
#define MEMORY 100
#define SLOT0 500
#define SLOT1 250
#define SLOT2 150
#define SLOT3 100

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
//start time of current process
int tickStart = 0;
//this array acts as the data structure in memory like a PCB with a memory set to 100 processes
process list_of_processes[MEMORY];
//ready queue which contains queue of processes in ready state
Queue readyQueue;
//total number of processes
int totalNumProc = 0;
//increment each time a process is SUSPENDED
int numProcSuspended = 0;
// array of memorySlot objects
memorySlot list_of_memorySlots[4];
//metrics:
//Array of processMetrics structs which holds metrics for each process
processMetrics list_of_procMetrics[MEMORY];
//index number assigned to each process to access specific process metrics
//also acts as size for array
int metricsIndex = 0;
//total memory allocated
int totalMemAllocated = 0;
//counter for number of memory allocations performed
int numOfAllocations = 0;
//total memory available
int memAvailable = SLOT0 + SLOT1 + SLOT2 + SLOT3;
//pointer to file
FILE *file;

//func prototypes
void setIOWaitTime(int ioFrequency);
void checkCurProcCPUTime();
int initIOProcesses();
int addIOProcess(process *ioProc);
void removeIOProcess(int i);
int incrementIOProcesses();
int allocateMemory();
void deallocateMemory(process* p);
void checkFreeMemory();
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
 * Intializes all memory slot sizes and sets all their occupied flag to false.
*/
void init_list_of_memorySlots(){

  list_of_memorySlots[0].slotSize = SLOT0; //500
  list_of_memorySlots[1].slotSize = SLOT1; //250
  list_of_memorySlots[2].slotSize = SLOT2; //150
  list_of_memorySlots[3].slotSize = SLOT3; //100

  list_of_memorySlots[0].slotID = 0; //500
  list_of_memorySlots[1].slotID = 1; //250
  list_of_memorySlots[2].slotID = 2; //150
  list_of_memorySlots[3].slotID = 3; //100
  for(int i = 0; i < 4; i++){
    list_of_memorySlots[i].occupied = false;
  }
}


/*
 * Add a process to the array of processes and assign the process information
 * (pid,arrival time, total CPU execution time, IO Frequency, IO Duration)
 */
void list_add(int pid, int arrivalTime, int totalCPUTime, int ioFrequency, int ioDuration, int memoryRequired){
  totalNumProc++;
  for(int i=0;i<MEMORY;i++){
    if(list_of_processes[i].state == PROCESS_UNDEFINED){
      list_of_processes[i].pid = pid;
      list_of_processes[i].arrivalTime = arrivalTime;
      list_of_processes[i].totalCPUTime = totalCPUTime;
      list_of_processes[i].ioFrequency = ioFrequency;
      list_of_processes[i].ioDuration = ioDuration;
      list_of_processes[i].state = PROCESS_READY;
      list_of_processes[i].memoryRequired = memoryRequired;
      list_of_processes[i].memorySlotAllocated = 0;
      return;
    }
  }
  printf("There is no more allocated memory.");
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
void arrivalSort(struct process list_of_processes[MEMORY])
{
    int i, j, min_idx;
    int n = 0;
    while(list_of_processes[n].state != PROCESS_UNDEFINED){
      n++;
    }
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
  process_data = strtok(NULL,delim);
  int priority = atoi(process_data); //not used for FCFS
  process_data = strtok(NULL,delim);
  int memoryRequired = atoi(process_data);

  list_add(pid, arrivalTime, totalCPUTime, ioFrequency, ioDuration, memoryRequired);
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
* It outputs data for a state transition each time a process changes states.
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
void fcfsMem(int i){
  printf("Memory Management with FCFS Simulation # %i \nState Sequence: \nTIME PID OLDSTATE NEWSTATE\n",i+1);
  //process index in the array
  int processRunning=false; //boolean value if a process is running
  const char* tempOldState; //temp variable used to keep track of processes old states

  file = fopen("outputMM.txt","a+");
  fprintf(file,"Memory Management First Come First Serve Scheduler Simulation # %i  \nState Sequence: \nTIME PID OLDSTATE NEWSTATE\n", i+1);
  fclose(file);

  initIOProcesses(); //initialize the array of processes in IO (waiting state)
  arrivalSort(list_of_processes);
  while(1){
    if(totalNumProc == numProcSuspended){ //all processes complete, end simulation
      break;
    }
    if(tickCount==0){
      checkProcessArrival();
    }

    if(processArrived == true){ //case where the only process that has arrived is in IO
      while(readyQueue.head == NULL){//while there are no processes in queue
        tickCount++;
        incrementIOProcesses();
        checkProcessArrival();
      }
    }else{ //no processes have arrived
      while(!processArrived){
        tickCount++;
        checkProcessArrival();
      }
    }

      if(!processRunning){ //no process is currently running then assign a process to run
        if(readyQueue.head!=NULL){ //if ready queue has a process in ready state waiting to run
            currentProcess = dequeue(&readyQueue); //remove first process from readyqueue
            if(allocateMemory()){ //if a process can be allocated =>
            //output data: READY => RUNNING
            tempOldState = getState(currentProcess->state);
            currentProcess->state = PROCESS_RUNNING;
            processRunning = true;
            outputData("outputMM.txt",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
            printf("%d %d %s %s \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
            tickStart = tickCount; //tick count the current process begins
          }else{ //memory could not be allocated => move to back of readyQueue
            currentProcess = dequeue(&readyQueue);
            enqueue(&readyQueue,currentProcess);
            currentProcess=NULL;
          }
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
          deallocateMemory(currentProcess); //deallocate memory

          outputData("outputMM.txt",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
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
          checkProcessArrival(); //check if process has arrived
          currentProcess->totalCPUTime = currentProcess->totalCPUTime - 1; //decreases total CPU execution time from current process
          incrementIOProcesses(); //if a process is in IO then decrement time process in IO
          if(currentProcess->totalCPUTime==0){ //if a process has finished executing
            const char* tempOldState = getState(currentProcess->state);
            currentProcess->state = PROCESS_SUSPENDED;
            deallocateMemory(currentProcess); //deallocate memory
            outputData("outputMM.txt",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
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
        outputData("outputMM.txt",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
        printf("%d %d %s %s \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
      }
    }
  }
/**
* Function which  attempts to allocate memory to the currentProcess if a partition is available
*/
int allocateMemory(){
  int memAllocated;
  if(currentProcess->memoryRequired < SLOT0){ //ensure that process is less than max partition slot
    for(int i = 0; i < 4; i++){
      if(list_of_memorySlots[i].occupied == false){
        if(list_of_memorySlots[i].slotSize >= currentProcess->memoryRequired){
          currentProcess->memorySlotAllocated = list_of_memorySlots[i].slotID;
          list_of_memorySlots[i].occupied = true;
          memAllocated = list_of_memorySlots[i].slotSize;
          printf("\t---Memory Allocation Metrics---\n");
          printf("\tMemory Allocated: %iK PID: %i \n", list_of_memorySlots[i].slotSize,currentProcess->pid);
          //metric calculations
          totalMemAllocated += memAllocated;
          numOfAllocations++;
          printf("\tTotal Memory Allocated: %ik\n", totalMemAllocated);
          checkFreeMemory();
          memAvailable -= memAllocated;
          printf("\tAvailable Memory:%ik\n", memAvailable);
          printf("\t-------------------------------\n");
          return true;
        }
      }
    }
  }else{ //process will never be able to run because of memory requirements
    const char* tempOldState = getState(currentProcess->state);
    currentProcess->state = PROCESS_SUSPENDED;
    outputData("outputMM.txt",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
    printf("%d %d %s %s \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
    processRunning = false;
    //calculate metrics
    list_of_procMetrics[currentProcess->processData].finishTime = tickCount;
    calculateTurnAroundTime();
    printf("Process %i could can never be allocated to main memory, suspending process.\n", currentProcess->pid);
    currentProcess = NULL;
    numProcSuspended++; //counter to track how many processes have finished executing
    return false;
  }
  printf("Process %i could not be allocated to memory\n", currentProcess->pid);
  return false;


}

/**
* Function which takes a process as an argument and deallocates the memory
* previously allocated
*/
void deallocateMemory(process *p){
  int memDeallocated;
  int tempSlotID = p->memorySlotAllocated;
  p->memorySlotAllocated = 0;

  for(int i = 0; i < 4; i++){
    if(list_of_memorySlots[i].slotID == tempSlotID){
      list_of_memorySlots[i].occupied = false;
      memDeallocated = list_of_memorySlots[i].slotSize;
    }
  }
  //metrics
  memAvailable += memDeallocated;
}


/**
* Function which checks which memory slots are free (metrics)
*/
void checkFreeMemory(){
  printf("\tFree Slots: ");
  for(int i=0; i < 4; i++){
    if(list_of_memorySlots[i].occupied == false){
      printf("%i ", list_of_memorySlots[i].slotSize);
    }
  }
  printf("\n");
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
        outputData("outputMM.txt",tickCount, p->process->pid,tempOldState, getState(p->process->state));
        printf("%d %d %s %s \n",tickCount, p->process->pid,tempOldState, getState(p->process->state));
        enqueue(&readyQueue, p->process); //IO has complete add process to ready queue
        deallocateMemory(p->process);
        removeIOProcess(i); //clears process data at index
      }
    }
  }
  return 0;
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
  return totalWaitTime / (metricsIndex);
}
/**
* Throughput: num of processes/ total simulation time
*/
float calculateThroughput(){
  return (metricsIndex)/(double)tickCount;
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
  return totalTurnAroundTime / (metricsIndex);
}
/**
* Function which calculates the average memory used per allocation
*/
float calculateMemory(){
  return (totalMemAllocated)/(double)numOfAllocations;
}


void calculateMetrics(int i){
  printf("--------------------------------------------\n");
  printf("METRIC CALCULATIONS SIMULATION: %i\n", i+1);
  printf("Throughput: %f\n",calculateThroughput());
  printf("Average Turnaround Time: %f ticks\n",calculateAverageTurnAroundTime());
  printf("Average Wait Time: %f ticks\n",calculateAverageWaitTime());
  printf("Total Memory Allocated: %ik\n", totalMemAllocated);
  printf("Number of Times Memory Allocated: %i\n", numOfAllocations);
  printf("Average Memory Allocated: %fk\n", calculateMemory());
  printf("--------------------------------------------\n");
}
/*
* Function that resets all variables for the next simulation
*/
void resetVariables(){
  tickCount = 0;
  totalNumProc = 0;
  numProcSuspended = 0;
  memset(list_of_processes,0,sizeof(list_of_processes));
  memset(list_of_procMetrics, 0, sizeof(list_of_procMetrics)); //reset list_of_procMetrics
  metricsIndex = 0;
  totalMemAllocated = 0;
  numOfAllocations = 0;
  memAvailable = SLOT0 + SLOT1 + SLOT2 + SLOT3;
}


/*
* FCFS scheduler that implements Memory Management by utilizing a first fit picking
* algorithm.
*/
int main()
{
  //clear file before execution
  FILE *clearFile = fopen("outputMM.txt","w");
  fclose(clearFile);
  for(int i=0;i<10;i++){
    //init list of processes
    init_list_of_processes();
    //init list of memory slots
    init_list_of_memorySlots();
    //read input file
    char fileName[22];
    sprintf(fileName,"InputFiles/input%d.txt",i);
    readFile(fileName);
    //run simulation
    fcfsMem(i);
    calculateMetrics(i);
    resetVariables();
  }
}
