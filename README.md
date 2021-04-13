# SYSC4001 
## About
These are 3 assignments for SYSC4001 course taken with Professor Gabriel Wainer in Winter semester 2021. Th README's for the 3 assignments done throughout this course are provided below.






# SYSC 4001 - Assignment 1

**Date:** 2020-02-04

**Group members:**
- Marko Majkic, 101109409  
- Alex Cameron, 101114698

**IMPORTANT NOTE:** The **only** difference between main.c and mainCMD.c is that mainCMD.c accepts command line arugments in order to run different input files without changing the source code.

## Running batch files

The batch files are individually linked to the two input files (input1.txt, input2.txt), running **test1.bat** will execute the program with **input1.txt**, and running **test2.bat** will execute the program with **input2.txt**. 

Once the files are executed, the output will be shown in the terminal, and can also be seen in the **output.txt** file found in the 'Assignment1_Part2' directory. (Note: the output in **output.txt** will be overwritten any time a .bat or .exe is run)

## Running the program with custom input files

Running the program with custom input files can be done in one of two ways:

-   Put the custom input file (in .txt format) in the 'Assignment1_Part2' directory, then from the command prompt, cd into the 'Assignment1_Part2' folder and run the command **cmd /k mainCMD.exe [custom file name].txt** without the brackets. The output should appear in the command prompt as well as in the **output.txt** file found in the 'Assignment1_Part2' directory.
-   Paste the contents of the custom input file into one of the two **input.txt** files and run the corresponding batch file.

## Running the program in an optimized IDE

There are two .c files in the 'Assignment1_Part2' directory. **mainCMD.c** is the .c file that allows for command line arguments (for the batch files and custom files), and the **main.c** file is the original .c file which can be run from an IDE and does not require arguments to be passed to it (automatically takes **input2.txt** but can be changed to accept any input file within the main function).

## Final deliverables
- **Source code:** main.c, mainCMD.c, assignment1.h
- **Program executables (exe):** main.exe, mainCMD.exe
- **Input/Output files:** input1.txt, input2.txt, output.txt
- **Batch script files:** test1.bat, test2.bat 
- **Part 1 question answers**
- **README files**

# SYSC 4001 - Assignment 2

**Date:** 2020-03-05
Note: Part 2 c) Does not work properly.
**Group members:**
- Marko Majkic, 101109409  
- Alex Cameron, 101114698

## Running batch files

The batch files are individually linked to the different scheduler programs. Running a bat file will run the respective scheduler 10 time with 10 different input files. The output can be seen in the respective output file for that scheduler. 

- Running **fcfsScheduler.bat** will run the first come, first serve scheduler. The output for this can be found in the **outputFCFS.txt**
- Running **roundRobinScheduler.bat** will run the Round Robin Scheduler. The output for this can be found in the **outputRR.txt**
- Running the **priorityScheduler.bat** will run the multi-queue priority scheduler. The output for this can be found in **outputP.txt**
- Running the **fcfsMemoryManagement.bat** will run the first come, first serve scheduler with memory management implemented. The output for this can be found in the **outputMM.txt**


(Note: the output in the output files will be overwritten any time a .bat or .exe is run)

**PLEASE NOTE:** The analytics (throughput, turnaround time, wait time, memory data) for the test cases can only be seen in the **command prompt output**. The output .txt files will only contain the execution data. 


## Running the program with custom input files

-   Paste the contents of the custom input file into one of the two 10 inputX.txt files located in the InputFiles folder and run the appropriate batch file.

## Running the program in an optimized IDE 

- Open the project in any optimized IDE
- Compile and run the .c file corresponding to the scheduler you wish to run (All programs are already set to run 10 times, with the 10 different input files).


## About Part 2
There are 4 different .c files with different schedulers: fcfsScheduler.c , roundRobinScheduler.c, priorityScheduler.c & memManagement.c. 
Each file runs 10 simulations with 10 different scenarios and outputs all the data into their respected output files: "outputFCFS.txt, "outputRR.txt", "outputP.txt" & "outputMM.txt"
There are several sets of calculations performed for part 2 b) consisting of average wait time, average turnaround time and throughput. These calculations are specified in their function documentation within the source code if unsure of how these calculations were performed. 

### Assumptions:
- Part 2 a): For external priority queue, upon discussing with TA’s they said that having 3 queues (same as multiqueue in part 1c)/d) iii) is sufficient. However, ideally there should be any number of priority levels, but for the simplicity of scheduler algorithm the first option with 3 queues is used.
- Lower priority level integer = higher priority
- In the priority queue we made an assumption that there is no aging. This meaning that once a process is bumped down from a higher priority queue to a lower priority then it cannot go back up to the higher queue and must keep either going down priority levels until the process finishes executing in it’s current queue.
- For Part 2 c), An assumption was made that if a process’ memory is too large for the biggest partition then the process is suspended and removed from readyQueue


## Final deliverables
- **Source code:** assignment2.h, fcfsScheduler.c, memManagement.c, priorityScheduler.c, roundRobinScheduler.c
- **Program executables (exe):** fcfsScheduler.exe, roundRobinScheduler.exe, priorityScheduler.exe, fcfsMemoryManagementScheduler.exe
- **Input/Output files:** input0.txt, input1.txt, input2.txt, input3.txt, input4.txt, input5.txt, input6.txt, input6.txt, input7.txt, input8.txt, input9.txt, outputFCFS.txt, outputRR.txt, outputP.txt, outputMM.txt, 
- **Batch script files:** fcfsScheduler.bat, roundRobinScheduler.bat, priorityScheduler.bat, fcfsMemoryManagementScheduler.bat 
- **Part 1 question answers with Part 2 b) &c) Analysis**
- **README files**


