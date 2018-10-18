#ifndef MAIN_C
#define MAIN_C
#pragma warning(disable : 4996)
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "configFile.h"
#include "job.h"
#include "eventQueue.h"
#include "server.h"
#include "serverQueue.h"
#define num_servers 3

/*
CIS3207: Lab 1 - Giorgio's Discrete Event Simulator
Sean Reddington
September 7, 2018

*/

unsigned job_arrival(JOB *j, SERVER *cpu, EVENTQUEUE *eq, const unsigned a_MIN, const unsigned a_MAX);
int rng(unsigned MIN, unsigned MAX);
FILE* create_log_file(CONFIG cnf);

int main() {

	//reads config values from cnf file
	CONFIG config_file = readConfigFile();

	//set seed
	srand(config_file.SEED);
	//srand(time(NULL)); //time seed

	//creates log output file
	FILE *log = create_log_file(config_file);

	//sets current time
	unsigned CURRENT_TIME = config_file.INIT_TIME;

	//creates event priority queue
	EVENTQUEUE *eqPtr = init_event_queue();

	//creates servers
	SERVER *CPU = init_server(entered_CPU, finished_CPU, config_file.CPU_MIN, config_file.CPU_MAX, config_file.QUIT_PROB);
	SERVER *DISK1 = init_server(entered_DISK1, finished_DISK1, config_file.DISK1_MIN, config_file.DISK1_MAX, 0);
	SERVER *DISK2 = init_server(entered_DISK2, finished_DISK2, config_file.DISK2_MIN, config_file.DISK2_MAX, 0);

	//add first job event queue
	JOB *job1 = newJob(1, CURRENT_TIME);
	insertEvent(eqPtr, job1, arrive);

	//add Termination event to event queue
	JOB *SENTINEL = newJob(0, config_file.FIN_TIME);
	SENTINEL->server_arrive_time =config_file.INIT_TIME;
	insertEvent(eqPtr, SENTINEL, finished_SIM);

	//levelOrderTraversal(eqPtr);

	while (eqPtr->size != 0 && (CURRENT_TIME < config_file.FIN_TIME)) {

		////checks if each server is finished servicing its current job
		//for (size_t i = 0; i < num_servers; i++) {
		//	server_check(server_arr[i], eqPtr, CURRENT_TIME);
		//}

		//testing purposes
		//levelOrderTraversal(eqPtr);

		//removes next event to be handled
		EVENT *current_event = removeEvent(eqPtr);
		JOB *events_job = current_event->job;

		//prints event to log
		fprintf(log, "%s\n", current_event->event_str);

		//testing purposes
		//printf("%s\n", current_event->event_str);

		//updates current time
		CURRENT_TIME = current_event->action_time;

		//switch to handle the type of event
		switch (current_event->ec) {
		case arrive:
			CURRENT_TIME = job_arrival(events_job, CPU, eqPtr, config_file.ARRIVE_MIN, config_file.ARRIVE_MAX);
			break;

		case finished_CPU:
		case finished_JOB:
			update_queue_stats(CPU);
			events_job = CPU_finished(CPU, events_job, eqPtr, CURRENT_TIME);

			//if job did not complete, then it is set to a disk
			if (events_job != NULL) {
				determine_DISK(DISK1, DISK2, events_job, eqPtr, CURRENT_TIME);
			}
			break;

		case finished_DISK1:
			update_queue_stats(DISK1);
			DISK_finished(DISK1, CPU, events_job, eqPtr, CURRENT_TIME);
			break;

		case finished_DISK2:
			update_queue_stats(DISK2);
			DISK_finished(DISK2, CPU, events_job, eqPtr, CURRENT_TIME);
			break;

		case finished_SIM:
			//FIN_TIME has been reach
			CURRENT_TIME = config_file.FIN_TIME;
			break;
		}//end switch

	}//end while loop

	//total time the simulation runs
	unsigned total_time = config_file.FIN_TIME - config_file.INIT_TIME;

	//prints summaries to console
	printf("CPU stats:\n");
	finalize_server(CPU, total_time);
	printf("\nDISK1 stats:\n");
	finalize_server(DISK1, total_time);
	printf("\nDISK2 stats:\n");
	finalize_server(DISK2, total_time);
	fclose(log);

	//system("pause");
	return 0;
}

//handler function for when a new job event occurs
unsigned job_arrival(JOB *j, SERVER *cpu, EVENTQUEUE *eq, const unsigned a_MIN, const unsigned a_MAX) {
	unsigned cur_time = j->event_time;

	//determine arrival time for next job to enter the system and adds it to the eventQueue
	unsigned next_arrival = cur_time + rng(a_MIN, a_MAX);
	unsigned next_job_num = j->job_num;
	JOB *next_job = newJob(++next_job_num, next_arrival);
	insertEvent(eq, next_job, arrive);

	//sends initial job to CPU
	send_to_server(cpu, j, eq, cur_time);

	return cur_time;
}

//Random number generator
int rng(unsigned MIN, unsigned MAX) {
	unsigned r = MIN + (rand() % ((MAX - MIN) + 1));
	return r;
}

//Creates a log.txt file containing the config file values used
//and the events handled from the event queue
FILE * create_log_file(CONFIG cnf) {
	FILE *outPtr = (FILE*)malloc(sizeof(FILE));
	if ((outPtr = fopen("log.txt", "w")) == NULL) {
		puts("File could not be opened");
	}
	else {
		fprintf(outPtr, "SEED: %u\n", cnf.SEED);
		fprintf(outPtr, "INIT_TIME: %u\nFIN_TIME: %u\n", cnf.INIT_TIME, cnf.FIN_TIME);
		fprintf(outPtr, "ARRIVE_MIN: %u\nARRIVE_MAX: %u\n", cnf.ARRIVE_MIN, cnf.ARRIVE_MAX);
		fprintf(outPtr, "QUIT_PROB: %.2f\n", cnf.QUIT_PROB);
		fprintf(outPtr, "CPU_MIN: %u\nCPU_MAX: %u\n", cnf.CPU_MIN, cnf.CPU_MAX);
		fprintf(outPtr, "DISK1_MIN: %u\nDISK1_MAX: %u\n", cnf.DISK1_MIN, cnf.DISK1_MAX);
		fprintf(outPtr, "DISK2_MINL %u\nDISK2_MAX: %u\n", cnf.DISK2_MIN, cnf.DISK2_MAX);
		fprintf(outPtr, "\n***EVENT QUEUE***\n");
	}
	
	return outPtr;
}


#endif
