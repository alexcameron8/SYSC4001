/** Definitions for different process states. */
enum process_state {

  /** Suspended: process is ready to run, but is not yet the highest priority process */
  PROCESS_SUSPENDED,
  /** Stopped: corresponding process is not scheduled to run (no start event, i.e., period expiration, yet) */
  PROCESS_STOPPED,
  /** Waiting: corresponding processis waiting to execute. */
  PROCESS_WAITING,
  /** Running: currently executing process */
  PROCESS_RUNNING,
  /** Running: currently executing process */
  PROCESS_READY,
  /** Undefined: corresponding process has not been initialized. */
  PROCESS_UNDEFINED
};

/**First Come First Server Scheduler*/

/** Structure that holds information for each task */
typedef struct process
{
  //unique identifer for the process_state
  int pid;
  //Initial simulated is 0 and the arrival time can be at 0 or any time value thereafter. The time units to be used are milliseconds.
  int arrivalTime;
  // it is the total time the process needs to complete (does not include I/O time: only the total time needed in the CPU)
  int totalCPUTime;
  //the processes are assumed to make an input/output with this frequency
  int ioFrequency;
  // this is the duration for the I/O for each of the processes (assumed to be the same for all the I/O operations)
  int ioDuration;
  //current state of the process
  enum process_state state;
  //index in the array to access this processes data/metrics
  int processData;
  // amount of memory required to execute the process
  int memoryRequired;
  // slot ID of allocated memory slot
  int memorySlotAllocated;
} process;

/**Structure that holds info for a process and a duration which will be changed each tick*/
typedef struct processIO
{
  process* process;
  int ioDuration;
} processIO;

//structs for Queue setup
typedef struct Node {
  process *process;
  struct Node *next;
}node;

typedef struct Queue{
  node *head;
  node *current;
}Queue;

/**Priority Scheduler*/

/** Structure that holds information for each task */
typedef struct processP
{
  //unique identifer for the process_state
  int pid;
  //Initial simulated is 0 and the arrival time can be at 0 or any time value thereafter. The time units to be used are milliseconds.
  int arrivalTime;
  // it is the total time the process needs to complete (does not include I/O time: only the total time needed in the CPU)
  int totalCPUTime;
  //the processes are assumed to make an input/output with this frequency
  int ioFrequency;
  // this is the duration for the I/O for each of the processes (assumed to be the same for all the I/O operations)
  int ioDuration;
  //current state of the process
  enum process_state state;
  //priority of the process
  int priority;
} processP;

/**Structure that holds info for a process and a duration which will be changed each tick*/
typedef struct processIOP
{
  processP* process;
  int ioDuration;
} processIOP;

//structs for Queue setup
typedef struct NodeP {
  processP *process;
  struct NodeP *next;
}nodep;

typedef struct PQueue{
  nodep *head;
  nodep *current;
}PQueue;

/**Round Robin Scheduler*/
/**Structure which holds a process and manages how much allocated CPU time the Round Robin Quantum allows */
typedef struct processRR
{
  //unique identifer for the process_state
  int pid;
  //Initial simulated is 0 and the arrival time can be at 0 or any time value thereafter. The time units to be used are milliseconds.
  int arrivalTime;
  // it is the total time the process needs to complete (does not include I/O time: only the total time needed in the CPU)
  int totalCPUTime;
  //the processes are assumed to make an input/output with this frequency
  int ioFrequency;
  // this is the duration for the I/O for each of the processes (assumed to be the same for all the I/O operations)
  int ioDuration;
  //current state of the process
  enum process_state state;
  //amount of CPU execution time left until IO required
  int ioFrequencyRemaining;
  //index in the array to access this processes data/metrics
  int processData;
} processRR;

//structs for Queue setup
typedef struct NodeRR {
  processRR *process;
  struct NodeRR *next;
}nodeRR;

typedef struct RRQueue{
  nodeRR *head;
  nodeRR *current;
}RRQueue;

/**Structure that holds info for a process and a duration which will be changed each tick*/
typedef struct processIORR
{
  processRR* process;
  int ioDuration;
} processIORR;

/**Metrics to be computed for part 2 b) for each process*/

typedef struct processMetrics
{
  int pid;
  int throughput;
  int avgTurnaroundTime;
  int avgWaitingTime;
  //time between 2 IO Operations
  int avgResponseTime;
  //time when process finishes
  int finishTime;
  int arrivalTime;
  int timeInIO;

} processMetrics;

typedef struct memorySlot
{
  int slotID;
  int slotSize;
  bool occupied;
} memorySlot;
