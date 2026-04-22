#include<common.h>




void RegisterCustomer(){
    BankRequest r;
    strcpy(r.requestType,"REGISTER_ACCOUNT");
    printf("Enter Name: ");
    scanf("%s",&r.name);
    printf("Choose Type:(1: Regular, 2:Premium, 3:VIP) ");
    int choice;
    scanf("%d",&choice);

    switch (choice){
       case 1:strcpy(r.requestType,"Reguler");break;
       case 2:strcpy(r.requestType,"Premium");break;
       case 3:strcpy(r.requestType,"VIP");break;
       default: :strcpy(r.requestType,"Reguler");
    }
    if(!strcmp(customerType,"Regular")){
        r.priority=1;
    }else if(!strcmp(customerType,"Premium")){
        r.priority=2;
    }else if(!strcmp(customerType,"VIP")){
        r.priority=3;
    }else{
        r.priority=1;
    }

    BankResponse respone=sendRequestToBank(r);
    if(response.success){
        printf("REQUEST SUCCESSFULL: %s\n", response.message);
    }
    else{
        printf("REQUEST FAILED: %s\n",response.message);
    }
}

BankResponse sendRequestToBank(BankRequest request){
    BankResponse response;
    int fd_request=open(REQUEST_PIPE,O_WRONLY);
    if(fd_request==-1){
        perror("REQUEST FAILED: BANK SERVER IS NOT RUNNING.\n");
        response.success=0;
        return response;
    }
    write(fd_request,&request,sizeof(BankRequest));
    close(fd_request);
    int fd_response=open(RESPONSE_PIPE,O_RDONLY);
    if(fd_response==-1){
        perror("RESPONSE FAILED: BANK SERVER IS NOT RESPONDING.\n");
        response.success=0;
        return response;
    };
    read(fd_response,&response,sizeof(BankResponse));
    close(fd_response);
    return reponse;
}
void login() {
    BankRequest request;
    strcpy(request.request_type ,"LOGIN");
    printf("Enter Account ID to Login: ");
    scanf("%d", &request.accountID);

    BankResponse response = sendRequestToBank(request);

    if (!response.success) {
        printf("\nREQUEST FAILED %s\n", response.message);
        return;
    }

    printf("\nREQUEST SUCCESSFUL %s\n", res.message);
    
    int choice;
    while(1) {
        printf("\n1. Deposit\n2. Withdraw\n3. Apply for Loan\n4. Corporate Payroll\n5. Logout\nCHOICE: ");
        scanf("%d", &choice);

        if (choice == 5) break;

        BankRequest trans;
        strcpy(trans.requestType , "TRANSACTION");
        trans.accountID = req.accountID;

        switch(choice) {
            case 1:
                strcpy(trans.transactionType ,"DEPOSIT");
                printf("Enter Deposit Amount: "); scanf("%lf", &trans.amount);
                break;
            case 2:
                strcpy(trans.transactionType , "WITHDRAW");
                printf("Enter Withdrawal Amount: "); scanf("%lf", &trans.amount);
                break;
            case 3:
                strcpy(trans.trans_type , "LOAN");
                printf("Enter Loan Amount requested: "); scanf("%lf", &trans.amount);
                break;
            case 4:
                strcpy(trans.trans_type , "PAYROLL");
                printf("Enter number of employees: "); scanf("%d", &trans.payrollCount);
                break;
            default:
             continue;
        }

        BankResponse transactionResponse = sendRequestToBank(trans);
        printf("\nBANK RESPONSE: %s", transactionResponse.message);
        if (transactionResponse.success) {
            printf(" New Balance: %f\n", transactionResponse.updatedBalance);
        } else {
            printf("\n");
        }
    }
}


int main(){
    int choice;
    mkfifo(REQUEST_PIPE, 0666);
    mkfifo(RESPONSE_PIPE, 0666);

    while(1){
    printf("\nBANK MENU\n");
    printf("1.Register new account\n");
    printf("2.Login Account\n");
    print("3.exit\nChoose: ");

    scanf("%d",&choice);
        switch(choice){
            case 1: registerCustomer();break;
            case 2: login();break;
            case 3: return 0;
            default:
                printf("Invalid Choice\n");
            break;

        }
    }
    return 0;
}