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

//struct shared_data *sharedData;
// accountInfo *sharedAccount;
int msqid,msqidRec;
int shmid;
/**Function prototypes */
int dbEditor();
void initMessageQueue();

/**
* Intiialization of the shared message queue between DBeditor and Dbserver.
*/
void initMessageQueue(){
  char *shm;
  key_t key = 2234;
  msqid = msgget(key,IPC_CREAT | 0600);
  if(msqid == -1){
    printf("Message queue failed to create\n");
  }
  //init message queue with sending data from DBServer to ATM
  key_t keyReceiveQueue = 4432;
  msqidRec = msgget(keyReceiveQueue,IPC_CREAT | 0600);
  if(msqidRec == -1){
    perror("msqidRec");
  }
}
/**
* This function requests an account #, pin # and avail funds and sends a msg to DBServer
* which will update the database with the new account created in this function.
*/
int dbEditor(){
  int reset = false;
  accountInfo newAccount;
  printf("DBEDitor:\n");
  while(1){
    char account[ACC_SIZE];
    char pinNum[PIN_SIZE];
    float funds;
    char confirm[2];
    char ready[2];
    int accountConfirmed = false;
    int pinConfirmed = false;
    int fundsConfirmed = false;
    char addAccount[2];
    int correctResetInput = false;
    if(reset){ //if entering not for first time, prompted with following msg
      sleep(2);
      while(!correctResetInput){ //Enter an N or X
        printf("\n1. Add a new account - \"N\"\n2. Exit - \"X\"\nOperation:");
        scanf("%s", addAccount);
        if(strcmp(addAccount, "N")==0 || strcmp(addAccount, "X")==0){
          correctResetInput = true;
        }
      }
    }else{
      strcpy(addAccount,"N");
    }

    fflush(stdin);
    if(strcmp(addAccount, "N")==0){ //new account to be added
      //Account #
      while(!accountConfirmed){
        if(!reset){
          printf("Enter an Account Number:");
        }
        sleep(1);
        fgets(account, ACC_SIZE,stdin);
        if(strlen(account)==6){
          accountConfirmed = true;
        }else if(strlen(account) ==1){
          reset = false;
        }else{
          printf("Invalid length. Must be length 5.\n");
        }
      }
      //enter pin number
      while(!pinConfirmed){
        printf("Enter a PIN:");
        sleep(1);
        fgets(pinNum, PIN_SIZE,stdin);
        if(strlen(pinNum)==4){
          pinConfirmed = true;
        }else{
          printf("Invalid PIN length. Must be length 3.\n");
        }
      }

      //enter available funds
      while(!fundsConfirmed){
        printf("Enter Account's Available Funds:");
        sleep(1);
        scanf("%f", &funds);
        if(funds >0){
          fundsConfirmed = true;
        }else{
          printf("Cannot enter negative available funds.\n");
        }
      }

      account[strcspn(account, "\n")] = 0;
      pinNum[strcspn(pinNum, "\n")] = 0;

      printf("Account #: %s\nPIN: %s\nAvailable Funds: %f\n", account, pinNum, funds);
      int confirmed =false;
      while(!confirmed){ //confirm the values
        printf("\nTo confirm values type \"Y\" or to reenter values type \"N\"\n");
        scanf("%s", confirm);
        if(strcmp(confirm,"Y")==0){
          printf("Confirmed.\n");
          confirmed = true;
          //create new account struct with given details
          strcpy(newAccount.accountNum,account);
          strcpy(newAccount.pinNum, pinNum);
          newAccount.fundsAvailable = funds;
          printf("Data sending:\nAccount #: %s\n",newAccount.accountNum);
          printf("PIN: %s\n", newAccount.pinNum);
          printf("Available Funds: %f\n", newAccount.fundsAvailable);

          reset = true; //reset back to beginning
          //send UPDATE to DBServer
          initMessageQueue();

          struct my_message msg;
          msg.account = newAccount;
          msg.message_type = 1;
          strcpy(msg.data, "UPDATE_DB");
          int msgLength = sizeof(struct my_message) - sizeof(long);
          printf("Sending...\n");
          if(msgsnd(msqid, &msg, msgLength,0) == -1){
            perror("Msgsnd error");
          }
          //delete message queue until next use.
          if(msgctl(msqid,IPC_RMID,NULL) ==-1){
            perror("msgctl");
            exit(1);
          }
        }else if(strcmp(confirm,"N")==0){
          printf("Not Confirmed.\n");
          confirmed = true;
          reset = true;
        }else{
          printf("Try again.\n");
        }
      }
    }else if(strcmp(addAccount,"X")==0){ //if user wishes to exit
      printf("Exiting...\n");
      initMessageQueue();
      if(msgctl(msqid, IPC_RMID,0) == -1){ //delete msg queue
        perror("msgctl");
      }
      if(msgctl(msqidRec, IPC_RMID,0) == -1){ //delete atm & dbserver msg queue
        perror("msgctl Rec");
      }
      exit(1);
    }
  }
  return -1;
}

int main(){
  dbEditor();
  //typeRequest();
}
