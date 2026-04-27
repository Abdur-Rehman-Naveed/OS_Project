

#include<stdio.h>
#include<stdlib.h>
#include<char.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<unistd.h>
#include<pthread.h>

#define REQUEST_PIPE "request_pipe"
#define RESPONSE_PIPE "response_pipe"
#define BURST_TIME 3

typedef struct{
    int accountID;
    char requestType[30];
    char name[50];
    char transactionType[30];
    int payrollCount;
    int priority;
    double amount;
}BankRequest;


typedef struct{
    int success;
    char msg[100];
}BankResponse;