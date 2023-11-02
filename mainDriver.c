#include "mainHeader.h"

//Global Mutexes
pthread_mutex_t IOlock = PTHREAD_MUTEX_INITIALIZER;

//realtime, nanosecond accuracy
double startTime;

//Q is the number of transactions received and waiting to be consumed.
int Q;
pthread_mutex_t Qlock = PTHREAD_MUTEX_INITIALIZER;


double getCurrentTimeInSeconds() {
    struct timeval time;

    //gettimeofday is thread safe, wrapper around a thread safe system call
    gettimeofday(&time, NULL);

    double seconds = time.tv_sec + (time.tv_usec / 1e6);

    //do not always need to check return value
    return seconds;
}

int main (int argc, char *argv[]) {
    int ret = 0;

    //get start time
    startTime = getCurrentTimeInSeconds();

    if (argc < 3) {
        printf("Usage: ./prodcon <number of consumers> <output file number>\n");
        exit(0);
    }

    //get number of consumers and filename
    numberConsumers = atoi(argv[1]);
    numberProducers = 1;

    int lengthFileString = strlen(argv[2]) + strlen("prodcon.") + strlen(".log");
    char outputFile[lengthFileString + 1];
    outputFile[0] = '\0';

    sprintf(outputFile, "prodcon.%s.log", argv[2]);

    //open output in fileno mode
    int output = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC);

    //redirect stdout to output file, but not stderr, for all threads
    ret = dup2(output, STDOUT_FILENO);
    if (ret == -1) {
        perror("dup2, main");
        exit(1);
    }

    //create job queue
    jobQueue * queue = createJobQueue();

    if (queue == NULL) {
        fprintf(stderr, "Error creating job queue\n");
        exit(1);
    }

    //finish initializing queue
    queue->isFull = false;
    queue->isEmpty = true;
    queue->size = 0;

    //create consumer threads, need to dynamically alocate memory for this, do not acess stack of other threads, can be unsafe

    consumers = calloc(numberConsumers, sizeof(consumerThread));
    if (consumers == NULL) {
        fprintf(stderr, "Error allocating memory for consumer thread array\n");
        exit(1);
    }

    for (int i = 0; i < numberConsumers; i++) {
        consumers[i] = calloc(1, sizeof(consumerThread));
        if (consumers[i] == NULL) {
            fprintf(stderr, "Error allocating memory for consumer thread\n");
            exit(1);
        }

        //dynamically allocate memory for args, threads only accept pointers as args, need to free later
        threadArgs * args = calloc(1, sizeof(threadArgs));
        if (args == NULL) {
            fprintf(stderr, "Error allocating memory for thread args\n");
            exit(1);
        }

        args->queue = queue;
        args->id = i;

        consumers[i]->id = i;
        consumers[i]->jobsAsked = 0;
        consumers[i]->jobsReceived = 0;
        consumers[i]->jobsCompleted = 0;
        consumers[i]->currentlyProcessing = false;
        pthread_create(&consumers[i]->thread, NULL, consumeGoodies, &args);
    }

    //create producer threads, only one for this assignment
    producers = calloc(numberProducers, sizeof(producerThread));
    if (producers == NULL) {
        fprintf(stderr, "Error allocating memory for producer thread array\n");
        exit(1);
    }

    for (int i = 0; i < numberProducers; i++) {
        producers[i] = calloc(1, sizeof(producerThread));
        if (producers[i] == NULL) {
            fprintf(stderr, "Error allocating memory for producer thread\n");
            exit(1);
        }

        producers[i]->id = i;
        producers[i]->jobsProduced = 0;
        producers[i]->jobsSlept = 0;

        //dynamically allocate memory for args, threads only accept pointers as args, need to free later
        threadArgs * args = calloc(1, sizeof(threadArgs));
        if (args == NULL) {
            fprintf(stderr, "Error allocating memory for thread args\n");
            exit(1);
        }

        args->queue = queue;
        args->id = i;

        ret = pthread_create(&producers[i]->thread, NULL, produceGoodies, &args);
        if(ret != 0) {
            perror("pthread_create, producer");
            exit(1);
        }
    }

    //join producer threads, wait for complete processing
    for (int i = 0; i < numberProducers; i++) {
        ret = pthread_join(producers[i]->thread, NULL);
        if (ret != 0) {
            perror("pthread_join, producer");
            exit(1);
        }
    }


    //wait until queue is empty and all consumers are not processing


    /* 

    Currently doing a busy wait, wastes CPU Time, optimize code later to block this main thread until all consumers are not processing and the queue is empty

    */

    while (queue->size != 0) {
        //do nothing
    }

    for (int i = 0; i < numberConsumers; i++) {
        while (consumers[i]->currentlyProcessing) {
            //do nothing
        }
    }


    //print summary and terminate consumer threads
    printf("Summary:\n");

    char * summaryAction;
    int totalProducerWork, totalProducerSleep,totalConsumerAsk, totalConsumerReceive, totalConsumerComplete = 0;

    for (int i = 0; i < numberProducers; i++) {
        totalProducerWork += producers[i]->jobsProduced;
        totalProducerSleep += producers[i]->jobsSlept;
    }

    for (int i = 0; i < numberConsumers; i++) {
        totalConsumerAsk += consumers[i]->jobsAsked;
        totalConsumerReceive += consumers[i]->jobsReceived;
        totalConsumerComplete += consumers[i]->jobsCompleted;
    }

    summaryAction = "Work";
    printf("    %-10s %d\n", summaryAction, totalProducerWork);

    summaryAction = "Sleep";
    printf("    %-10s %d\n", summaryAction, totalProducerSleep);

    summaryAction = "Ask";
    printf("    %-10s %d\n", summaryAction, totalConsumerAsk);

    summaryAction = "Receive";
    printf("    %-10s %d\n", summaryAction, totalConsumerReceive);

    summaryAction = "Complete";
    printf("    %-10s %d\n", summaryAction, totalConsumerComplete);

    for (int i = 0; i < numberConsumers; i++) {
        printf("Thread %d     %d\n", i, consumers[i]->jobsCompleted);
    }


    printf("Transactions per second: %f", totalConsumerComplete / (getCurrentTimeInSeconds() - startTime));

    //free all memory, need to still free threadArgs for each consumer thread
    for (int i = 0; i < numberConsumers; i++) {
        free(consumers[i]);
    }

    for (int i = 0; i < numberProducers; i++) {
        free(producers[i]);
    }

    free(consumers);
    free(producers);

    destroyJobQueue(queue);
}


//pass in dynamically allocated memory for args
void * consumeGoodies(void * args) {
    threadArgs * argStruct = (threadArgs *) args;
    jobQueue * queue = argStruct->queue;
    int id = argStruct->id;
    
    double elapsed = 0;
    int ret = 0;
    char * action;

    while (1) {
        //record ask time
        elapsed = getCurrentTimeInSeconds();
        elapsed -= startTime;

        //obtain File IOlock
        ret = pthread_mutex_lock(&IOlock);
        if (ret != 0) {
            perror("pthread_mutex_lock, consumeGoodies");
            exit(1);
        }

        //layout: printf("%.3f ID= %d Q= %3d\n %-10s", elapsed, id, Q);

        action = "Ask";
        //print ask time
        printf("%.3f ID= %3d       %-10s\n", elapsed, id, action);

        //release File IOlock
        ret = pthread_mutex_unlock(&IOlock);
        if (ret != 0) {
            perror("pthread_mutex_unlock, consumeGoodies");
            exit(1);
        }

        //increment jobsAsked
        consumers[id]->jobsAsked++;

        //remove job blocks until a job is received
        int jobTime = removeJob(queue, id);

        //record receive time
        elapsed = getCurrentTimeInSeconds();
        elapsed -= startTime;

        //jobs has been received, adjust Q values, jobsReceived and other stats
        consumers[id]->jobsReceived++;

        //Q update
        ret = pthread_mutex_lock(&Qlock);
        if (ret != 0) {
            perror("pthread_mutex_lock, consumeGoodies");
            exit(1);
        }

        Q++;

        pthread_mutex_unlock(&Qlock);
        
        //obtain File IOlock
        ret = pthread_mutex_lock(&IOlock);
        if (ret != 0) {
            perror("pthread_mutex_lock, consumeGoodies");
            exit(1);
        }

        action = "Received";
        printf("%.3f ID= %3d Q= %3d %-10s %d\n", elapsed, id, Q, action, jobTime);

        //release File IOlock
        ret = pthread_mutex_unlock(&IOlock);
        if (ret != 0) {
            perror("pthread_mutex_unlock, consumeGoodies");
            exit(1);
        }

        //do work
        Trans(jobTime);

        //record complete time
        elapsed = getCurrentTimeInSeconds();
        elapsed -= startTime;


        //Q update
        ret = pthread_mutex_lock(&Qlock);
        if (ret != 0) {
            perror("pthread_mutex_lock, consumeGoodies");
            exit(1);
        }

        Q--;

        pthread_mutex_unlock(&Qlock);

        //obtain File IOlock
        ret = pthread_mutex_lock(&IOlock);
        if (ret != 0) {
            perror("pthread_mutex_lock, consumeGoodies");
            exit(1);
        }

        action = "Complete";

        printf("%.3f ID= %3d Q= %3d %-10s %d\n", elapsed, id, Q, action, jobTime);

        //release File IOlock
        ret = pthread_mutex_unlock(&IOlock);
        if (ret != 0) {
            perror("pthread_mutex_unlock, consumeGoodies");
            exit(1);
        }

        //increment jobsCompleted
        consumers[id]->jobsCompleted++;

        //thread is not currently processing, already did work
        consumers[id]->currentlyProcessing = false;
    }
    
    free(args);
}


//pass in dynamically allocated memory for args
void * produceGoodies(void * args) {
    threadArgs * argStruct = (threadArgs *) args;
    jobQueue * queue = argStruct->queue;
    int id = argStruct->id;
    
    double elapsed = 0;
    int ret = 0;
    char * action;

    char jobType;
    int jobTime;

    while (scanf(" %c%d", &jobType, &jobTime) == 2) {

        //add to queue
        if (jobType == 'T') {
            //add job to queue
            addJob(queue, jobTime);

            //record when job added time
            double elapsed = getCurrentTimeInSeconds();
            elapsed -= startTime;

            //obtain File IOlock
            int ret = pthread_mutex_lock(&IOlock);
            if (ret != 0) {
                perror("pthread_mutex_lock, produceGoodies");
                exit(1);
            }

            char * action = "Work";

            //print Work time
            printf("%.3f ID= %3d Q= %3d %-10s %d\n", elapsed, id, Q, action, jobTime);

            //release File IOlock
            ret = pthread_mutex_unlock(&IOlock);
            if (ret != 0) {
                perror("pthread_mutex_unlock, produceGoodies");
                exit(1);
            }

            //increment jobsProduced
            producers[id]->jobsProduced++;

        } else if(jobType == 'S') {

            //record when job added time
            double elapsed = getCurrentTimeInSeconds();
            elapsed -= startTime;

            //obtain File IOlock
            int ret = pthread_mutex_lock(&IOlock);
            if (ret != 0) {
                perror("pthread_mutex_lock, produceGoodies");
                exit(1);
            }

            char * action = "Sleep";

            //print Sleep time
            printf("%.3f ID= %3d Q= %3d %-10s %d\n", elapsed, id, Q, action, jobTime);

            //release File IOlock
            ret = pthread_mutex_unlock(&IOlock);
            if (ret != 0) {
                perror("pthread_mutex_unlock, produceGoodies");
                exit(1);
            }

            //producer should sleep
            Sleep(jobTime);

            //increment jobsSlept
            producers[id]->jobsSlept++;
            
        } else {
            fprintf(stderr, "Error in input file\n");
            exit(1);
        }
    }

    //producer has finished it's job

    //record end time
    elapsed = getCurrentTimeInSeconds();
    elapsed -= startTime;

    //obtain File IOlock
    ret = pthread_mutex_lock(&IOlock);
    if (ret != 0) {
        perror("pthread_mutex_lock, produceGoodies");
        exit(1);
    }

    action = "End";

    //print End time
    printf("%.3f ID= %3d Q= %3d %-10s\n", elapsed, id, Q, action);

    //release File IOlock
    ret = pthread_mutex_unlock(&IOlock);
    if (ret != 0) {
        perror("pthread_mutex_unlock, produceGoodies");
        exit(1);
    }

    free(args);
    
    pthread_exit(NULL);
}





