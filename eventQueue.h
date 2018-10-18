#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "job.h"

//heap element seeking equations
#define LCHILD(x) 2 * x + 1
#define RCHILD(x) 2 * x + 2
#define PARENT(x) (x - 1) / 2


/*
Header file for the event priority queue.
The event queue is a minHeap data structure.
by: Sean Reddington
*/

//enumerator cases for the types of events
typedef enum EVENTCASE {
	arrive, entered_CPU, finished_CPU, finished_JOB,
	entered_DISK1, finished_DISK1, entered_DISK2, finished_DISK2, finished_SIM
}EVENTCASE;

typedef struct EVENT {

	JOB *job;
	char event_str[50];
	unsigned job_num;
	unsigned action_time;
	enum EVENTCASE ec;
	
} EVENT;

typedef struct EVENTQUEUE {

	unsigned size;
	EVENT *event;

} EVENTQUEUE;

char* getEventStr(EVENT *e);
EVENT* newEvent(JOB *j, enum EVENTCASE c);
EVENTQUEUE* init_event_queue();
void swap(EVENT *E1, EVENT *E2);
void heapify(EVENTQUEUE *eq, int i);
void insertEvent(EVENTQUEUE *eq, JOB *j, enum EVENTCASE c);
EVENT* removeEvent(EVENTQUEUE *eq);
void deleteEventQueue(EVENTQUEUE *eq);
void levelOrderTraversal(EVENTQUEUE *eq);

//generates a string describing the event
char* getEventStr(EVENT *e) {
	char str[50];
	char *strPtr = malloc(sizeof(char) * 50);
	strcpy(str, "Time ");
	strPtr = &str;
	char buffer[1000];
	sprintf(buffer, "%d", e->action_time);
	strcat(str, buffer);

	strcat(str, ": Job ");
	sprintf(buffer, "%d", e->job_num);
	strcat(str, buffer);

	switch (e->ec) {
	case arrive:
		strcat(str, " arrived");
		break;
	case entered_CPU:
		strcat(str, " entered CPU");
		break;
	case finished_CPU:
		strcat(str, " finished at CPU");
		break;
	case finished_JOB:
		strcat(str, " completed");
		break;
	case entered_DISK1:
		strcat(str, " entered DISK 1");
		break;
	case finished_DISK1:
		strcat(str, " finished at DISK 1");
		break;
	case entered_DISK2:
		strcat(str, " entered DISK 2");
		break;
	case finished_DISK2:
		strcat(str, " finished at DISK 2");
		break;
	case finished_SIM:
		strcpy(str, "Time ");
		char buffer[1000];
		sprintf(buffer, "%d", e->action_time);
		strcat(str, buffer);
		strcat(str, " finished simulation");
		break;
	}

	strcpy(strPtr, str);

	return strPtr;
}

//initializes new event
EVENT* newEvent(JOB *j, enum EVENTCASE c) {
	EVENT *e = (EVENT*)malloc(sizeof(EVENT));
	e->job = j;
	e->job_num = j->job_num;
	e->action_time = j->event_time;
	e->ec = c;
	strcpy(e->event_str, getEventStr(e));
	return e;
}

//initialize the priority queue
EVENTQUEUE* init_event_queue() {
	EVENTQUEUE *eq = (EVENTQUEUE*)malloc(sizeof(EVENTQUEUE));
	eq->size = 0;

	return eq;
}

//swaps two events in the priority queue
void swap(EVENT *E1, EVENT *E2) {
	EVENT temp = *E1;
	*E1 = *E2;
	*E2 = temp;
}

//checks that the priority queue maintains its min heap properties
void heapify(EVENTQUEUE *eq, int i) {
	unsigned smallest = (LCHILD(i) < eq->size && eq->event[LCHILD(i)].action_time < eq->event[i].action_time) ? LCHILD(i) : i;

	if (RCHILD(i) < eq->size &&
		eq->event[RCHILD(i)].action_time < eq->event[smallest].action_time) {
		smallest = RCHILD(i);
	}
	if (smallest != i) {
		swap(&(eq->event[i]), &(eq->event[smallest]));
		heapify(eq, smallest);

	}

}

//inserts an event into the priority queue
void insertEvent(EVENTQUEUE *eq, JOB *j, enum EVENTCASE c) {
	if (eq->size) {
		eq->event = realloc(eq->event, (eq->size + 1) * sizeof(EVENT));
	}
	else {
		eq->event = malloc(sizeof(EVENT));
	}

	EVENT *e = (EVENT*)malloc(sizeof(EVENT));
	e = newEvent(j, c);

	int i = (eq->size)++;
	while (i && e->action_time < eq->event[PARENT(i)].action_time) {
		eq->event[i] = eq->event[PARENT(i)];
		i = PARENT(i);
	}
	eq->event[i] = *e;

}

//removes an event from the priority queue
EVENT* removeEvent(EVENTQUEUE *eq) {
	EVENT *removed = (EVENT*)malloc(sizeof(EVENT));
	if (eq->size) {
		*removed = eq->event[0];
		eq->event[0] = eq->event[--(eq->size)];
		eq->event = realloc(eq->event, eq->size * sizeof(EVENT));
		heapify(eq, 0);
	}
	else {
		free(eq->event);
		return NULL;
	}

	return removed;
}

//clears memory allocated for the priority queue
void deleteEventQueue(EVENTQUEUE *eq) {
	free(eq->event);
}

//prints the event in the priority queue by level order traversal for testing purposes
void levelOrderTraversal(EVENTQUEUE *eq) {
	for (int i = 0; i < eq->size; i++) {
		printf("Job%u: %u\n", eq->event[i].job_num, eq->event[i].action_time);
	}
}

#endif
