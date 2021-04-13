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

//global variables
#define USER_SIZE 100
#define PIN_SIZE 10
#define OP_SIZE 20

int shmid,shmidAcc, msqid, msqidRec;
//struct shared_data *sharedData;
struct accountInfo *sharedAccount;

/** Function Prototypes */
int bankOperation();
void requestInfo();
int getBalance();

/**
* Initialize shared message queues and shared variables.
*/
void initShared(){
  key_t key, keyAcc;
  char *shm;
  char * shmAcc;
  //shared memory variable to determine which program can execute.
  // key = ftok("SYSC4001/part2",1);
  // shmid = shmget(key,SHSIZE, IPC_CREAT | 0666);
  // if(shmid <0){
  //   perror("shared memory:");
  //   exit(1);
  // }
  // shm = shmat(shmid,NULL,0);
  // if(shm ==(char *)-1){
  //   printf("Failure has occured in making shared variable accessible.\n");
  //   exit(1);
  // }
  // sharedData = (struct shared_data *) shm;
  //init message queue with sending to DBServer
  key_t keySendQueue = 2234;
  msqid = msgget(keySendQueue,IPC_CREAT | 0600);
  if(msqid == -1){
    printf("Message queue failed to create\n");
  }
  //init message queue with receiving data from DBServer
  key_t keyReceiveQueue = 4432;
  msqidRec = msgget(keyReceiveQueue,IPC_CREAT | 0600);
  if(msqidRec == -1){
    perror("msqidRec");
  }
}

/**
* Verifys the new account is verified correctly.
*/
int verifyAccount(char user[], char pin[]){
  initShared(); //initialize shared variables
  //new account struct to send account and pin number info to DBserver
  accountInfo newAccount;
  strcpy(newAccount.accountNum,user);
  strcpy(newAccount.pinNum,pin);
  //send data containing account & pin numbers
  struct my_message msg;
  msg.account = newAccount;
  msg.message_type = 1;
  strcpy(msg.data, "VERIFY");
  int msgLength = sizeof(struct my_message) - sizeof(long);
  if(msgsnd(msqid, &msg, msgLength,0) == -1){
    perror("ATM Msgsnd");
  }
  printf("ATM waiting for account verification...\n");
  //receive reply from DBServer with verification update
  struct my_message msg_received;
  if(msgrcv(msqidRec, &msg_received,msgLength,1,0)==-1){
    perror("ATM msgrcv");
    exit(1);
  }else{
    //printf("Result: \"%s\"\n",msg_received.data);
    //receive response DBServer
    if(strcmp(msg_received.data, "PIN_WRONG") ==0){ //wrong pin or account number entered
      printf("Account information incorrect. Try again.\nUpon 3 failed attempts your account will be blocked.\n");
     return false;
   }else if(strcmp(msg_received.data, "OK") == 0){ //account and pin was OK.
      return true;
    }else if(strcmp(msg_received.data, "PIN_BLOCKED") == 0){
      printf("Too many attempts. Account has been blocked. Please contact bank support to resolve issue with your account.\n");
      return false;
    }else{
      printf("Error in account verification.\n");
      return false;
    }
  }
}
/**
* Function that verifies sufficient funds are in the active account when trying to withdraw
* money from the database.
*/
int verifyFunds(float withdrawalFunds){
  accountInfo newAccount;
  newAccount.fundsAvailable = withdrawalFunds;
  //send data to DBServer containing desired withdrawal amount
  struct my_message msg;
  msg.account = newAccount;
  msg.message_type = 1;
  strcpy(msg.data, "WITHDRAW");
  int msgLength = sizeof(struct my_message) - sizeof(long);
  if(msgsnd(msqid, &msg, msgLength,0) == -1){
    perror("ATM Msgsnd");
  }
  printf("ATM waiting for withdrawal verification...\n");
  //receive reply from DBServer
  struct my_message msg_received;
  if(msgrcv(msqidRec, &msg_received,msgLength,1,0)==-1){
    perror("ATM msgrcv");
    exit(1);
  }else{
    if(strcmp(msg_received.data,"NSF") ==0){ //no sufficient funds
      printf("No Sufficient Funds.\n");
      return false;
    }else if(strcmp(msg_received.data, "FUNDS_OK") ==0){ //sufficient funds
      printf("Withdrawal Successful.\nNew balance: %f\n", msg_received.account.fundsAvailable);
      return true;
    }
  }
}
/**
* ATM notifies DBServer to retrieve the balance of the active account.
*/
int getBalance(){
  //send data requesting a balance update
  struct my_message msg;
  msg.message_type = 1;
  strcpy(msg.data, "BALANCE");
  int msgLength = sizeof(struct my_message) - sizeof(long);
  if(msgsnd(msqid, &msg, msgLength,0) == -1){
    perror("ATM Msgsnd");
  }
  printf("ATM retrieving account balance..\n");
  //received reply from DBServer with accounts balance
  printf("Receiving balance from DBServer.\n");
  struct my_message msg_received;
  if(msgrcv(msqidRec, &msg_received,msgLength,1,0)==-1){
    perror("ATM msgrcv");
    exit(1);
  }else{ //print balance
    printf("BALANCE: \"%f\"\n",msg_received.account.fundsAvailable);
  }
  return 0;
}

/**
 * Retrieves users username, password and verifys the account details.
 * (For now the user and pin are always correct)
 */
void requestInfo(){
  int reset = false;

  int counter =0;
  char userName[USER_SIZE];
  char passWord[PIN_SIZE];
  char addAccount[2];
  int correctResetInput = false;
  strcpy(addAccount,"A");
  while(1){
    int correctResetInput = false;
    if(reset){
      sleep(2);
      while(!correctResetInput){
        printf("\n1. Use the ATM - \"A\"\n2. Exit - \"X\"\n");
        scanf("%s", addAccount);
        if(strcmp(addAccount, "A")==0 || strcmp(addAccount, "X")==0){
          correctResetInput = true;
        }
      }
    }else{
      strcpy(addAccount,"A");
    }
    if(strcmp(addAccount,"A") ==0){
      while(counter<=3){
        printf("\nEnter Account #: ");
        scanf("%s", userName);
        printf("Enter PIN: ");
        scanf("%s", passWord);
        printf("User: %s , PIN: %s\n", userName, passWord);
        if(verifyAccount(userName,passWord)){
          printf("OK\n");
          bankOperation();
          reset = true;
          break;
        }
      }
    }else if(strcmp(addAccount,"X") ==0){
      printf("Atm exiting...\n");
      //delete & clear shared memory and shared message queues (NOT USED IN A3)
      // if(shmctl(shmid, IPC_RMID,0) == -1){
      //   printf("shmid");
      // }
      msgctl(msqid,IPC_RMID,NULL);
      msgctl(msqidRec,IPC_RMID,NULL);
      exit(1);
    }else{
      printf("Error...\n");
    }
  }
}

/**
* Once the user logs into their account they can perform the following operations.
* @Marko the balance and exit operations are fine its just the withdraw one.
*/
int bankOperation(){
  char operation[OP_SIZE];
  char buffer[OP_SIZE];
  int operating = true;
  while(operating){
    printf("\nChoose Banking Operation: \n 1. Show balance - \"BALANCE\" \n 2. Withdrawal - \"WITHDRAW\" \n 3. Exit - \"X\" \n\n");
    printf("Enter Operation: ");
    scanf("%s", operation);
    //printf("Operation entered: %s\n", operation);
    if(strcmp(operation, "BALANCE") ==0){
      printf("Retrieving Balance...\n"); //TO-DO
      getBalance();
      operating = true;
    }else if(strcmp(operation, "X") ==0){
      printf("Exiting ATM Machine.\n");
      return 0;
    }else if(strcmp(operation, "WITHDRAW")==0){ //check first word is WITHDRAW
      double amount;
      printf("Enter Withdrawal Amount: ");
      scanf("%lf", &amount);
      //printf("Amount: %f\n", amount);
      if(verifyFunds(amount)){ //if successful then operation complete and exit.
        operating = true;
      }
    }else{
      printf("Invalid operation\n");
    }
  }
  return -1;
}


int main(){
  requestInfo();
}
