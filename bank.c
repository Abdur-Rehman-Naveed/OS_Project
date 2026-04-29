#include<common.h>



int timeQuantam=BURST_TIME;
int Time=0;
pthread_mutex_t transactionLock;
pthread_mutex_t bankLock;


typedef struct{
    BankRequest request;
    int arrivalTime;
    int startTime;
    int completionTime;
    int remainingTime;
    struct Node* next;
}Node;

typedef struct{
    int accountID;
    char name[50];
    int priority;
    double balance;
    int maxNeed;
    struct Customer *next;
}Customer;


typedef struct{
    int balance;
    int customerCount;   
}Bank;
Customer *customers=NULL;
Node *queue=NULL;
Bank bank = {1000000, 0};

void enqueue(BankRequest r){
    Node *newNode = (Node*)malloc(sizeof(Node));
    newNode->request=r;
    newNode->next=NULL;
    newNode->remainingTime=BURST_TIME;
    if(!queue || r.priority>=queue->request.priority){
        newNode->next=queue;
        queue=newNode;
    }else{
        Node *temp=queue;
        while(temp->next!=NULL && temp->next->request.priority>=r.priority){
            temp=temp->next;
        }
        newNode->next=temp->next;
        temp->next=newNode;
    }
}
void rrEnqueue(Node* n){

    if(!queue || n.priority>=queue->request.priority){
        n->next=queue;
        queue=n;
    }else{
        Node *temp=queue;
        while(temp->next!=NULL && temp->next->request.priority>=n.priority){
            temp=temp->next;
        }
        n->next=temp->next;
        temp->next=n;
    }
}

void scheduler_RoundRobin(){
    while(1){
        if(!queue){
            sleep(1);
            continue;
        }

        Node *current=queue;
        queue=queue->next;
        
        for(int i =0;i<timeQuantam;i++){
            Time++;
            current->remainingTime--;
            usleep(500000);
        }
        if(current->remainingTime<=0){
            if(processRequest(current->request)){
                send_response(current->request, 1, "Transaction Complete");
            }else{
                send_response(current->request, 1, "Transaction Complete");
            }
            free(current);
        }else{
            rrEnqueue(current);
        }        
    }
}

void scheduler_FCFS(){
    while(1){
        if(!queue){
            sleep(1);
            continue;
        }

        Node *current=queue;
        queue=queue->next;

        for(int i =0;i<current->remainingTime;i++){
            current->remainingTime--;
            usleep(500000);
        }
        if(processRequest(current->request)){
            send_response(current->request, 1, "Transaction Complete");
        }else{
            send_response(current->request, 0, "Transaction failed");
        }
        free(current);

    }
}

void send_response(BankRequest r,int success,char msg[]){
    BankResponse res;
    res.success=success;
    strcpy(res.msg,msg);

    int fd=open(RESPONSE_PIPE,O_WRONLY);
    if(fd!=-1){
        write(fd,&res,sizeof(BankResponse));
        close(fd);
    }

}


void addCustomer(int accountID,char name[50],int priority,double balance,int maxNeed){
    Customer *newCustomer=(Customer*)malloc(sizeof(Customer));
    newCustomer->accountID=accountID;
    newCustomer->name=name;
    newCustomer->balance=balance;
    newCustomer->priority=priority;
    newCustomer->next=NULL;
    newCustomer->maxNeed=maxNeed;

    if(!customers){
        customers=newCustomer;
    }else{
        Customer *temp=customers;
        while(temp->next!=NULL){
            if(temp->accountID==accountID){
                printf("Customer already exists");
                return;
            }
            temp=temp->next;
        }
        temp->next=newCustomer;
    }
}


void Priority(){}

int loggedInAccountID;
Customer* searchAccounts(int id){
    Customer *temp=customers;
    while(temp!=NULL){
        if(temp->accountID==id){
            return temp;
        }
    }
    return NULL;
}

int withdraw(Customer *c,double amount)
{
    if(c->balance>=amount)
    {
        pthread_mutex_lock(&transactionLock);
        c->balance-=amount;
        pthread_mutex_unlock(&transactionLock);
        return 1;
    }else
    {
        return 0;
    }
}
int deposit(Customer *c,double amount)
{
        c->balance+=amount;
        return 1;
}

int processRequest(BankRequest req){
    
    if(!strcmp(req.requestType,"REGISTER_ACCOUNT"))
    {
        int maxNeed;
        if(req.priority==3){
            maxNeed=10000;
        }
        else if(req.priority==2){
            maxNeed=5000;
        }else if(req.priority==1){
            maxNeed=3000;
        }
        addCustomer(req.accountID,req.name,req.priority,req.amount,maxNeed);
        return 1;

    }else if(!strcmp(req.requestType,"LOGIN"))
    {
        if(searchAccounts(req.accountID))
        {
            loggedInAccountID=req.accountID;
            return 1;
        }else
        {
            return 0;
        }
        
    }else if(!strcmp(req.requestType,"TRANSACTION"))
    {
        if(!strcmp(req.transactionType,"DEPOSIT"))
        {
            Customer *cust=searchAccounts(req.accountID);
            if(cust)
            {
                if(deposit(cust,req.amount))
                {
                  return 1;
                }else
                {
                  return 0;
                }
            }else
            {
                return 0;
            }
        }else if(!strcmp(req.transactionType,"WITHDRAW"))
        {
            Customer *cust=searchAccounts(req.accountID);
            if(cust)
            {
                if(withdraw(cust,req.amount))
                {
                return 1;
                }else
                {
                    return 0;
                }
            }else
            {
                return 0;
            }

        }else if(!strcmp(req.transactionType,"LOAN"))
        {
            Customer *cust=searchAccounts(req.accountID);
            if(cust)
            {
                applyLoan(cust,req.amount);
                return 1;
            }else
            {
                return 0;
            }
        }else
        {

        }
    }else
    {
        return 0;
    }
    
}

int applyLoan(Customer *c,double amount){
    pthread_mutex_lock(&bankLock);
    if(amount<=bank.balance){
        bank.balance-=amount;
        deposit(c,amount);
        pthread_mutex_unlock(&bankLock);
        return 1;
    }else{
        pthread_mutex_unlock(&bankLock);
        return 0;
    }
}
void* readRequests(void* arg){
    
    while(1){
        int fd_request= open(REQUEST_PIPE,O_RDONLY);
        if(fd_request==-1){
            perror("PIPE NOT OPEN");
            return NULL;
        }
        BankRequest req;
        if(read(fd_request,&req,sizeof(BankRequest))>0){
            enqueue(req);
        }
        close(fd_request);
    }

    return NULL;
}
void* processThreads(void* arg){

    return NULL;
}


int main(){
    mkfifo(REQUEST_PIPE, 0666);
    mkfifo(RESPONSE_PIPE, 0666);

    pthread_t readingThread,processThread;
    pthread_create(&readingThread,NULL,readRequests,NULL);
    pthread_create(&processThread,NULL,processThreads,NULL);

    pthread_join(readingThread,NULL);
    pthread_join(processThread,NULL);


    return 0;
}