#include<string.h>
#include<common.h>
#include<unistd.h>

int accountNum=0;


void sendCustomerToBank(Customer c){
    int fd=open(PIPE_NAME, O_WRONLY);
    if(fd==-1){
        perror("BANK IS NOT OPENn\n");
        return;
    }

    write(fd,&c,sizeof(Customer));
    close(fd);
    printf("Request has been sent to the bank to register customer\n");
}

Customer createCustomer(){
    int accNum=accountNUm;
    accountNum++;
    string name,customerType;
    scanf("Enter Customer Name: ",&name);
    int choice;
    printf("\nChoose customer type: \n");
    printf("1.Regular\n2.Premium\n3.VIP\n");
    scanf("Choice: ",&choice);
    switch (choice){
       case 1:customerType="Reguler";break;
       case 2:customerType="Premium";break;
       case 3:customerType="VIP";break;
       default:printf("Invalid Choice");return NULL;

    }
    scanf("Enter Cusomter Type: ",&customerType);
    int priority;
    if(customerType=="Regular"){
        priority=1;
    }else if(customerType=="Premium"){
        priority=2;
    }else if(customerType=="VIP"){
        priority=3;
    }else{
        printf("Invalid Customer Type(error)\n");
        return NULL;
    }
    Customer c{accNum,priority,name,customerType};
    return c;
}

void menu(){}

void sendLoginToBank(int accID){
    int fd=open(PIPE_NAME, O_WRONLY);
    if(fd==-1){
        perror("BANK IS NOT OPEN");
        return;
    }

    write(fd,&accID,sizeof(int));
    close(fd);
}
void login(){
    int accID;
    scanf(&accID);
    sendLoginToBank(accID);

    printf("1.Deposit\n");
    printf("2.Withdraw\n");
    printf("3.Loan\n");
    printf("4.exit\n");
    int choice;
    while(1){
        switch(choice){
            case 1:
                t=createTransaction("DEPOSIT",accID,amount,priority);

            break;
            case 2:
                t=createTransaction("WITHDRAW",accID,amount,priority);

            break;
            case 3:
                t=createTransaction("LOAN",accID,amount,priority);

            break;
            case 4:
            break;
            default:
            break;
        }
    }
}
void registerCustomer(){}

void sendTransactToBank(Transaction t){
    int fd=open(PIPE_NAME, O_WRONLY);
    if(fd==-1){
        perror("BANK IS NOT OPEN");
        return;
    }

    write(fd,&t,sizeof(Transaction));
    close(fd);
    printf("Request has been sent to the bank");
}
void requestLoan(){}

void requestPayRoll(){}

void requestDeposit(int amount){}
void requestWithdrawal(int amount){}
Transaction createTransaction(string transType,int accId,double amnt,int prty,int payrollCnt=0){
    Transaction t;
    t.accountID=accId;
    t.amount=amnt;
    t.burstTime=BURST_TIME;
    if(transType =="Payroll"){t.payrollCount=payrollCnt;}
    t.priority=prty;
    t.transactionType=transType;
    return t;
}

int main(){
    int choice;
    while(1){
        
    printf("1.Register new account\n");
    printf("2.Login Account\n");
    print("3.exit\n");

    scanf(&choice);
        switch(choice){
            case 1:
                registerCustomer();
            break;
            case 2:
                login();
            break;
            case 3:
                return 0;
           
            default:
                printf("Invalid Input\n");
            break;

        }
    }
    return 0;
}