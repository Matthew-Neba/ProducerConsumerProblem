#ifndef mainHeader
#define mainHeader


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <fcntl.h>


typedef struct consumerThread {
    pthread_t thread;
    int id;
    int jobsAsked;
    int jobsReceived;
    int jobsCompleted;
    bool currentlyProcessing;
} consumerThread;

typedef struct producerThread {
    pthread_t thread;
    int id;
    int jobsProduced;
    int jobsSlept;
} producerThread;

typedef struct jobNode {
    int jobTime;
    struct jobNode * next;
} jobNode;

typedef struct jobQueue {
   jobNode * head;
   jobNode * tail;
   int size;
   pthread_mutex_t queueMutex;
   pthread_cond_t notFull;
   pthread_cond_t notEmpty;
   bool isFull;
   bool isEmpty;
} jobQueue;

typedef struct threadArgs {
    jobQueue * queue;
    int id;
} threadArgs;

//Global Variables
int numberConsumers;
int numberProducers;
pthread_mutex_t IOlock;


//thread arrays
consumerThread ** consumers;
producerThread ** producers;

//functions
void * consumeGoodies(void * args);
void * produceGoodies(void * args);

void Trans( int n );
void Sleep( int n );

jobQueue * createJobQueue();
int addJob(jobQueue * queue, int jobSize);
int removeJob(jobQueue * queue, int threadID);
int destroyJobQueue(jobQueue * queue);


//CTRL_C terminates all threads
//exit(0) terminates all threads

#endif
