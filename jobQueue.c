#include "mainHeader.h"



//dynamic memory allocation to allow for process to continue if main thread exits
jobQueue * createJobQueue() {
    int ret;
    jobQueue *queue = calloc(1,sizeof(jobQueue));

    if (queue == NULL) {
        return NULL;
    }

    queue->head = NULL;
    queue->tail = NULL;

    //mutex and condition variable initalization initialization with this function is for non static/global mutexes (i.e mutexes initialized at runtime), they take in pointers to the variable

    ret = pthread_mutex_init(&queue->queueMutex, NULL);
    if (ret != 0) {
        fprintf(stderr, "Error initializing mutex\n");
        return NULL;
    }

    ret = pthread_cond_init(&queue->notFull, NULL);
    if (ret != 0) {
        fprintf(stderr, "Error initializing condition variable\n");
        return NULL;
    } 

    ret = pthread_cond_init(&queue->notEmpty, NULL);
    if (ret != 0) {
        fprintf(stderr, "Error initializing condition variable\n");
        return NULL;
    } 

    return queue;
}

int addJob(jobQueue *queue, int jobSize) {
    int ret;
    //lock queueMutex
    ret = pthread_mutex_lock(&queue->queueMutex);

    //check if queue is full
    while(queue->isFull) {

        //wait for queue to not be full, release mutex while waiting
        ret = pthread_cond_wait(&queue->notFull, &queue->queueMutex);
        if (ret != 0) {
            fprintf(stderr, "Error waiting for queue to not be full\n");
            return -1;
        }
    }

    if (ret != 0) {
        return -1;
    }

    //create job
    jobNode * job = calloc(1, sizeof(jobNode));

    //set job time
    job->jobTime = jobSize;

    //setup Queue
    if (queue->tail == NULL) {
        //tail is Null when Head is Null
        queue->head = job;
    } else {
        queue->tail->next = job;
    }
    queue->tail = job;

    //increment size, check if full
    queue->size++;
    if (queue->size == 2 * numberConsumers) {
        queue->isFull = true;
    }

    //set boolean isEmpty
    queue->isEmpty = false;

    //signal that queue is not empty, wake up all consumers to proceed, sinc while loop for consumers, if still full, the other consumers will go back to sleep and wait, broadcast is useful here
    pthread_cond_signal(&queue->notEmpty);
    

    //unlock mutex
    ret = pthread_mutex_unlock(&queue->queueMutex);
    if (ret != 0) {

        fprintf(stderr, "Error unlocking mutex");
        return -1;
    }
 
    return 0;
}

int removeJob(jobQueue *queue, int threadID) {
    int ret;
    //lock queueMutex
    ret = pthread_mutex_lock(&queue->queueMutex);
    if (ret != 0) {
        fprintf(stderr, "Error locking mutex");
        return -1;
    }

    //check if queue is empty
    while(queue->isEmpty) {
        //wait for queue to not be empty, release mutex while waiting
        ret = pthread_cond_wait(&queue->notEmpty, &queue->queueMutex);
        if (ret != 0) {
            return -1;
        }
    }

    //only removing a job if queue is not empty
    jobNode * temp = queue->head;
    queue->head = queue->head->next;

    if (queue->head == NULL) {
        queue->tail = NULL;
    }

    int jobSize = temp->jobTime;

    free(temp);

    //decrement size, check if empty
    queue->size--;
    if (queue->size == 0) {
        queue->isEmpty = true;
    }

    //set boolean isFull
    queue->isFull = false;

    //signal that queue is not full
    pthread_cond_signal(&queue->notFull);

    //this is to signal to main thread that the current consumer thread is currently processing
    consumers[threadID]->currentlyProcessing = true;

    //unlock mutex
    ret = pthread_mutex_unlock(&queue->queueMutex);
    if (ret != 0) {
        fprintf(stderr, "Error unlocking mutex\n");
        return -1;
    }

    return jobSize;
}



int destroyJobQueue(jobQueue *queue) {
    int ret;

    //need to destroy mutex and condition variable if not static/global
    ret = pthread_mutex_destroy(&queue->queueMutex);
    if (ret != 0) {
        fprintf(stderr, "Error destroying mutex\n");
        return -1;
   } 

    ret = pthread_cond_destroy(&queue->notEmpty);
    if (ret != 0) {
        fprintf(stderr, "Error destroying condition variable\n");
        return -1;
   } 

    ret = pthread_cond_destroy(&queue->notFull);
    if (ret != 0) {
        fprintf(stderr, "Error destroying condition variable\n");
        return -1;
   }

    jobNode * current = queue->head;
    jobNode * temp;

    //free all jobs in queue
    while (current != NULL) {
        temp = current;
        current = current->next;
        free(temp);
    }
    
    //free queue
    free(queue);
    
    return 0;
}
