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
            send_response(current->request, 1, "Transaction Complete");
        }
        free(current);

    }
}

void send_response(BankRequest r,int success,char msg[]){

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



int processRequest(BankRequest r){
    /*
    BankResponse res;
    int fd_request= open(REQUEST_PIPE,O_RDONLY);
    if(fd_request==-1){
        perror("PIPE NOT OPEN");
        return;
    }
    BankRequest req;
    read(fd_request,&req,sizeof(BankRequest));
    close(fd_request);
    */
    if(!strcmp(r.requestType,"REGISTER_ACCOUNT")){
        bank.addCustomer(req.accountID,req.name,req.priority,req.amount);
        return 1;

    }else if(!strcmp(req.requestType,"LOGIN")){
        
    }else if(!strcmp(req.requestType,"TRANSACTION")){
        if(!strcmp(req.transactionType,"DEPOSIT")){

        }else if(!strcmp(req.transactionType,"WITHDRAW")){

        }else if(!strcmp(req.transactionType,"LOAN")){
            
        }else{

        }
    }else{
        return 0;
    }
    
}



int main(){



    return 0;
}