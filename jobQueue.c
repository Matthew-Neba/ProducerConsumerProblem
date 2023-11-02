#include "mainHeader.h"



//dynamic memory allocation to allow for process to continue if main thread exits
jobQueue * createJobQueue() {
    int ret;
    jobQueue *queue = calloc(1,sizeof(jobQueue));

    if (queue == NULL) {
        perror("calloc, createJobQueue");
        return NULL;
    }

    queue->head = NULL;
    queue->tail = NULL;

    //mutex and condition variable initalization initialization with this function is for non static/global mutexes (i.e mutexes initialized at runtime), they take in pointers to the variable

    ret = pthread_mutex_init(&queue->queueMutex, NULL);
    if (ret != 0) {
        perror("pthread_mutex_init, createJobQueue");
        return NULL;
    }

    ret = pthread_cond_init(&queue->notFull, NULL);
    if (ret != 0) {
    perror("pthread_cond_init, createJobQueue");
    return NULL;
    } 

    ret = pthread_cond_init(&queue->notEmpty, NULL);
    if (ret != 0) {
    perror("pthread_cond_init, createJobQueue");
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
            perror("pthread_cond_wait, addJob");
            return -1;
        }
    }

    if (ret != 0) {
        perror("pthread_mutex_lock, addJob");
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

    //signal that queue is not empty, wake up all consumers to proceed, sinc while loop for consumers, if still full, the other consumers will go back to sleep and wait, broadcast is useful here
    pthread_cond_signal(&queue->notEmpty);

    //unlock mutex
    ret = pthread_mutex_unlock(&queue->queueMutex);
    if (ret != 0) {
        perror("pthread_mutex_unlock, addJob");
        return -1;
    }
 
    return 0;
}

int removeJob(jobQueue *queue, int threadID) {
    int ret;
    //lock queueMutex
    ret = pthread_mutex_lock(&queue->queueMutex);
    if (ret != 0) {
        perror("pthread_mutex_lock, removeJob");
        return -1;
    }

    //check if queue is empty
    while(queue->isEmpty) {
        //wait for queue to not be empty, release mutex while waiting
        ret = pthread_cond_wait(&queue->notEmpty, &queue->queueMutex);
        if (ret != 0) {
            perror("pthread_cond_wait, removeJob");
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

    //signal that queue is not full
    pthread_cond_signal(&queue->notFull);

    //signal thread is currently processing
    consumers[threadID]->currentlyProcessing = true;

    //unlock mutex
    ret = pthread_mutex_unlock(&queue->queueMutex);
    if (ret != 0) {
        perror("pthread_mutex_unlock, removeJob");
        return -1;
    }

    return jobSize;
}



int destroyJobQueue(jobQueue *queue) {
    int ret;

    //need to destroy mutex and condition variable if not static/global
    ret = pthread_mutex_destroy(&queue->queueMutex);
    if (ret != 0) {
    perror("pthread_mutex_destroy, destroyJobQueue");
    return -1;
   } 

    ret = pthread_cond_destroy(&queue->notEmpty);
    if (ret != 0) {
    perror("pthread_cond_destroy, destroyJobQueue");
    return -1;
   } 

    ret = pthread_cond_destroy(&queue->notFull);
    if (ret != 0) {
    perror("pthread_cond_destroy, destroyJobQueue");
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
