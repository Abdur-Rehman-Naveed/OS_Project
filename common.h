

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<unistd.h>


#define PIPE_NAME="bank_pipe";
#define BURST_TIME=3;

typedef struct{
    int accountID;
    string transactionType;
    int payrollCount;
    int priority;
    double amount;
    int burstTime;
}Transaction;

typedef struct{
    int accNo;
    int priority;
    string name;
    string customerType;
}Customer;