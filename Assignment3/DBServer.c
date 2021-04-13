#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include "assignment3.h"

#define DB_SIZE 100

FILE *file; //the database file pointer
const char *fileName = "database.txt"; //dataase with account information
accountInfo currentAccount;
struct shared_data *sharedData;//used for single terminal implementation (will most likely be used in term project)
struct accountAttempts accountEntered; //local variable to remember account entered info
int currentAccountLineDB;
int msqid,msqidRec; //the shared message queue ids
key_t key; //unique key identifier

/** Function Prototypes */
void database_add(char accountNum[], char pinNum[], float fundsAvailable);
int verifyEncryption(char accountpinNum[], char pinNum[]);
int decrementFunds(float amount);
int updateDatabase();
void initShared();
int waitForMessage();
int checkLine(char text[], struct accountInfo account);
int getAvailableFunds();
int verifyAccount(accountInfo account);
void verifyWithdrawal(accountInfo account);
int blockAccount(char account[]);

/**
* Initialize child and parent processes. First child process is DBEditor , Second child process is attempt
* and the parent process is DBServer.
* $ gcc DBServer.c -o output
* $ gcc DBEditor.c -o dbeditor
* $ gcc atm.c -o atm
*/
void processInit(){
  int child_a = fork();
  if(child_a==-1){
    printf("Error initializing child DBEditor.\n");
    exit(1);
  }
  if(child_a == 0){ //first child process (DBEditor)
    printf("I am DBEditor\n");
    static char *argv[]={"./dbeditor",NULL};
    execv(argv[0],argv);
  }else{
    printf("I am DBServer\n");
    strcpy(accountEntered.accountNum, " "); //initialize the account number entered
    initShared();
    while(1){
      waitForMessage();
    }
  }
}
/**
* Initialize shared variables between DBServer and DBEditor.
*/
void initShared(){
  //shared message queue to receive msgs from DBEditor and ATM
  char *shm;
  key_t keyReceiveQueue = 2234;
  msqid = msgget(keyReceiveQueue,IPC_CREAT | 0600);
  if(msqid == -1){
    printf("Message queue failed to create\n");
  }
  //shared account var NOT USED IN ASSIGN 3 BUT WILL BE USEFUL FOR TERM PROJECT
  // key = ftok("SYSC4001/part2",1);
  // int shmid = shmget(key,SHSIZE, IPC_CREAT | 0666);
  // if(shmid <0){
  //   printf("Failure has occured in creating shared memory variable.\n");
  //   exit(1);
  // }
  // shm = shmat(shmid,NULL,0);
  // if(shm ==(char *)-1){
  //   printf("Failure has occured in making shared variable accessible.\n");
  //   exit(1);
  // }
  // sharedData = (struct shared_data *) shm;
  //init message queue with receiving data from DBServer
  key_t keySendQueue = 4432;
  msqidRec = msgget(keySendQueue,IPC_CREAT | 0600);
  if(msqidRec == -1){
    perror("msqidRec");
  }
}


/**
* This function is always receiving messages from ATM and DBEditor and when a msg is received it performs the corresponding
* operation.
*/
int waitForMessage(){
  char * message; //temp var
  initShared();
  while(1){
    struct my_message msg_received;
    int msgLength = sizeof(struct my_message) - sizeof(long);
    //printf("DBServer waiting for messages...\n"); TEST
    if(msgrcv(msqid, &msg_received,msgLength,1,0)==-1){
      //perror("DBServer msgrcv");
      exit(1);
    }else{
      //printf("DBServer Executing: \"%s\"\n",msg_received.data); TESTING
      //PIN message is received:
      if(strcmp(msg_received.data,"VERIFY") ==0){
        return verifyAccount(msg_received.account); //verify account number and pin number are in database
      }else if(strcmp(msg_received.data, "WITHDRAW") ==0){
        verifyWithdrawal(msg_received.account); //verify the account has sufficient funds to perform withdrawal
      }else if(strcmp(msg_received.data,"BALANCE") ==0){
          getAvailableFunds();
      }else if(strcmp(msg_received.data,"UPDATE_DB") ==0){
        //printf("Updating Database...\n"); TESTS
        //ADD account to database from DBEditor
        updateDatabase(msg_received.account);
      }
      return 0;
    }
  }
  return 0;
}
/**
* Adds a new account to the next line in the database file with the account information
*/
int updateDatabase(struct accountInfo account){
  // create new line to be entered in database by combining new information
  char temp[50];
  char updatedDatabase[100];
  char funds[20];
  sprintf(funds,"%f",account.fundsAvailable);
  //printf("%s\n", funds); TEST
  sprintf(temp, "%s,%s",account.accountNum,account.pinNum);
  sprintf(updatedDatabase, "%s,%s", temp,funds);
  //printf("(TEST)Updated Database line:%s\n",updatedDatabase); (Updated line in database) TEST

  FILE * dbPtr;
  FILE * tempFile;
  char buffer[1000];
  dbPtr = fopen(fileName,"r");
  tempFile = fopen("temp.txt", "w");

  if(!dbPtr || !tempFile){
    printf("Error opening file(s).");
    return 1;
  }
  //copy all database over
  while((fgets(buffer,1000,dbPtr))){
      fputs(buffer, tempFile);
  }
  fputs(updatedDatabase,tempFile);
  fputs("\n",tempFile);

  fclose(dbPtr);
  fclose(tempFile);

  //delete former database file
  remove(fileName);

  //rename new temp file to new database
  rename("temp.txt", fileName);
  //printf("Successfully updated Database with new Account: %s.\n",account.accountNum); TEST
}

/**
* Get the available funds in the current account
*/
int getAvailableFunds(){
  //printf("Available Funds in Acc# %s : $%f\n ", currentAccount.accountNum, currentAccount.fundsAvailable);
  struct my_message msg;
  msg.message_type = 1;
  msg.account.fundsAvailable = currentAccount.fundsAvailable;
  strcpy(msg.data, "BAL");
  int msgLength = sizeof(struct my_message) - sizeof(long);
  if(msgsnd(msqidRec, &msg, msgLength,0) == -1){
    perror("ATM Msgsnd");
  }
  return 0;
}

/**
* This function validates there are sufficient funds in the current active account to be able
* to follow through with the withdrawal
*/
void verifyWithdrawal(accountInfo account){
  if(currentAccount.fundsAvailable >= account.fundsAvailable){
    decrementFunds(account.fundsAvailable); //decrementFunds
  }
  else{
    //notify atm NSF
    struct my_message msg;
    msg.message_type = 1;
    strcpy(msg.data, "NSF");
    int msgLength = sizeof(struct my_message) - sizeof(long);
    if(msgsnd(msqidRec, &msg, msgLength,0) == -1){
      perror("ATM Msgsnd");
    }
  }
}
/**
* Decrement the specified funds and update the database
*/
int decrementFunds(float amount){
  //decrement funds
  currentAccount.fundsAvailable = currentAccount.fundsAvailable - amount;
  //printf("New updated account funds: %f\n", currentAccount.fundsAvailable); TEST

  // create new line to be entered in database
  char temp[50];
  char updatedDatabase[100];
  char funds[20];
  sprintf(funds,"%f",currentAccount.fundsAvailable);
  sprintf(temp, "%s,%s",currentAccount.accountNum,currentAccount.pinNum);
  sprintf(updatedDatabase, "%s,%s", temp,funds);
  //printf("(TEST)Updated Database line:%s\n",updatedDatabase); TEST

  //Update database file
  FILE * dbPtr;
  FILE * tempFile;
  char buffer[1000];
  int count = 0;
  dbPtr = fopen(fileName,"r");
  tempFile = fopen("temp.txt", "w");

  if(!dbPtr || !tempFile){
    printf("Error opening file(s).");
    return 1;
  }
  //when the count = the line to be update insert new line in database created above
  while((fgets(buffer,1000,dbPtr))){
    count++;
    if(count == currentAccountLineDB){
      fputs(updatedDatabase,tempFile);
    }else{
      fputs(buffer, tempFile);
    }
  }
  fclose(dbPtr);
  fclose(tempFile);

  //delete former database file
  remove(fileName);

  //rename new temp file to new database
  rename("temp.txt", fileName);

  //printf("Successfully updated Database with decremented funds.\n"); TEST
  //send msg to notify atm of decremented funds
  struct my_message msg;
  msg.message_type = 1;
  msg.account = currentAccount;
  strcpy(msg.data, "FUNDS_OK");
  int msgLength = sizeof(struct my_message) - sizeof(long);
  if(msgsnd(msqidRec, &msg, msgLength,0) == -1){
    perror("ATM Msgsnd");
  }
  return 0;
}

/**
* This function sets the local current active account that is open in the ATM.
*/
void setCurrentAccount(char acc[], char pin[], float availFunds){
  strcpy(currentAccount.accountNum, acc); //set account num
  strcpy(currentAccount.pinNum, pin); //set pin num
  currentAccount.fundsAvailable = availFunds; // set available funds
}

/**
* This function checks the database and checks if the account entered is valid.
*/
int verifyAccount(accountInfo account){
  //printf("Verify account func.\n"); TEST
  //update accountAttempts
  if(strcmp(accountEntered.accountNum,account.accountNum)==0){
    accountEntered.counter++;
  }else{
    strcpy(accountEntered.accountNum,account.accountNum);
    accountEntered.counter=1;
  }
  char text[1000];
  int success = false;
  //read file
  file = fopen(fileName,"r");
  if(!file){
    return 1;
  }
  int counter = 0;
    while (fgets(text,1000, file)!=NULL){
      counter++;
      if(checkLine(text, account)){ //check account number NOTE: CHANGE currentAccount TO SHARED MEM VARIABLE LATER
        currentAccountLineDB = counter; //update database location of account info
        success = true;
        break;
      }
    }
    fclose(file);
    if(!success){ //account was not found
      struct my_message msg;
      if(accountEntered.counter<3){
        strcpy(msg.data, "PIN_WRONG");
      }else{
        blockAccount(account.accountNum); //more than 3 attempts on entering account info
        strcpy(msg.data, "PIN_BLOCKED");
      }
      //send msg back to ATM
      msg.message_type = 1;
      int msgLength = sizeof(struct my_message) - sizeof(long);
      if(msgsnd(msqidRec, &msg, msgLength,0) == -1){
        perror("ATM Msgsnd");
      }
      return false;
    }else{ //account was found
      struct my_message msg;
      msg.message_type = 1;
      strcpy(msg.data, "OK");
      int msgLength = sizeof(struct my_message) - sizeof(long);
      if(msgsnd(msqidRec, &msg, msgLength,0) == -1){
        perror("ATM Msgsnd");
      }
      return true;
    }
    return 0;
}

/**
* Parses a line in the database and retrieves the valid account details.
*/
int checkLine(char text[], struct accountInfo account){
  const char delim[2] = ",";
  int accountFound = false;
  int accountVerified = false;
  char *database_Data;
  char pinNum[10];
  char accountNum[10];
  database_Data = strtok(text,delim);
  strcpy(accountNum, database_Data);
  if(strcmp(account.accountNum,text) ==0){
    //printf("Account %s found in Database.", account.accountNum); TEST
    accountFound = true;
  }
  if(accountFound){ //If the account number matches validate PIN Encryption.
    database_Data = strtok(NULL,delim);
    strcpy(pinNum, database_Data);
    if(verifyEncryption(account.pinNum,pinNum)){
      accountVerified = true;
    }
  }
  //set local active account variable now that user has successfully validated and entered account
  if(accountVerified){
    setCurrentAccount(accountNum,pinNum,atof(strtok(NULL,delim)));
  }
  return accountVerified;
}

/**
* Performs encyrption validation for PIN given in an account number
*/
int verifyEncryption(char accountPin[], char pinNum[]){
  int accountPinNum = 0;
  int encryptedPinNum = 0;
  accountPinNum = atoi(accountPin);
  encryptedPinNum = atoi(pinNum);
  //printf("PIN: %d\n", accountPinNum); TEST
  //printf("Encrypting... \n");
  accountPinNum--;
  if(accountPinNum == encryptedPinNum){ //encryption verified
    //printf("Encryption Verified: %d, %d\n", accountPinNum, encryptedPinNum); TESTING
    return true;
  }else{ //Encryption failed
    //printf("Encryption Failed: %d, %d\n", accountPinNum, encryptedPinNum); TESTING
    return false;
  }
}
/**
* Block an account in the database by updating the first char of the account # to
* a 'x'.
*/
int blockAccount(char account[]){
  //Update database file
  FILE * dbPtr;
  FILE * tempFile;
  char buffer[1000], text[1000];
  int count = 0;
  int lineBlocked;
  int success = false;
  char updatedDatabase[100];
  dbPtr = fopen(fileName,"r");
  tempFile = fopen("temp.txt", "w");

  if(!dbPtr || !tempFile){
    printf("Error opening file(s).");
    return 1;
  }
  //find account number to blocks line.
  while((fgets(buffer,1000,dbPtr))){
    count++;
    const char delim[2] = ",";
    char accountNum[10];
    char pinNum[10];
    double fundsAvail;
    char * database_Data;
    database_Data = strtok(buffer,delim);
    strcpy(accountNum, database_Data);
    if(strcmp(account, accountNum)==0){
      database_Data = strtok(NULL,delim);
      strcpy(pinNum, database_Data);
      database_Data = strtok(NULL,delim);
      fundsAvail = atof(database_Data);
      //create new line to be entered with blocked account edited.
      lineBlocked = count;
      success = true;
      char temp[50];
      char funds[20];
      sprintf(funds,"%f",fundsAvail);
      sprintf(temp, "%s,%s",accountNum,pinNum);
      sprintf(updatedDatabase, "%s,%s\n", temp,funds);
      updatedDatabase[0] = 'x';
      //printf("(TEST)Updated Database line:%s\n",updatedDatabase); TEST
    }
  }
  fclose(dbPtr);
  fclose(tempFile);
  dbPtr = fopen(fileName,"r");
  tempFile = fopen("temp.txt", "w");
  if(success){
    count = 0;
    while((fgets(text,1000,dbPtr))){
      count++;
      if(count == lineBlocked){
        fputs(updatedDatabase,tempFile);
      }else{
        fputs(text, tempFile);
      }
    }
  }else{ //could not find account # in database
    //printf("TEST COULD NOT FIND ACC # %s IN DB.\n", account);
  }

  fclose(dbPtr);
  fclose(tempFile);

  //delete former database file
  remove(fileName);

  if(success){
    //rename new temp file to new database
    rename("temp.txt", fileName);
  }
  //printf("Successfully updated Database with blocked account.\n"); TEST
  return 0;
}

int main(){
  processInit();
}
