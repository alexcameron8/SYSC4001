SYSC 4001 - Assignment 3

2021-04-13
Marko Majkic, 101109409 
Alex Cameron, 101114698 

----------------------
Assignment Description
----------------------
The goal of this assignment is to familiarize ourselves with concurrency, IPC, sempahores, shared memeory, and linux system calls. We are tasked with designing a system to simulate an automatic teller machine.
The system comprises of three concurrent processes - the ATM (user facing), the DB server that fetches data from the database, and the DB editor that makes changes to the database based on user actions. 

------------
Deliverables
------------
-Part1.pdf - Answers to part 1 questions.
-Atm.c - The atm component of the system which acts as an interface between the user and the back-end of the system
-DBEditor.c - The database editor component of the system which provides the system database editing capabilities
-DBServer.c - The databse server feeds the atm data from the database
-database.txt - The text file with all the information regarding the database
-Assignment3.h - the header file containing all relevant structures

----------------
Running the code
----------------
To run the part 2 B) code you will need to open 2 terminal windows. Once you are in the directory which contains the assignment3 files, in the first terminal window you will write the following commands:
Compile the 2 C files using these SPECIFIC files: (If DBServer.c or DBEditor.c are not compile with output and dbeditor then the program will not run properly since DBServer fork and execs these specific files)
$ gcc DBServer.c -o output
$ gcc DBEditor.c -o dbeditor
Run DBServer:
$ ./output 

In the second terminal window once you have compiled and run the first C file you will perform the following commands:
Compile the Atm.c C file (Not specific as to what file is opened):
$ gcc Atm.c -o atm
Run the ATM component:
./atm

Now there are 2 terminal windows open. The first window contains DBEditor and DBServer. In this window you can enter an account #, pin # and available funds. Once all these valid inputs are entered the user will be prompted
whether they wish to continue and add this account to the database (database.txt). Upon clicking yes the DBEditor sends an update to DBServer to update the database with the entered account. The user will then be prompted to 
either enter a new account by typing "N" or to exit the DBEditor and DBServer typing "X". 

The second terminal window will contain the ATM component. The user will be prompted to enter an account number and PIN number. Once both account and pin number are entered it sends the information to DBServer which will perform
a verification function to ensure the right account info was entered. If there is an account number in the database that contains the account #: 00001 and PIN: 107, then to correctly enter that account in the ATM you will enter 
account # :00001 and PIN: 108 because the verification involves an encryption algorithm of subtracting 1 from the PIN entered and checking the database.
Once a user is entered into their account they can perform the following operations:
1. "BALANCE" 
2. "WITHDRAW"
3. "EXIT"

The BALANCE operation will return the current available funds for the account logged in. The WITHDRAW operation will immediately prompt the user to enter the amount they wish to withdraw from the account. If the sufficient funds
are in the account, the DBServer will perform the withdrawal and update the user and account with the new updated decremented available funds. Otherwise, it will return an error with NSF (No sufficient funds). The last operation will 
Exit the ATM and prompt the user to a screen which has 2 options. The first option is to enter a new account by typing "A" and this will repeat the previous cycle. The second option is to Exit the ATM by typing "X". This will delete 
all shared variables used and close the program down safely.

NOTE: When exiting the program close the DBEditor using the "Exit" operation first THEN close the ATM terminal (Either force close "Ctl + c" while atm waits for user input or by exiting through console). Since there are 2 required 
terminals to concurrently enter inputs we could not figure out a way to close the ATM when the DBEditor is closed since ATM is almost always listening for user inputs. Implementing a timeout for the input is the only way we could 
think of as a good approach for solving this issue. If the TA who marks this has a better suggestion we are open to any ideas for implementing this in the term project!

----------------
Important Notes
----------------
1. Make sure to run the code for part 2 in the specific order specified and in 2 terminal windows (DBEditor and DBServer BEFORE Atm)
2. When closing the program ENSURE the program is exited properly (read instructions in last 2 paragraphs of "Running the code"). If the program is exited abruptly then there will be shared memory and message queues
that are not deleted and will potentially mess up the output and execution of the code when attempting to run again. See troubleshooting section if this occurs.
3. The code must be run in 2 terminal windows to concurrently accept input from both DBEditor and atm at the same time. Otherwise the input and output would not be possible all in one window under the case that
both DBeditor and ATM are looping forever. 
4. There are alot of commented out print statements for testing purposes left in the code to aid when developing the term project. :) 
----------------
Troubleshooting
----------------
In the event that the program is exited abruptly:
Perform the following commands to ensure all shared memory and shared message queues are deleted:
To view shared variables type:
$ ipcs 
To delete the shared message variables manually type the following in the console:
$ ipcrm -M 4294967295
$ ipcrm -Q 2234
$ ipcrm -Q 4432



 
