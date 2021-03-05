# SYSC 4001 - Assignment 2

**Date:** 2020-03-05

**Group members:**
- Marko Majkic, 101109409  
- Alex Cameron, 101114698

## Running batch files //TO-DO

The batch files are individually linked to the two input files (input1.txt, input2.txt), running **test1.bat** will execute the program with **input1.txt**, and running **test2.bat** will execute the program with **input2.txt**. 

Once the files are executed, the output will be shown in the terminal, and can also be seen in the **output.txt** file found in the 'Assignment1_Part2' directory. (Note: the output in **output.txt** will be overwritten any time a .bat or .exe is run)

## Running the program with custom input files //TO-DO

Running the program with custom input files can be done in one of two ways:

-   Put the custom input file (in .txt format) in the 'Assignment1_Part2' directory, then from the command prompt, cd into the 'Assignment1_Part2' folder and run the command **cmd /k mainCMD.exe [custom file name].txt** without the brackets. The output should appear in the command prompt as well as in the **output.txt** file found in the 'Assignment1_Part2' directory.
-   Paste the contents of the custom input file into one of the two **input.txt** files and run the corresponding batch file.

## Running the program in an optimized IDE //TO-DO


## About Part 2
There are 4 different .c files with different schedulers: fcfsScheduler.c , roundRobinScheduler.c, priorityScheduler.c & memManagement.c. 
Each file runs 10 simulations with 10 different scenarios and outputs all the data into their respected output files: "outputFCFS.txt, "outputRR.txt", "outputP.txt" & "outputMM.txt"
There are several sets of calculations performed for part 2 b) consisting of average wait time, average turnaround time and throughput. These calculations are specified in their function documentation within the source code if unsure of how these calculations were performed. 

### Assumptions:
- Part 2 a): For external priority queue, upon discussing with TA’s they said that having 3 queues (same as multiqueue in part 1c)/d) iii) is sufficient. However, ideally there should be any number of priority levels, but for the simplicity of scheduler algorithm the first option with 3 queues is used.
- Lower priority level integer = higher priority
- In the priority queue we made an assumption that there is no aging. This meaning that once a process is bumped down from a higher priority queue to a lower priority then it cannot go back up to the higher queue and must keep either going down priority levels until the process finishes executing in it’s current queue.
- For Part 2 c), An assumption was made that if a process’ memory is too large for the biggest partition then the process is suspended and removed from readyQueue


## Final deliverables //TO-DO
- **Source code:** 
- **Program executables (exe):** 
- **Input/Output files:** input0.txt, input1.txt, input2.txt, input3.txt, input4.txt, input5.txt, input6.txt, input6.txt, input7.txt, input8.txt, input9.txt, outputFCFS.txt, outputRR.txt, outputP.txt, outputMM.txt, 
- **Batch script files:** 
- **Part 1 question answers with Part 2 b) &c) Analysis**
- **README files**
