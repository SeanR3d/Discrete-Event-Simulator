#ifndef SERVERQUEUE_H
#define SERVERQUEUE_H
#include <stdio.h>
#include "job.h"
#include "server.h"
#include "eventQueue.h"

/*
	Header file to implement SERVER queues.
	Server queues use a link listed design for each JOB
	by: Sean Reddington
*/

//Node struct for job elements to store in the queue
typedef struct NODE {
	JOB *job;
	struct NODE *next;
}NODE;

//Queue struct
typedef struct SERVERQUEUE {
	unsigned size;
	NODE *head;
	NODE *tail;
}SERVERQUEUE;

NODE* newNode(JOB *j);
SERVERQUEUE *createQueue();
void enQueue(SERVERQUEUE *q, JOB *j);
NODE *deQueue(SERVERQUEUE *q);

//Creates new job NODE
NODE* newNode(JOB *j) {
	NODE *temp = (NODE*)malloc(sizeof(NODE));
	temp->job = j;
	temp->next = NULL;
	return temp;
}

//Creates a server queue for a given server
SERVERQUEUE *createQueue() {
	SERVERQUEUE *q = (SERVERQUEUE*)malloc(sizeof(SERVERQUEUE));
	q->size = 0;
	q->head = NULL;
	q->tail = NULL;
	return q;
}

//Adds a job node to queue
void enQueue(SERVERQUEUE *q, JOB *j) {
	NODE *temp = newNode(j);

	//if queue is empty
	if (q->tail == NULL) {
		q->head = temp;
		q->tail = temp;
		q->size++;
		return;
	}

	//Adds the new node at the end of queue
	q->tail->next = temp;
	q->tail = temp;
	q->size++;
}

//Removes a node from given queue q
NODE *deQueue(SERVERQUEUE *q) {

	//if queue is empty
	if (q->head == NULL) {
		return NULL;
	}
		
	//stores previous head and move head one node ahead
	NODE *temp = q->head;
	q->head = q->head->next;

	//if head becomes NULL, then also change rear to NULL
	if (q->head == NULL) {
		q->tail = NULL;
		q->size = 0;
	}
	else {
		q->size--;
	}
		
	
	return temp;
}
#endif