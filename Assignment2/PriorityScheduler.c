#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "assignment2.h"

//predetermined memory for array of processes
#define MEMORY 100
#define QUANTUM_HIGH 1
#define QUANTUM_MED 2
#define QUANTUM_LOW 4

const int max_priority = 3;
const int min_priority = 1;
//tick counter
int tickCount = 0;
//current running process
processP *currentProcess;
//array of processes in the IO waiting state
processIOP *ioProcesses[MEMORY];
//boolean value if process is currently running
int processRunning=false;
//boolean value if a process has processArrived
int processArrived = false;
int tickStart = 0; //start time of current process
//this array acts as the data structure in memory like a PCB with a memory set to 100 processes
processP list_of_processes[MEMORY];
//ready queue which contains queue of processes in ready state
PQueue highPriorityQueue, medPriorityQueue, lowPriorityQueue;
int burstRemaining;
int totalNumProc = 0; //total number of processes
int numProcSuspended = 0; //increment each time a process is SUSPENDED
//metrics:
//Array of processMetrics structs which holds metrics for each process
processMetrics list_of_procMetrics[MEMORY];
//index number assigned to each process to access specific process metrics
int metricsIndex = 0;
FILE *file;

//func prototypes
void setIOWaitTime(int ioFrequency);
int initIOProcesses();
int addIOProcess(processP *ioProc);
void removeIOProcess(int i);
int incrementIOProcesses();
void printIOProcs();
void checkArrivalTime();
void checkProcPriority();
void checkCurrProcPriority();
void calculateTurnAroundTime();

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
void list_add(processP arr[], int pid, int arrivalTime, int totalCPUTime, int ioFrequency, int ioDuration, int priority){
  for(int i=0;i<MEMORY;i++){
    if(arr[i].state == PROCESS_UNDEFINED){
      arr[i].pid = pid;
      arr[i].arrivalTime = arrivalTime;
      arr[i].totalCPUTime = totalCPUTime;
      arr[i].ioFrequency = ioFrequency;
      arr[i].ioDuration = ioDuration;
      arr[i].state = PROCESS_READY;
      list_of_processes[i].ioFrequencyRemaining = ioFrequency;
      if(priority>max_priority){
        arr[i].priority = max_priority;
      }else if(priority<min_priority){
        arr[i].priority = min_priority;
      }else{
        arr[i].priority = priority;
      }
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
    printf("pid: %i , arrival: %i at position %i, priority level: %i \n", list_of_processes[i].pid, list_of_processes[i].arrivalTime, i, list_of_processes[i].priority);
  }
}
printf("------------TEST------------\n");
}
/*
* Helper function to swap position of 2 int
*/
void swap(processP* xp, processP* yp)
{
    processP temp = *xp;
    *xp = *yp;
    *yp = temp;
}
/*
 * Function which sorts the processes read by the input file from arrival times
 * smallest to greatest for each process
*/
void selectionSort(struct processP list_of_processes[MEMORY])
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
 * Function which sorts the processes read by the input file from arrival times
 * smallest to greatest for each process
*/
void prioritySort(struct processP list_of_processes[MEMORY])
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
            if (list_of_processes[j].priority < list_of_processes[min_idx].priority)
                min_idx = j;

        // Swap the found minimum element
        // with the first element
        swap(&list_of_processes[min_idx], &list_of_processes[i]);
    }
}


/*
* Function to iterate through ready queue and find process with highest priority.
* (lowest value).
*/
processP* smallestPriorProcess(PQueue q)
{
    processP* min = NULL;
    PQueue temp = q;
    // Check loop while head not equal to NULL
    while (temp.head != NULL) {
        if(min == NULL){
          min = temp.head->process;
        }
        if (min->priority > temp.head->process->priority){
            min = temp.head->process;
        }
        if(temp.head->next == NULL){
          return min;
        }else{
          temp.head = temp.head->next;
        }
    }
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
  process_data = strtok(NULL,delim);
  int arrivalTime = atoi(process_data);
  process_data = strtok(NULL,delim);
  int totalCPUTime = atoi(process_data);
  process_data = strtok(NULL,delim);
  int ioFrequency = atoi(process_data);
  process_data = strtok(NULL,delim);
  int ioDuration = atoi(process_data);
  process_data = strtok(NULL,delim);
  int priority = atoi(process_data);
  totalNumProc++;
  list_add(list_of_processes, pid, arrivalTime, totalCPUTime, ioFrequency, ioDuration, priority);
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
void enqueue(PQueue *q, processP *p) {
   //create a new link and link the process to the process passed through the argument
  nodep *att = (nodep*) malloc(sizeof(node));
  att->process = p;
  att->next = NULL; //last node of queue
  if(q->head==NULL){
      q->head = att;
  } else{
      nodep *temp = q->head;
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
processP * dequeue(PQueue *q) {
   //create temp node and set reference to head link
   nodep *tempLink = q->head;
   //mark next node as new head
   q->head = q->head->next;
   //return the deleted node
   return tempLink->process;
}
/*
 * Adds a process to a queue based on priority level.
 */
void addToQueue(processP *p){
  int priority = p->priority;
  if(priority >= 3){ //priority is greater or equal to 3 then put in low priority queue
    enqueue(&lowPriorityQueue, p);
  }else if(priority == 2){ //priority level is 2 then put in the medium priority queue
    enqueue(&medPriorityQueue, p);
  }else{ //priority level is 0/1 then put in the high priority queue
    enqueue(&highPriorityQueue, p);
  }
}

/*
 * Remove a process from the front of a queue based on priority level.
 */
processP * removeFromQueue(int priority){
  if(priority < 0){
      printf("Cannot remove from queue\n");
  }else if(priority >= max_priority){ //priority is greater or equal to 3 then remove from low priority queue
    return dequeue(&lowPriorityQueue);
  }else if(priority == 2){ //priority level is 2 then remove from the medium priority queue
    return dequeue(&medPriorityQueue);
  }else{ //priority level is 0/1 then remove from the high priority queue
    return dequeue(&highPriorityQueue);
  }
  return NULL;
}
/*
* Sets the quantum value to the corresponding queue level
*/
void setRemainingBurst(int priority){
  if(priority >= 3){ //priority is greater or equal to 3 then set quantum for new process to low priority quantum
    burstRemaining = QUANTUM_LOW;
  }else if(priority == 2){ //priority level is 2 then set quantum for new process to medium priority quantum
    burstRemaining = QUANTUM_MED;
  }else if(priority>0){ //priority level is 0/1 then set quantum for new process to high priority quantum
    burstRemaining = QUANTUM_HIGH;
  }else{
    printf("wtf are u doing\n"); //test
  }
}

/**
* Checks if queues are empty. If highPriorityQueue has process ready then return 1
* If medPriorityQueue has process ready then return 2. If medPriorityQueue has process
* ready then return 3. Otherwise, return -1.
*/
int checkQueue(){
  if(highPriorityQueue.head !=NULL){
    return 1;
  }else if(medPriorityQueue.head !=NULL){
    return 2;
  }else if(lowPriorityQueue.head !=NULL){
    return 3;
  }else{
    return -1;
  }
}

/*
* Function checks if a process has to arrive yet and if so it is added to ready queue
*/
void checkProcessArrival(){
  for(int i=0; i<MEMORY;i++){
    if(list_of_processes[i].state!=PROCESS_UNDEFINED && list_of_processes[i].arrivalTime == tickCount){
      processP *temp = &list_of_processes[i];
      //printf("Added to ReadyQueue: %i @ tickCount: %i\n", temp->pid, tickCount); testing
      addToQueue(temp);
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
void priorityScheduler(int i){
  printf("Priority Scheduler Simulation # %i  \nState Sequence: \nTIME PID OLDSTATE NEWSTATE PRIORITY\n", i+1);
  //process index in the array
  int processRunning=false; //boolean value if a process is running
  //no processes have arrived  const char* tempOldState; //temp variable used to keep track of processes old states
  const char* tempOldState; //temp variable used to keep track of processes old states

  initIOProcesses(); //initialize the array of processes in IO (waiting state)
  selectionSort(list_of_processes);
  file = fopen("outputFCFS.txt","a+");
  fprintf(file,"Priority Scheduler Simulation # %i  \nState Sequence: \nTIME PID OLDSTATE NEWSTATE PRIORITY\n", i+1);
  fclose(file);
  while(1){
    if(totalNumProc == numProcSuspended){//all processes complete, end simulation
      break;
    }
    if(tickCount==0){
      checkProcessArrival();
    }
    if(processArrived == true){ //case where the only process that has arrived is in IO
      while(checkQueue() == -1){ //while there are no processes in high/med/low queues
        tickCount++;
        incrementIOProcesses();
      }
    }else{//no processes have arrived
      while(!processArrived){
        tickCount++;
        checkProcessArrival();
      }
    }

      if(!processRunning){
        if(checkQueue() > 0){ //if there is a process in a queue
          currentProcess = removeFromQueue(checkQueue()); //remove first process from highest priority queue
          setRemainingBurst(currentProcess->priority);
          //output data: READY => RUNNING
          tempOldState = getState(currentProcess->state);
          currentProcess->state = PROCESS_RUNNING;
          processRunning = true;
          outputData("outputP.txt",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
          printf("%d %d %s %s %i \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state), currentProcess->priority);
          tickStart = tickCount; //tick count the current process begins
        }
      }

      if(currentProcess->ioFrequency ==0){ //if there is no ioFrequency then perform execution until process completes execution
        while(currentProcess->totalCPUTime!=0){
          tickCount++;
          checkProcessArrival();
          currentProcess->totalCPUTime--; //decreases total CPU execution time from current process
          burstRemaining--;
          incrementIOProcesses(); //if a process is in IO then decrement time process in IO
          if(burstRemaining==0){
            if(currentProcess->totalCPUTime!=0){ //if process uses quantum time in a queue and does not execute
              if(currentProcess->priority < 3){ //the process is bumped down a priority
                currentProcess->priority++;
              }
            }
            break;
          }
        }

        //check if current process quantum completed
        if(currentProcess !=NULL){
          if(burstRemaining == 0 && currentProcess->totalCPUTime != 0){ //if burst time is 0 then process shall be preemted
            const char* tempOldState = getState(currentProcess->state); //RUNNING
            currentProcess->state = PROCESS_READY;
            printf("%d %d %s %s %i \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state), currentProcess->priority);
            processRunning = false;
            addToQueue(currentProcess); //put process back in ready queue
            currentProcess = NULL;
          }
        }

        if(processRunning){
          const char* tempOldState = getState(currentProcess->state);
          currentProcess->state = PROCESS_SUSPENDED;
          outputData("outputP.txt",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
          printf("%d %d %s %s %i \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state), currentProcess->priority);
          processRunning = false;
          //calculate metrics
          list_of_procMetrics[currentProcess->processData].finishTime = tickCount;
          calculateTurnAroundTime();
          currentProcess = NULL;
          numProcSuspended++; //counter to track how many processes have finished executing
        }
      }else{
        while(currentProcess->ioFrequencyRemaining>0){ //execute until process requests IO
          tickCount++;
          checkProcessArrival();
          currentProcess->ioFrequencyRemaining--;
          currentProcess->totalCPUTime--; //decreases total CPU execution time from current process
          burstRemaining--;
          incrementIOProcesses(); //if a process is in IO then decrement time process in IO
          if(currentProcess->totalCPUTime==0){ //if a process has finished executing
            const char* tempOldState = getState(currentProcess->state);
            currentProcess->state = PROCESS_SUSPENDED;
            outputData("outputP.txt",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
            printf("%d %d %s %s %i \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state), currentProcess->priority);
            processRunning = false;
            //calculate metrics
            list_of_procMetrics[currentProcess->processData].finishTime = tickCount;
            calculateTurnAroundTime();
            currentProcess = NULL;
            numProcSuspended++; //counter to track how many processes have finished executing
            break;
          }
          if(burstRemaining==0){ //if quantum for current process execution reached
            if(currentProcess->ioFrequencyRemaining > 0){ //case where process has executed max quantum time but not terminated or request IO yet
              if(currentProcess->priority < 3){ //the process is bumped down a priority because it did not finish executing
                currentProcess->priority++;
              }
              tempOldState = getState(currentProcess->state);
              currentProcess->state = PROCESS_READY;
              outputData("outputP.txt",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
              printf("%d %d %s %s %i \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state), currentProcess->priority);
              addToQueue(currentProcess);
              currentProcess = NULL;
              processRunning = false;
            }
            break;
          }
        }

        if(currentProcess !=NULL){ //check if current process quantum execution time completed
          if(burstRemaining == 0 && currentProcess->totalCPUTime != 0 && currentProcess->ioFrequencyRemaining !=0 && currentProcess->ioFrequencyRemaining!=0){ //if burst time is 0 then process shall be preemted
            const char* tempOldState = getState(currentProcess->state); //RUNNING
            currentProcess->state = PROCESS_READY;
            printf("%d %d %s %s %i \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state), currentProcess->priority);
            processRunning = false;
            addToQueue(currentProcess); //put process back in ready queue
            currentProcess = NULL;
          }
        }
      }

        if(currentProcess !=NULL && currentProcess->ioFrequencyRemaining == 0){
          //send process to IO
          tempOldState = getState(currentProcess->state);
          currentProcess->state = PROCESS_WAITING;
          processRunning = false;
          addIOProcess(currentProcess); //add the process to the array of IO
          //send data (RUNNING => WAITING)
          outputData("outputP.txt",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state));
          printf("%d %d %s %s %i \n",tickCount, currentProcess->pid,tempOldState, getState(currentProcess->state), currentProcess->priority);
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
    ioProcesses[i] = (processIOP*) malloc(sizeof(processIO));
    ioProcesses[i]->process = NULL;
  }
  return 0;
}
/*
 * Add a process to the IO waiting Structure
 * Note: This data structure is required because there can be several different processes accessing IO
 * at the same time. Therefore, the easiest way to keep track of the processes in IO is to use an array
 */
int addIOProcess(processP *ioProc){
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
        processIOP *p = ioProcesses[i];
        const char* tempOldState = getState(p->process->state);
        p->process->state= PROCESS_READY;
        outputData("outputP.txt",tickCount, p->process->pid,tempOldState, getState(p->process->state));
        printf("%d %d %s %s %i \n",tickCount, p->process->pid,tempOldState, getState(p->process->state),p->process->priority);
        p->process->ioFrequencyRemaining = p->process->ioFrequency; //reset amount until IO runs again
        addToQueue(p->process);
        removeIOProcess(i); //clears process data at index
      }
    }
  }
  return 0;
}

void printQueue(PQueue q){
  while(q.head->next != NULL){
    printf("%i\n", q.head->process->arrivalTime);
    q.head->next = q.head->next->next;
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
* Priority Queue part c test file : "priorityInput.txt"
* FCFS part d test file : "priorityInputIO.txt"
*/
int main()
{
  //clear file before execution
  FILE *clearFile = fopen("outputP.txt","w");
  fclose(clearFile);
  for(int i=0;i<10;i++){
    //init list of processes
    init_list_of_processes();
    //read input file
    char fileName[22];
    sprintf(fileName,"InputFiles/input%d.txt",i);
    readFile(fileName);
    //run simulation
    priorityScheduler(i);
    calculateMetrics(i);
    resetVariables();
  }
}
