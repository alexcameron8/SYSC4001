/** Structure that holds information for each task */
struct process
{
  //unique identifer for the process_state
  int pid;
  //Initial simulated is 0 and the arrival time can be at 0 or any time value thereafter. The time units to be used are milliseconds.
  int arrivalTime;
  //: it is the total time the process needs to complete (does not include I/O time: only the total time needed in the CPU)
  int totalCPUTime;
  //the processes are assumed to make an input/output with this frequency
  int ioFrequency;
  // this is the duration for the I/O for each of the processes (assumed to be the same for all the I/O operations)
  int ioDuration;
  //current state of the process
  int state;
};

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
  /** Undefined: corresponding process has not been initialized. */
  PROCESS_UNDEFINED
};
