

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>

#define REQUEST_PIPE "request_pipe"
#define RESPONSE_PIPE "response_pipe"
#define BURST_TIME 3

typedef struct{
    int accountID;
    char requestType[30];
    char name[50];
    char customerType[30];
    char transactionType[30];
    int payrollCount;
    int priority;
    double amount;
    char responsePipe[50];
}BankRequest;


typedef struct{
    int success;
    char msg[100];
    double updatedBalance;
}BankResponse;