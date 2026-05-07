#include "common.h"




void registerCustomer(){
    BankRequest r;
    strcpy(r.requestType,"REGISTER_ACCOUNT");
    printf("Enter Name: ");
    scanf("%s",r.name);
    printf("Choose Type:(1: Regular, 2:Premium, 3:VIP) ");
    r.amount = 0; // Initialize starting balance
    int choice;
    scanf("%d",&choice);

    switch (choice){
       case 1:strcpy(r.customerType,"Regular");break;
       case 2:strcpy(r.customerType,"Premium");break;
       case 3:strcpy(r.customerType,"VIP");break;
       default: strcpy(r.customerType,"Regular");
    }
    if(!strcmp(r.customerType,"Regular")){
        r.priority=1;
    }else if(!strcmp(r.customerType,"Premium")){
        r.priority=2;
    }else if(!strcmp(r.customerType,"VIP")){
        r.priority=3;
    }else{
        r.priority=1;
    }

    BankResponse response=sendRequestToBank(r);
    if(response.success){
        printf("REQUEST SUCCESSFULL: %s\n", response.msg);
    }
    else{
        printf("REQUEST FAILED: %s\n",response.msg);
    }
}

BankResponse sendRequestToBank(BankRequest request){
    BankResponse response;
    
    // Create unique response pipe name
    sprintf(request.responsePipe, "resp_%d", getpid());
    mkfifo(request.responsePipe, 0666);

    int fd_request=open(REQUEST_PIPE,O_WRONLY);
    if(fd_request==-1){
        perror("REQUEST FAILED: BANK SERVER IS NOT RUNNING.\n");
        response.success=0;
        unlink(request.responsePipe);
        return response;
    }
    write(fd_request,&request,sizeof(BankRequest));
    close(fd_request);

    // Wait for response on our private pipe
    int fd_response=open(request.responsePipe,O_RDONLY);
    if(fd_response==-1){
        perror("RESPONSE FAILED: BANK SERVER IS NOT RESPONDING.\n");
        response.success=0;
        unlink(request.responsePipe);
        return response;
    };
    read(fd_response,&response,sizeof(BankResponse));
    close(fd_response);
    
    // Clean up our private pipe
    unlink(request.responsePipe);
    return response;
}
void login() {
    BankRequest request;
    strcpy(request.requestType ,"LOGIN");
    printf("Enter Account ID to Login: ");
    scanf("%d", &request.accountID);

    BankResponse response = sendRequestToBank(request);

    if (!response.success) {
        printf("\nREQUEST FAILED %s\n", response.msg);
        return;
    }

    printf("\nREQUEST SUCCESSFUL %s\n", response.msg);
    
    int choice;
    while(1) {
        printf("\n1. Deposit\n2. Withdraw\n3. Apply for Loan\n4. Corporate Payroll\n5. Logout\nCHOICE: ");
        scanf("%d", &choice);

        if (choice == 5) break;

        BankRequest trans;
        strcpy(trans.requestType , "TRANSACTION");
        trans.accountID = request.accountID;

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
                strcpy(trans.transactionType , "LOAN");
                printf("Enter Loan Amount requested: "); scanf("%lf", &trans.amount);
                break;
            case 4:
                strcpy(trans.transactionType , "PAYROLL");
                printf("Enter number of employees: "); scanf("%d", &trans.payrollCount);
                break;
            default:
             continue;
        }

        BankResponse transactionResponse = sendRequestToBank(trans);
        printf("\nBANK RESPONSE: %s", transactionResponse.msg);
        if (transactionResponse.success) {
            printf(" New Balance: %f\n", transactionResponse.updatedBalance);
        } else {
            printf("\n");
        }
    }
}


void runAutomatedTests() {
    printf("\n--- STARTING AUTOMATED TESTS ---\n");
    BankRequest r;
    r.amount = 0;
    
    // 1. Test Registration
    printf("[Test 1] Registering Test Accounts...\n");
    strcpy(r.requestType, "REGISTER_ACCOUNT");
    r.accountID = 999;
    strcpy(r.name, "Test_User");
    r.priority = 2; // Premium
    sendRequestToBank(r);

    // 2. Test Deposit
    printf("[Test 2] Testing Deposit of $1000...\n");
    strcpy(r.requestType, "TRANSACTION");
    strcpy(r.transactionType, "DEPOSIT");
    r.amount = 1000.0;
    BankResponse res = sendRequestToBank(r);
    printf("Result: %s, New Balance: %.2f\n", res.msg, res.updatedBalance);

    // 3. Test Withdrawal
    printf("[Test 3] Testing Withdrawal of $500...\n");
    strcpy(r.transactionType, "WITHDRAW");
    r.amount = 500.0;
    res = sendRequestToBank(r);
    printf("Result: %s, New Balance: %.2f\n", res.msg, res.updatedBalance);

    // 4. Test Over-Withdrawal
    printf("[Test 4] Testing Over-Withdrawal ($2000)...\n");
    r.amount = 2000.0;
    res = sendRequestToBank(r);
    printf("Result: %s (Expected Failure)\n", res.msg);

    // 5. Test Loan (Banker's Algorithm)
    printf("[Test 5] Testing Loan Request $5000 (Safe)...\n");
    strcpy(r.transactionType, "LOAN");
    r.amount = 5000.0;
    res = sendRequestToBank(r);
    printf("Result: %s\n", res.msg);

    printf("\n--- TESTS COMPLETED ---\n");
}

int main(){
    int choice;
    mkfifo(REQUEST_PIPE, 0666);
    mkfifo(RESPONSE_PIPE, 0666);

    while(1){
    printf("\nBANK MENU\n");
    printf("1. Register new account\n");
    printf("2. Login Account\n");
    printf("3. Run Automated Tests\n");
    printf("4. Exit\nChoose: ");

    scanf("%d",&choice);
        switch(choice){
            case 1: registerCustomer();break;
            case 2: login();break;
            case 3: runAutomatedTests(); break;
            case 4: return 0;
            default:
                printf("Invalid Choice\n");
            break;

        }
    }
    return 0;
}