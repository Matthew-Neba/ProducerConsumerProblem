#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct consumerThread {
    pthread_t thread;
    int id;
    int jobsAsked;
    int jobsReceived;
    int jobsCompleted;
} consumerThread;

typedef struct producerThread {
    pthread_t thread;
    int id;
    int jobsProduced;
    int jobsSlept;
} producerThread;


//CTRL_C terminates all threads
//exit(0) terminates all threads
