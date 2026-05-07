#include "common.h"



int timeQuantam=BURST_TIME;
int Time=0;
pthread_mutex_t transactionLock;
pthread_mutex_t bankLock;
sem_t transactionSemaphore;
pthread_mutex_t queueLock;
pthread_mutex_t customersLock;


typedef struct Node{
    BankRequest request;
    int arrivalTime;
    int startTime;
    int completionTime;
    int remainingTime;
    int turnaroundTime;
    int waitingTime;
    struct Node* next;
}Node;

typedef struct Customer{
    int accountID;
    char name[50];
    int priority;
    double balance;
    int maxNeed;
    struct Customer *next;
}Customer;

BankResponse processRequest(BankRequest req);
void send_response(BankRequest r, BankResponse res);
int addCustomer(int accountID, char name[50], int priority, double balance, int maxNeed);
int deposit(Customer *c, double amount);
int withdraw(Customer *c, double amount);
Customer* searchAccounts(int id);
int processPayroll(Customer *c, int payrollCount);




typedef struct{
    double balance;
    int customerCount;   
}Bank;
Customer *customers=NULL;
Node *queue=NULL;
Bank bank = {1000000.0, 0};

void enqueue(BankRequest r){
    Node *newNode = (Node*)malloc(sizeof(Node));
    newNode->request=r;
    newNode->next=NULL;
    newNode->remainingTime=BURST_TIME;
    newNode->arrivalTime=Time;
    
    pthread_mutex_lock(&queueLock);
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
    pthread_mutex_unlock(&queueLock);
}
void rrEnqueue(Node* n){
    pthread_mutex_lock(&queueLock);
    if(!queue || n->request.priority>=queue->request.priority){
        n->next=queue;
        queue=n;
    }else{
        Node *temp=queue;
        while(temp->next!=NULL && temp->next->request.priority>=n->request.priority){
            temp=temp->next;
        }
        n->next=temp->next;
        temp->next=n;
    }
    pthread_mutex_unlock(&queueLock);
}
void log_metrics(int id, int arrival, int burst, int completion) {
    FILE *f = fopen("metrics_log.txt", "a");
    if (f) {
        fprintf(f, "%d,%d,%d,%d\n", id, arrival, burst, completion);
        fclose(f);
    }
}

void scheduler_RoundRobin(){
    FILE *log = fopen("scheduler_log.txt", "a");
    while(1){
        pthread_mutex_lock(&queueLock);
        if(!queue){
            pthread_mutex_unlock(&queueLock);
            sleep(1);
            continue;
        }

        Node *current=queue;
        queue=queue->next;
        pthread_mutex_unlock(&queueLock);

        if (log) {
            fprintf(log, "%d\t%d\t%d\n", Time, current->request.accountID, current->request.priority);
            fflush(log);
        }

        int runTime = (current->remainingTime < timeQuantam) ? current->remainingTime : timeQuantam;

        for(int i =0;i<runTime;i++){
            current->remainingTime--;
            Time++;
            usleep(500000);
        }
        if(current->remainingTime<=0){
            current->completionTime=Time;
            BankResponse res = processRequest(current->request);
            send_response(current->request, res);
            log_metrics(current->request.accountID, current->arrivalTime, BURST_TIME, current->completionTime);
            free(current);
        }else{
            rrEnqueue(current);
        }        
    }
    if (log) fclose(log);
}

void scheduler_FCFS(){
    FILE *log = fopen("scheduler_log.txt", "a"); 
    while(1){
        pthread_mutex_lock(&queueLock);
        if(!queue){
            pthread_mutex_unlock(&queueLock);
            sleep(1);
            continue;
        }

        Node *current=queue;
        queue=queue->next;
        pthread_mutex_unlock(&queueLock);

        if (log) {
            fprintf(log, "%d\t%d\t%d\n", Time, current->request.accountID, current->request.priority);
            fflush(log); 
        }
        int burst = current->remainingTime;
        for(int i =0;i<burst;i++){
            current->remainingTime--;
            usleep(500000);
            Time++;
        }
        current->completionTime=Time;
        BankResponse res = processRequest(current->request);
        send_response(current->request, res);
        log_metrics(current->request.accountID, current->arrivalTime, BURST_TIME, current->completionTime);
        free(current);

    }
    if (log) fclose(log);
}

void scheduler_Priority() {
    FILE *log = fopen("scheduler_log.txt", "a");
    while (1) {
        pthread_mutex_lock(&queueLock);
        if (!queue) {
            pthread_mutex_unlock(&queueLock);
            sleep(1);
            continue;
        }

        Node *current = queue;
        queue = queue->next;
        pthread_mutex_unlock(&queueLock);

        fprintf(log, "%d\t%d\t%d\n", Time, current->request.accountID, current->request.priority);
        fflush(log);

        usleep(500000);
        Time++;
        current->remainingTime--;

        if (current->remainingTime > 0) {
            rrEnqueue(current);
        } else {
            BankResponse res = processRequest(current->request);
            send_response(current->request, res);
            log_metrics(current->request.accountID, current->arrivalTime, BURST_TIME, Time);
            free(current);
        }
    }
    fclose(log);
}

void* metrics_analyzer(void* arg) {
    FILE *f = fopen("metrics_log.txt", "r");
    if (!f) {
        printf("No metrics logged yet.\n");
        return NULL;
    }

    printf("\n--- PERFORMANCE METRICS ---\n");
    printf("ID\tArrival\tBurst\tFinish\tTAT\tWT\n");
    
    int id, arr, burst, comp;
    double totalWT = 0, totalTAT = 0, count = 0;

    while (fscanf(f, "%d,%d,%d,%d", &id, &arr, &burst, &comp) == 4) {
        int tat = comp - arr;
        int wt = tat - burst;
        printf("%d\t%d\t%d\t%d\t%d\t%d\n", id, arr, burst, comp, tat, wt);
        
        totalTAT += tat;
        totalWT += wt;
        count++;
    }
    fclose(f);

    if (count > 0) {
        printf("---------------------------------------\n");
        printf("Avg Turnaround Time: %.2f\n", totalTAT / count);
        printf("Avg Waiting Time: %.2f\n", totalWT / count);
    }
    printf("---------------------------------------\n");
    
    FILE *clear = fopen("metrics_log.txt", "w");
    if(clear) fclose(clear);

    return NULL;
}
int isSafeState(Customer *requestingCust, double amount) {
    pthread_mutex_lock(&customersLock);
    double work = bank.balance - amount;
    
    int customerCount = 0;
    Customer *temp = customers;
    while(temp) { customerCount++; temp = temp->next; }
    
    int *finish = calloc(customerCount, sizeof(int));
    int completed = 0;

    while (completed < customerCount) {
        int found = 0;
        int i = 0;
        temp = customers;
        
        while (temp) {
            double currentNeed = temp->maxNeed - temp->balance;
            
            if (temp->accountID == requestingCust->accountID) {
                currentNeed -= amount; 
            }

            if (!finish[i] && currentNeed <= work) {
                work += temp->balance;
                if (temp->accountID == requestingCust->accountID) work += amount;
                
                finish[i] = 1;
                found = 1;
                completed++;
            }
            temp = temp->next;
            i++;
        }
        if (!found) break;
    }

    free(finish);
    pthread_mutex_unlock(&customersLock);
    return (completed == customerCount);
}
int applyLoan(Customer *c,double amount){
    pthread_mutex_lock(&bankLock);
    if(amount<=bank.balance && isSafeState(c,amount)){
        bank.balance-=amount;
        deposit(c,amount);
        pthread_mutex_unlock(&bankLock);
        return 1;
    }else{
        pthread_mutex_unlock(&bankLock);
        return 0;
    }
}




void printGanttChart() {
    FILE *log = fopen("scheduler_log.txt", "r");
    if (!log) {
        printf("No scheduler logs found.\n");
        return;
    }

    printf("\n"
           "╔═══════════════════════════════════════════════════════════════╗\n"
           "║                      GANTT CHART VISUAL                       ║\n"
           "╚═══════════════════════════════════════════════════════════════╝\n");
    
    int t, id, p;
    int timeline[1000];
    int ids[1000];
    int count = 0;

    while (fscanf(log, "%d\t%d\t%d", &t, &id, &p) != EOF && count < 1000) {
        timeline[count] = t;
        ids[count] = id;
        count++;
    }
    fclose(log);

    if (count == 0) {
        printf("No data to display.\n");
        return;
    }

    // Top border
    printf("┌");
    for(int i=0; i<count; i++) printf("──────┬");
    printf("\n│");

    // Middle content (Account IDs)
    for(int i=0; i<count; i++) {
        printf(" A%d  │", ids[i]);
    }
    printf("\n├");

    // Bottom border
    for(int i=0; i<count; i++) printf("──────┼");
    printf("\n0");

    // Time markers
    for(int i=0; i<count; i++) {
        printf("%7d", timeline[i]);
    }
    printf("\n\n");
}


void send_response(BankRequest r, BankResponse res){
    int fd=open(r.responsePipe,O_WRONLY);
    if(fd!=-1){
        write(fd,&res,sizeof(BankResponse));
        close(fd);
    }
}


int addCustomer(int accountID,char name[50],int priority,double balance,int maxNeed){
    Customer *newCustomer=(Customer*)malloc(sizeof(Customer));
    
    pthread_mutex_lock(&customersLock);
    if (accountID == -1) {
        bank.customerCount++;
        accountID = 1000 + bank.customerCount; // Start IDs from 1001
    }
    
    newCustomer->accountID=accountID;
    strcpy(newCustomer->name,name);
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
                pthread_mutex_unlock(&customersLock);
                free(newCustomer);
                return -1;
            }
            temp=temp->next;
        }
        if(temp->accountID==accountID){
            printf("Customer already exists");
            pthread_mutex_unlock(&customersLock);
            free(newCustomer);
            return -1;
        }
        temp->next=newCustomer;
    }
    pthread_mutex_unlock(&customersLock);
    return accountID;
}



int loggedInAccountID;
Customer* searchAccounts(int id){
    pthread_mutex_lock(&customersLock);
    Customer *temp=customers;
    while(temp!=NULL){
        if(temp->accountID==id){
            pthread_mutex_unlock(&customersLock);
            return temp;
        }
        temp=temp->next;
    }
    pthread_mutex_unlock(&customersLock);
    return NULL;
}

int withdraw(Customer *c,double amount)
{
    pthread_mutex_lock(&transactionLock);
    if(c->balance>=amount)
    {
        c->balance-=amount;
        pthread_mutex_unlock(&transactionLock);
        return 1;
    }else
    {
        pthread_mutex_unlock(&transactionLock);
        return 0;
    }
}
int deposit(Customer *c,double amount)
{   
        pthread_mutex_lock(&transactionLock);
        c->balance+=amount;
        pthread_mutex_unlock(&transactionLock);
        return 1;
}

typedef struct{
        Customer *c;
        int id;
        int *successCount;
        pthread_mutex_t *countLock;
    }payrollData;

void *payrollThread(void * arg){
    payrollData *pd=(payrollData*)arg;
    if(withdraw(pd->c,20)){
        pthread_mutex_lock(pd->countLock);
        (*pd->successCount)++;
        pthread_mutex_unlock(pd->countLock);
        printf("Payroll: Successfully paid Employee id %d\n",pd->id);
    }else{
        printf("Payment failed for Employee id %d\n",pd->id);
    }
    return NULL;
}
int processPayroll(Customer *c,int payrollCount){
    if (payrollCount <= 0) return 0;
    
    // Safety limit to prevent system-wide crash
    if (payrollCount > 10000) {
        printf("Warning: Payroll count too high (%d). Capping at 10,000 for safety.\n", payrollCount);
        payrollCount = 10000;
    }

    int successCount = 0;
    pthread_mutex_t countLock;
    pthread_mutex_init(&countLock, NULL);

    // Process in batches of 100 to avoid resource exhaustion
    int batchSize = 100;
    for (int i = 0; i < payrollCount; i += batchSize) {
        int currentBatch = (i + batchSize > payrollCount) ? (payrollCount - i) : batchSize;
        
        pthread_t *threads = malloc(sizeof(pthread_t) * currentBatch);
        payrollData *pd = malloc(sizeof(payrollData) * currentBatch);

        if (!threads || !pd) {
            perror("Failed to allocate memory for payroll threads");
            if (threads) free(threads);
            if (pd) free(pd);
            break;
        }

        for (int j = 0; j < currentBatch; j++) {
            pd[j].c = c;
            pd[j].id = i + j + 1;
            pd[j].successCount = &successCount;
            pd[j].countLock = &countLock;
            if (pthread_create(&threads[j], NULL, payrollThread, &pd[j]) != 0) {
                perror("Thread creation failed");
                // Stop creating threads in this batch if limit reached
                currentBatch = j; 
                break;
            }
        }
        
        for (int j = 0; j < currentBatch; j++) {
            pthread_join(threads[j], NULL);
        }
        
        free(threads);
        free(pd);
    }

    pthread_mutex_destroy(&countLock);
    return successCount;
}

BankResponse processRequest(BankRequest req){
    BankResponse res;
    res.success = 0;
    res.accountID = req.accountID;
    res.updatedBalance = 0;
    strcpy(res.msg, "");
    
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
        int newID = addCustomer(-1,req.name,req.priority,req.amount,maxNeed);
        if (newID != -1) {
            res.success = 1;
            res.accountID = newID;
            strcpy(res.msg, "Account Registered Successfully");
        } else {
            strcpy(res.msg, "Registration Failed");
        }
        return res;

    }else if(!strcmp(req.requestType,"LOGIN"))
    {
        Customer *c = searchAccounts(req.accountID);
        if(c)
        {
            loggedInAccountID=req.accountID;
            res.success = 1;
            res.updatedBalance = c->balance;
            strcpy(res.msg, "Login Successful");
        }else
        {
            strcpy(res.msg, "Invalid Account ID");
        }
        return res;
        
    }else if(!strcmp(req.requestType,"TRANSACTION"))
    {
        sem_wait(&transactionSemaphore);
        Customer *cust=searchAccounts(req.accountID);
        int result=0;
        if(cust){
            if (!strcmp(req.transactionType, "DEPOSIT")) {
                result = deposit(cust, req.amount);
                strcpy(res.msg, "Deposit Successful");
            } 
            else if (!strcmp(req.transactionType, "WITHDRAW")) {
                result = withdraw(cust, req.amount);
                if (result) strcpy(res.msg, "Withdrawal Successful");
                else strcpy(res.msg, "Insufficient Balance");
            } 
            else if (!strcmp(req.transactionType, "LOAN")) {
                result = applyLoan(cust, req.amount);
                if (result) strcpy(res.msg, "Loan Approved (Safe State)");
                else strcpy(res.msg, "Loan Denied (Unsafe State or Insufficient Bank Funds)");
            } 
            else if (!strcmp(req.transactionType, "PAYROLL")) {
                int paid = processPayroll(cust, req.payrollCount);
                result = 1;
                sprintf(res.msg, "Payroll Processed: %d/%d Employees Paid", paid, req.payrollCount);
            }
            
            if (result) {
                res.success = 1;
                res.updatedBalance = cust->balance;
            }
        } else {
            strcpy(res.msg, "Customer Not Found");
        }
        sem_post(&transactionSemaphore);
        return res;
    }
    return res;   
}
void* readRequests(void* arg){
    int fd_request= open(REQUEST_PIPE,O_RDONLY);
    if(fd_request==-1){
        perror("PIPE NOT OPEN");
        return NULL;
    }
    BankRequest req;
    while(1){
        if(read(fd_request,&req,sizeof(BankRequest))>0){
            enqueue(req);
        }
    }
    close(fd_request);
    return NULL;
}
void* processThreads(void* arg){
    int choice=*(int*)arg;
    switch(choice){
        case 1: 
            while(1){
                scheduler_FCFS();
                sleep(1);
            }
            break;
        case 2: 
           while(1){
                scheduler_RoundRobin();
                sleep(1);
            }
            break;
        case 3: 
            while(1){
                scheduler_Priority();
                sleep(1);
            }
            break;
        default:
            printf("Invalid Choice Choosing FCFS automatically\n");
            break;
    }

    return NULL;
}


int main(){
    pthread_t readingThread,processThread,metrics;
    mkfifo(REQUEST_PIPE, 0666);
    mkfifo(RESPONSE_PIPE, 0666);
    pthread_mutex_init(&transactionLock,NULL);
    pthread_mutex_init(&queueLock, NULL);
    pthread_mutex_init(&customersLock, NULL);
    sem_init(&transactionSemaphore, 0, 2);
    FILE *f = fopen("scheduler_log.txt", "w");

    printf("Choose Your Scheduler:\n 1.FCFS\n2.ROUND ROBIN\n3.PriorityQueue\n");
    int choice;
    printf("Choice: ");
    scanf("%d",&choice);
    switch(choice){
        case 1: 
            pthread_create(&processThread,NULL,processThreads,&choice);
            break;
        case 2: 
            pthread_create(&processThread,NULL,processThreads,&choice);
            break;
        case 3: 
            pthread_create(&processThread,NULL,processThreads,&choice);
            break;
        default:
            printf("Invalid Choice Choosing FCFS automatically\n");
            break;
    }

    pthread_create(&readingThread,NULL,readRequests,NULL);
    
    int running = 1;
    while(running) {
        printf("\nBANK SERVER CONTROL\n");
        printf("1. Show Performance Metrics\n");
        printf("2. Show Gantt Chart\n");
        printf("3. Shutdown Server\n");
        printf("Choice: ");
        int m_choice;
        if(scanf("%d", &m_choice) != 1) break;

        switch(m_choice) {
            case 1:
                metrics_analyzer(NULL);
                break;
            case 2:
                printGanttChart();
                break;
            case 3:
                running = 0;
                break;
            default:
                printf("Invalid choice.\n");
        }
    }

    printf("Shutting down...\n");
    fclose(f);
    return 0;
}