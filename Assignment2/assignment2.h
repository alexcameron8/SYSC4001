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

/** Process information for Priority Scheduler*/
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
