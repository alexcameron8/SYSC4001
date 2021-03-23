#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
//#include "assignment3.h"

FILE *file;

/**Function Prototypes */
void getDataBaseData(char *databaseData);
void database_add(char accountNum[4], char pinNum[2], float fundsAvailable);
/*
 * The readFile function takes a file name as a parameter and opens such file. This function
 *  calls the getProcessData() function to convert the input from the .txt file to a process struct.
 */
int readFile(const char *fileName){
    char text[1000];
    //Read file
    file = fopen(fileName,"r");
    if (!file)
        return 1;

    while (fgets(text,1000, file)!=NULL)
        getDataBaseData(text);

fclose(file);
    return 0;
  }

  /*
   * This function parses a line from the input file and splits the information
   * and adds the process information to the list of processes.
   */
  void getDataBaseData(char *databaseData){
    const char delim[2] = ",";
    char *database_Data;
    database_Data = strtok(databaseData,delim);
    char accountNum[4] = atoi(database_Data);
    database_Data = strtok(NULL,delim);
    char pinNum[2] = atoi(database_Data);
    database_Data = strtok(NULL,delim);
    float fundsAvailable = atoi(database_Data);

    database_add(accountNum, pinNum, fundsAvailable);
  }

  void database_add(char accountNum[4], char pinNum[2], float fundsAvailable){
    printf("%s , %s, %f\n", accountNum, pinNum, fundsAvailable);
  }

  int main(){
    readFile("database.txt");

    return 0;
  }
