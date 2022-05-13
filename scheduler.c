//TODO: Remove this flag after creating end shared memory for testing
int flag = 1;

#include "SRTN.h"

// global variables
int msgq_id;
int algorithm;
int scheduler_pGenerator_sem;
struct msgbuff message;
struct PQueue *priority_queue;
struct Queue *queue;

void getProcess(int);

void printQueue(int);

struct ProcessStruct *create_process
        (int id, int arrivalTime, int priority, int runTime,
         bool running, bool startedBefore, int enterQueue,
         int quitQueue, int executionTime, int waitingTime, int pid) {
    struct ProcessStruct *process = (struct ProcessStruct *) malloc(sizeof(struct ProcessStruct));
    process->id = id;
    process->arrivalTime = arrivalTime;
    process->priority = priority;
    process->runTime = runTime;
    process->running = running;
    process->startedBefore = startedBefore;
    process->enterQueue = enterQueue;
    process->quitQueue = quitQueue;
    process->executionTime = executionTime;
    process->waitingTime = waitingTime;
    process->pid = pid;
    return process;
}

void intializeMessageQueue() {
    msgq_id = msgget(PROSCH, 0666 | IPC_CREAT);

    if (msgq_id == -1) {
        perror("Error in create message queue");
        exit(-1);
    }
}

void initializeSemaphore() {

    key_t semkey = ftok("sem.txt", 70);

    // Create a semaphore to synchronize the scheduler with process generator
    scheduler_pGenerator_sem = semget(semkey, 1, 0666 | IPC_CREAT);

    // Check is semaphore id is -1
    if (scheduler_pGenerator_sem == -1) {
        perror("Error in creating semaphores");
        exit(-1);
    }
    printf("Scheduler: Semaphore created with id: %d\n", scheduler_pGenerator_sem);
}

int main(int argc, char *argv[]) {

    //add signal handler to get the processes from process_generator
    signal(SIGUSR1, getProcess);
    signal(SIGTRAP, printQueue);


    //TODO implement the scheduler :)
    initClk();

    //Initialize the semaphore
    initializeSemaphore();

    //initialize the message queue
    intializeMessageQueue();


    //get the algorithm number
    algorithm = atoi(argv[1]);

    switch (algorithm) {
        case 1:
            // Allocate the priority queue
            priority_queue = createPriorityQueue();

            // TODO: Add your algorithm call here (HPF)

            break;
        case 2:
            // Allocate the priority queue
            priority_queue = createPriorityQueue();

            // Call the algorithm function
            SRTN(priority_queue);

            break;
        case 3:
            // Allocate the queue
            queue = createQueue();

            // TODO: Add your algorithm call here (RR)

            break;
    }

    printf("\n\n===================================scheduler Terminated at time = %d===================================\n\n",
           getClk());

    // TODO: Check its logic
    // Destroy your clock
    destroyClk(true);
    return 0;
}

void add_to_SRTN_queue(struct ProcessStruct process) {
    // Push to the queue and the priority is the runTime (The remaining time at the beginning)
    struct ProcessStruct *newProcess = create_process(process.id, process.arrivalTime, process.priority,
                                                      process.runTime, process.running,
                                                      process.startedBefore, process.enterQueue, process.quitQueue,
                                                      process.executionTime,
                                                      process.waitingTime, process.pid);
    push(priority_queue, newProcess, newProcess->runTime);
}

void getProcess(int signum) {
    //receive from the message queue and add to the ready queue
    int rec_val = msgrcv(msgq_id, &message, sizeof(message.process), 7, !IPC_NOWAIT);

    printf("message received: %d\n", message.process.id);
    fflush(stdout);
    if (rec_val == -1) {
        perror("Error in receive");
    }
    // For testing only
    //__SRTN_print_process_info(&message.process);
    switch (algorithm) {
        case 1:
            // TODO: Add to [PRIORITY QUEUE] as HPF

            break;
        case 2:
            // DONE: Add to priority queue as SRTN
            add_to_SRTN_queue(message.process);
        case 3:
            // TODO: Add to [QUEUE] as RR

            break;
    }

    // Process has been pushed to the queue
    // Up the semaphore to allow process generator to continue
    up(scheduler_pGenerator_sem);

    // message.process.id == <id of last process> => Last process is not valid
    // This is only for testing
    // Until we make a shared memory for terminating
    // Use the flag as a condition of your main loop
    //TODO: Remove this condition after creating end shared memory for testing
    if (message.process.id == -1) {
        flag = 0;
    }
}

void printQueue(int sigNum) {
    printf("I have recieved signal %d\n", sigNum);
    struct PQNode *start = priority_queue->head;
    while (start != NULL) {
        __SRTN_print_process_info(start->data);
        start = start->next;
    }
}