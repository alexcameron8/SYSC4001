# SYSC 4001 - Assignment 1

**Date:** 2020-02-04

**Group members:**
- Marko Majkic, 101109409  
- Alex Cameron, 101114698

**IMPORTANT NOTE:** The **only** difference between main.c and mainCMD.c is that mainCMD.c accepts command line arugments in order to run different input files without changing the source code.

## Running batch files

The batch files are individually linked to the two input files (input1.txt, input2.txt), running test1.txt will execute the program with **input1.txt**, and running **test2.txt** will execute the program with **input2.txt**.

Once the files are executed, the output will be shown in the terminal, and can also be seen in the **output.txt** file found in the 'Assignment 1' directory.

## Running the program with custom input files

Running the program with custom input files can be done in one of two ways:

-   Put the custom input file (in .txt format) in the 'Assignment 1' directory, then from the command prompt, cd into the 'Assignment1' folder and run the command **cmd /k mainCMD.exe [custom file name].txt** without the brackets. The output should appear in the command prompt as well as in the **output.txt** file found in the 'Assignment 1' directory.
-   Paste the contents of the custom input file into one of the two **input.txt** files and run the corresponding batch file.

## Running the program in an optimized IDE

There are two .c files in the 'Assignment 1' directory. **mainCMD.c** is the .c file that allows for command line arguments (for the batch files and custom files), and the **main.c** file is the original .c file which can be run from an IDE and does not require arguments to be passed to it (automatically takes **input1.txt** but can be changed to accept any input file).

## Final deliverables
- **Source code:** main.c, mainCMD.c, assignment1.h
- **Program executables (exe):** main.exe, mainCMD.exe 
- **Batch script files:** test1.bat, test2.bat 
- **Part 1 question answers**
- **README files**
