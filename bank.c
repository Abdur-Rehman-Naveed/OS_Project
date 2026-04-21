#include<common.h>




typedef struct{
    int accountID;
    char name[50];
    int priority;
    double balance;
}Customer;


struct Bank{
    int balance;
    Customer *customers;
    int customerCount=0;
    int size=1000;
    Bank(){
        customers=malloc(sizeof(Customer)*size);
    }
    void resize(){
        int oSize=size;
        int size=size*2;
        Customer *newCustomers=malloc(sizeof(Customer)*size);
        for(int i =0;i<customerCount;i++){
            newCustomers[i]=customers[i];
        }
        //line to deallocate cant remember syntax right now
        customers=newCustomers;
    }
    void addCustomer(int accountID,char name[50],int priority,double balance=0){
        if(customerCount==size){
            resize();
        }
        Customer c={accountID,name,priority,balance};
        customers[customerCount++]=c;
    }
};


void FCFS(){}
void Priority(){}
void RoundRobin(){}



void processRequest(){
    BankResponse res;
    int fd_request= open(REQUEST_PIPE,O_RDONLY);
    if(fd_request==-1){
        perror("PIPE NOT OPEN");
        return;
    }
    BankRequest req;
    read(fd_request,&req,sizeof(BankRequest));
    close(fd_request);

    if(req.requestType=="LOGIN"){
        bank.addCustomer(req.accountID,req.name,req.priority,req.amount);


    }else if(req.requestType==""){
        if(req.transactionType==""){

        }else if(req.requestType==""){

        }else if(req.requestType==""){
            
        }else{

        }
    }else{

    }
    
}



int main(){



    return 0;
}