#define SHSIZE 20
#define ACC_SIZE 10
#define PIN_SIZE 10

typedef struct accountInfo
{
  //account number
  char accountNum[ACC_SIZE];
  //pin number
  char pinNum[PIN_SIZE];
  //available funds
  float fundsAvailable;
} accountInfo;

typedef struct shared_data {
  int writtenFlag;
  char data[SHSIZE];
} shared_data;

struct my_message {
  long message_type; //message type (postiive number)
  accountInfo account;
  char data[SHSIZE]; //data being transfered
};

typedef struct accountAttempts
{
  //account number
  char accountNum[ACC_SIZE];
  //counter for attempts
  int counter;
} accountAttempts;
