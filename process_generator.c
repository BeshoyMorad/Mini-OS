/*
Code for chosen scheduling algorithm:
=====================================
HPF --> 1
SRTN --> 2
RR --> 3
*/
#include "headers.h"

void clearResources(int);

void readFile(struct Queue* processQueue) {
    FILE * file = fopen("processes.txt", "r");

    int id, arrivaltime, runningtime, priority;
    //read the first line (commented one)
    char s[30];
    fgets(s, 30, file);

    //start reading the file
    while (fscanf(file, "%d\t%d\t%d\t%d", &id, &arrivaltime, &runningtime, &priority) != EOF)
    {
        struct ProcessStruct* p = (struct ProcessStruct*)malloc(sizeof(struct ProcessStruct));
        p->id = id;
        p->arrivalTime = arrivaltime;
        p->priority = priority;
        p->runTime = runningtime;
        p->running = false;
        p->startedBefore = false;
        p->enterQueue = 0;
        p->quitQueue = 0;
        p->executionTime = 0;
        p->waitingTime = 0;
        
        enQueue(processQueue, p);
    }
    fclose(file);
}

int getSchedulingAlgorithm() {
    int algorithm;
    printf("Which scheduling algorithm do you want?\n");
    printf("=======================================\n");
    printf("[1] Non-preemptive Highest Priority First (HPF)\n");
    printf("[2] Shortest Remaining time Next (SRTN)\n");
    printf("[3] Round Robin (RR)\n");
    printf("=======================================\n");
    scanf("%d", &algorithm);
    while (algorithm > 3 || algorithm < 1)
    {
        printf("Enter a number from 1 to 3\n");
        scanf("%d", &algorithm);
    }
    
    return algorithm;
}

void sendProcess(struct ProcessStruct* process) {

}

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    struct Queue* processQueue = createQueue();
    readFile(processQueue);

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    int algorithm = getSchedulingAlgorithm();

    // 3. Initiate and create the scheduler and clock processes.
    int clkpid = fork();
    if (clkpid == 0) {
        //run the clk file
        char* args[] = {"./clk.o", NULL};
        execv(args[0], args);
    }

    int schedulerpid = fork();
    if (schedulerpid == 0) {
        //run the scheduler file
        int length = snprintf( NULL, 0, "%d", algorithm );
        char* str = malloc( length + 1 );
        snprintf( str, length + 1, "%d", algorithm );
        char* args[] = {"./scheduler.o", str, NULL};
        execv(args[0], args);
    }
    // 4. Use this function after creating the clock process to initialize clock
    sleep(1);
    initClk();
    // To get time use this
    int clk = getClk();
    printf("current time is %d\n", clk);

    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    while (!isEmptyN(processQueue))
    {
        struct ProcessStruct* process = peekN(processQueue);

        //get the current time to check with the arrival time of the process
        clk = getClk();
        while (process->arrivalTime != clk)
            clk = getClk();
        
        printf("Process with id=%d arrived at %d\n", process->id, clk);
        //send a signal to the schedular to be ready for the coming process
        kill(schedulerpid, SIGUSR1);

        //send the process to the schedular
        sendProcess(process);

        //remove it from the queue
        deQueue(processQueue);
    }
    
    //wait for the schedular to finish before clearing the clock resources
    waitpid(schedulerpid, NULL, 0);

    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}
