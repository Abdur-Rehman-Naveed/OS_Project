#include<common.h>



int timeQuantam=BURST_TIME;

typedef struct{
    BankRequest request;
    int remainingTime=BURST_TIME;
    Node* next;
}Node;


Node *queue=NULL;

void enqueue(BankRequest r){
    Node *newNode = (Node*)malloc(sizeof(Node));
    newNode->request=r;
    newNode->next=NULL;

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

void scheduler_RoundRobin(){
    while(1){
        if(!queue){
            sleep(1);
            continue;
        }

        Node *current=queue;
        queue=queue->next;

        for(int i =0;i<timeQuantam;i++){
            current->remainingTime--;
            usleep(500000);
        }
        if(processRequest(current->request)){
            send_response(current->request, 1, "Transaction Complete");
        }else{
            send_response(current->request, 1, "Transaction Complete");
        }
        free(current);

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
        write(fd,&res,sizeof(BankRequest));
        close(fd);
    }

}

typedef struct{
    int accountID;
    char name[50];
    int priority;
    double balance;
    Customer *next;
}Customer;


struct Bank{
    int balance;
    int customerCount=0;
    int size=1000;
   
};
Customer *customers=NULL;

void addCustomer(int accountID,char name[50],int priority,double balance=0){
    Customer newCustomer={accountID,name,priority,balance};
    if(!customers){
    customers=newCustomer;
    }else{
        Customer *temp;
        while(temp->next!=NULL){
            if(temp->accountID==accountID){
                print("Customer already exists");
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
    while(temp->next!=NULL){
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
        pthread_mutex_lock(&lock);
        c->balance-=amount;
        unlock(&lock);
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
        bank.addCustomer(req.accountID,req.name,req.priority,req.amount);
        return 1;

    }else if(!strcmp(req.requestType,"LOGIN"))
    {
        if(searchAccounts(req.accountID))
        {
            loggedInAccountID=accountID;
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
            return;
        }
        BankRequest req;
        read(fd_request,&req,sizeof(BankRequest));
        enqueue(req);
        close(fd_request);
    }

    return NULL;
}


int main(){
    mkfifo(REQUEST_PIPE, 0666);
    mkfifo(RESPONSE_PIPE, 0666);

    pthread_t readingThread,processThread;
    pthread_create(&readingThread,NULL,readRequests,NULL);
    pthread_create(&processThread,NULL,processThread,NULL);

    pthread_join(&readingThread);
    pthread_join(&processThread);


    return 0;
}