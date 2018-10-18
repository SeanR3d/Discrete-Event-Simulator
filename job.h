#ifndef JOB_H
#define JOB_H
#include <stdio.h>

//header file for JOBS that are processed within the simulator

typedef struct JOB {

	unsigned job_num;
	unsigned event_time;
	unsigned server_arrive_time;
	unsigned is_complete;


} JOB;

//initializes a new job
JOB* newJob(unsigned jnum, unsigned time) {
	JOB *j = (JOB*)malloc(sizeof(JOB));
	j->job_num = jnum;
	j->event_time = time;
	j->server_arrive_time = time;
	j->is_complete = 0;
	return j;
}

#endif