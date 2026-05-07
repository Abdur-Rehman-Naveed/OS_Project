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
    int turnaroundTime;
    int waitingTime;
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
    newNode->arrivalTime=Time;
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
void log_metrics(int id, int arrival, int burst, int completion) {
    FILE *f = fopen("metrics_log.txt", "a");
    if (f) {
        fprintf(f, "%d,%d,%d,%d\n", id, arrival, burst, completion);
        fclose(f);
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
        int runTime = (current->remainingTime < timeQuantam) ? current->remainingTime : timeQuantam;

        for(int i =0;i<runTime;i++){
            current->remainingTime--;
            Time++;
            usleep(500000);
        }
        if(current->remainingTime<=0){
            current->completionTime=Time;
            if(processRequest(current->request)){
                send_response(current->request, 1, "Transaction Complete");
            }else{
                send_response(current->request, 1, "Transaction Complete");
            }
            log_metrics(current->request.accountID, current->arrivalTime, BURST_TIME, current->completionTime);
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
            Time++;
        }
        current->completionTime=Time;
        if(processRequest(current->request)){
            send_response(current->request, 1, "Transaction Complete");
        }else{
            send_response(current->request, 0, "Transaction failed");
        }
        log_metrics(current->request.accountID, current->arrivalTime, BURST_TIME, current->completionTime);
        free(current);

    }
}
void scheduler_Priority(){

    while (1) {
        if (!queue) {
            sleep(1);
            continue;
        }

        Node *current = queue;
        queue = queue->next;

        for (int i = 0; i < BURST_TIME; i++) {
            Time++;
            current->remainingTime--;
            
            if (queue && queue->request.priority > current->request.priority) {
                break;
            }
            if (current->remainingTime <= 0) break;
        }

        if (current->remainingTime > 0) {
            rrEnqueue(current);
        } else {
            if(processRequest(current->request)) {
                send_response(current->request, 1, "Transaction Complete");
            }
            free(current);
        }
    }

}

void* metrics_analyzer(void* arg) {
    while(1) {
        sleep(10);
        FILE *f = fopen("metrics_log.txt", "r");
        if (!f) continue;

        printf("\n--- PERFORMANCE METRICS (Every 10s) ---\n");
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
        fclose(clear);
    }
    return NULL;
}
int isSafeState(Customer *requestingCust, double amount) {
    double availableAfterLoan = bank.balance - amount;
    Customer *temp = customers;
    int safe = 0;

    while (temp != NULL) {
        if (availableAfterLoan + temp->balance >= temp->maxNeed) {
            safe = 1;
            break;
        }
        temp = temp->next;
    }
    return safe; 
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

    char header[100];
    fgets(header, sizeof(header), log);

    printf("\n--- GANTT CHART (Execution Flow) ---\n|");
    
    int t, id, p;
    int timeline[1000];
    int count = 0;

    while (fscanf(log, "%d\t%d\t%d", &t, &id, &p) != EOF) {
        printf(" Acc%d |", id);
        timeline[count++] = t;
    }

    printf("\n0");
    for(int i = 0; i < count; i++) {
        printf("      %d", timeline[i]);
    }
    printf("\n------------------------------------\n");
    
    fclose(log);
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
                return;
            }
            temp=temp->next;
        }
        temp->next=newCustomer;
    }
}



int loggedInAccountID;
Customer* searchAccounts(int id){
    Customer *temp=customers;
    while(temp!=NULL){
        if(temp->accountID==id){
            return temp;
        }
        temp=temp->next;
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
        pthread_mutex_lock(&transactionLock);
        c->balance+=amount;
        pthread_mutex_unlock(&transactionLock);
        return 1;
}

typedef struct{
        Customer *c;
        int id;
    }payrollData;

void *payrollThread(void * arg){
    payrollData pd=(payrollData*)arg;
    if(withdraw(pd.c,20)){
        printf("Payroll: Successfully paid Employee id %d\n",pd.id);
    }else{
        printf("Payment failed\n");
    }
    return NULL;
}
void processPayroll(Customer *c,int payrollCount){
    pthread_t threads[payrollCount];
    for(int i=0;i<payrollCount;i++){
        payrollData pd;
        pd.c=c;
        pd.id=i+1;
        pthread_create(&threads[i],NULL,payrollThread,pd);
    }
    for(int i=0;i<payrollCount;i++){
        pthread_join(threads[i],NULL);
    }
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
        }else if(!strcmp(req.transactionType,"PAYROLL"))
        {
            Customer *cust=searchAccounts(req.accountID);
            if(cust)
            {
                processPayroll(cust,req.payrollCount);
                return 1;
            }else
            {
                return 0;
            }
        }
    }else
    {
        return 0;
    }
    
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
                scheduler_RoundRobin();
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

    printf("Choose Your Scheduler:\n 1.FCFS\n2.ROUND ROBIN\n3.PriorityQueue\n");
    int choice;
    printf("Choice: ");
    scanf("%d",&choice);
    switch(choice){
        case 1: 
            pthread_create(&processThread,NULL,processThreads,1);
            break;
        case 2: 
            pthread_create(&processThread,NULL,processThreads,2);
            break;
        case 3: 
            pthread_create(&processThread,NULL,processThreads,3);
            break;
        default:
            printf("Invalid Choice Choosing FCFS automatically\n");
            break;
    }

    pthread_create(&readingThread,NULL,readRequests,NULL);
    pthread_create(&metrics,NULL,metrics_analyzer,NULL);
    pthread_join(metrics_analyzer,NULL);
    pthread_join(readingThread,NULL);
    pthread_join(processThread,NULL);


    return 0;
}