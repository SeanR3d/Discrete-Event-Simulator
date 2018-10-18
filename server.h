#ifndef SERVER_H
#define SERVER_H
#include <stdio.h>
#include "job.h"
#include "eventQueue.h"
#include "serverQueue.h"

/*
Header file to implement SERVER components and their functionality, including servicing jobs,
	maintaining queues for jobs waiting to enter the server, and passing jobs to other servers.

by: Sean Reddington
*/

//SERVER struct for servers in the simulator 
typedef struct SERVER {
	JOB *current_job;
	unsigned service_end_time;
	unsigned service_duration;
	float quit_prob;
	SERVERQUEUE *serverQueue;
	unsigned SERVER_MIN;
	unsigned SERVER_MAX;
	EVENTCASE enter_case;
	EVENTCASE finish_case;
	unsigned num_jobs_serviced;
	unsigned average_queue_size;
	unsigned max_queue_size;
	unsigned total_busy_time;
	double server_utilization;
	unsigned average_response_time;
	unsigned max_response_time;
	double throughtput;

}SERVER;

SERVER* init_server(EVENTCASE enter, EVENTCASE finish, const unsigned s_MIN, const unsigned s_MAX, const float qp);
void server_check(SERVER *server, EVENTQUEUE *eq, unsigned cur_time);
void service_next(SERVER *server, EVENTQUEUE *eq, unsigned cur_time);
void send_to_server(SERVER *server, JOB *j, EVENTQUEUE *eq, unsigned cur_time);
JOB* CPU_finished(SERVER *cpu, JOB *j, EVENTQUEUE *eq, unsigned cur_time);
void DISK_finished(SERVER *disk, SERVER *cpu, JOB *j, EVENTQUEUE *eq, unsigned cur_time);
void determine_DISK(SERVER *disk1, SERVER *disk2, JOB *j, EVENTQUEUE *eq, unsigned cur_time);
void update_queue_stats(SERVER* server);
void finalize_server(SERVER *server, unsigned total_time);
void update_response_times(SERVER* server, unsigned cur_time);

//initializes server and its elements
SERVER* init_server(EVENTCASE enter, EVENTCASE finish, const unsigned s_MIN, const unsigned s_MAX, const float qp) {
	SERVER *s = (SERVER*)malloc(sizeof(SERVER));
	s->current_job = NULL;
	s->service_duration = 0;
	s->quit_prob = qp;
	s->serverQueue = createQueue();
	s->SERVER_MIN = s_MIN;
	s->SERVER_MAX = s_MAX;
	s->enter_case = enter;
	s->finish_case = finish;
	s->num_jobs_serviced = 0;
	s->average_queue_size = 0;
	s->max_queue_size = 0;
	s->total_busy_time = 0;
	s->server_utilization = 0;
	s->average_response_time = 0;
	s->max_response_time = 0;
	s->throughtput = 0;
	return s;
}

//Check if a server is has finished its current job and is ready to service the next job in its queue
void server_check(SERVER *server, EVENTQUEUE *eq, unsigned cur_time) {
	if (server->current_job != NULL) { //checks if server currently has a job
		//interrupts next job to give priority to sooner event
		if (eq->event[0].action_time > server->service_end_time) {
			server->current_job->event_time = server->service_end_time;
			insertEvent(eq, server->current_job, server->finish_case);
		}
	}
}

//Removes the job at the head of the passed server's queue and begins servicing it
void service_next(SERVER *server, EVENTQUEUE *eq, unsigned cur_time) {
	if (server->current_job == NULL) {
		server->service_duration = rng(server->SERVER_MIN, server->SERVER_MAX); //rng determines service duration

		//updates total time that the server has serviced jobs
		server->total_busy_time += server->service_duration;

		//dequeues front job into server
		server->current_job = deQueue(server->serverQueue)->job;
		server->current_job->event_time = cur_time;

		//tracks at what time the server will finish servicing the current job
		server->service_end_time = cur_time + server->service_duration;

		//inserts event for when the job will finish at the server
		server->current_job->event_time = server->service_end_time;
		//RNG determines if a job is complete and can leave the simulation based on QUIT_PROB
		if (server->quit_prob > 0 && rng(0, 100) <= (server->quit_prob * 100)) {
			server->current_job->is_complete = 1;
			insertEvent(eq, server->current_job, finished_JOB);
		}
		else {
			insertEvent(eq, server->current_job, server->finish_case);
		}
	}


}

//Sends a job to the degsinated server to be serviced, or added to its queue if busy
void send_to_server(SERVER *server, JOB *j, EVENTQUEUE *eq, unsigned cur_time) {

	//keeps track of when a job arrives at a server for response time stats
	j->server_arrive_time = cur_time;

	//if server is free and queue is empty, starts servicing job
	if (server->current_job == NULL && server->serverQueue->head == NULL) {
		//rng determines service duration
		server->service_duration = rng(server->SERVER_MIN, server->SERVER_MAX);
		server->current_job = j; //adds job to server
		server->current_job->event_time = cur_time;

		//updates total time that the server has serviced jobs
		server->total_busy_time += server->service_duration;
		

		//tracks at what time the server will finish servicing the current job
		server->service_end_time = cur_time + server->service_duration;


		//inserts event for when the job will finish at the server
		server->current_job->event_time = server->service_end_time;

		//RNG determines if a job is complete and can leave the simulation based on QUIT_PROB
		if (server->enter_case == 1 && rng(0, 100) <= (server->quit_prob * 100)) {
			server->current_job->is_complete = 1;
			insertEvent(eq, server->current_job, finished_JOB);
		}
		else {
			//job did not complete and is sent to next server
			insertEvent(eq, server->current_job, server->finish_case);
		}
		
	}
	else {
		enQueue(server->serverQueue, j); //if server is busy, sends job to end of queue
	}
}

/*Handler function for when the CPU reaches its servicing time for a job.
	function will check if a job is completely finished in the simulation
	and if not, it will return the job to be sent to a disk.
*/
JOB* CPU_finished(SERVER *cpu, JOB *j, EVENTQUEUE *eq, unsigned cur_time) {
	//checks that servicing time has passed and there is a job in the CPU ready to be removed
	if (cur_time >= j->event_time && cpu->current_job != NULL) {
		JOB *finished_job = cpu->current_job;
		update_response_times(cpu, cur_time);
		cpu->current_job = NULL; //removes job from CPU
		cpu->num_jobs_serviced++;

		//completed job event
		if (finished_job->is_complete) {
			finished_job = NULL;
		}

		//starts servicing next job in queue
		if (cpu->serverQueue->head != NULL) {
			service_next(cpu, eq, ++cur_time);
		}

		//returns job to be sent to a disk or returns NULL if job completed
		if (finished_job == NULL) {
			free(finished_job);
			return NULL;
			
		} else{
			return finished_job;
		}
	}
}

//Handler function for when a disk finishes servicing its current job
void DISK_finished(SERVER *disk, SERVER *cpu,  JOB *j, EVENTQUEUE *eq, unsigned cur_time) {
	if (cur_time == j->event_time //checks if current time = servicing time
		&& disk->current_job != NULL) { //and that the disk isn't empty
			JOB *finished_job = (JOB*)malloc(sizeof(JOB));
			finished_job = disk->current_job;
			update_response_times(disk, cur_time);
			disk->current_job = NULL; //removes job from disk
			disk->num_jobs_serviced++;
			send_to_server(cpu, finished_job, eq, ++cur_time); //returns job back to CPU

			if (disk->serverQueue->head != NULL) { //starts servicing next job in queue
				service_next(disk, eq, ++cur_time);
			}
	}
}

//Sends jobs from the CPU to one of two disks which is determined by which disk is not busy or has the shortest queue
void determine_DISK(SERVER *disk1, SERVER *disk2, JOB *j, EVENTQUEUE *eq, unsigned cur_time) {
	if (disk1->current_job == NULL) {
		//if both disks are empty, randomly picks where the job is sent at a 50% chance
		if (disk2->current_job == NULL) {
			if (rng(0, 100) < 50) {
				send_to_server(disk1, j, eq, cur_time); //rng chose disk 1
			}
			else {
				send_to_server(disk2, j, eq, cur_time); //rng chose disk 2
			}
		}
		else {
			//if disk1 is empty but disk2 is busy, sends job to disk1
			send_to_server(disk1, j, eq,  cur_time);
		}
	}
	else{
		//if disk1 is busy but disk2 is empty, sends job to disk 2
		if (disk2->current_job == NULL) {
			send_to_server(disk2, j, eq,  cur_time);
		}
		else {
			//if both disks are busy and have the same queue size, randomly picks where the job is sent at a 50% chance
			if (disk1->serverQueue->size == disk2->serverQueue->size) {
				if (rng(0, 100) < 50) {
					send_to_server(disk1, j, eq, cur_time); //rng chose disk 1
				}
				else {
					send_to_server(disk2, j, eq, cur_time); //rng chose disk 2
				}
			}
			//if disk1 queue size < disk2 queue size, sends to disk1
			else if(disk1->serverQueue->size < disk2->serverQueue->size) {
				send_to_server(disk1, j, eq, cur_time);
			}
			else {
				//if disk 1 queue size > disk2 queue size, sends to disk2
				send_to_server(disk2, j, eq, cur_time);
			}

		}
	}
}

//updates server's average and max queue sizes, *HOWEVER*, average queue size is not complete until simulation complete
void update_queue_stats(SERVER* server) {
	int cur_size = server->serverQueue->size;
	if(cur_size > server->max_queue_size) {
		server->max_queue_size = cur_size;
	}
	server->average_queue_size += cur_size;
}

//updates server's average and max response times, *HOWEVER*, average response time is not complete until simulation completes
void update_response_times(SERVER* server, unsigned cur_time) {

	unsigned response_time = cur_time - server->current_job->server_arrive_time;

	if (response_time > server->max_response_time)
		server->max_response_time = response_time;

	server->average_response_time += cur_time - server->current_job->server_arrive_time;
}

//After the simulator is complete, prints out the following statistics and frees memory used by server
void finalize_server(SERVER *server, unsigned total_time) {

	update_queue_stats(server);

	//compute average queue size
	server->average_queue_size /= server->num_jobs_serviced;

	//ignores service time overflow
	if (server->service_end_time > total_time)
		server->total_busy_time -= (server->service_end_time % total_time);

	//compute server utilization
	server->server_utilization = (100*(double)server->total_busy_time / (double)total_time);

	//compute average response time
	server->average_response_time /= server->num_jobs_serviced;
	
	//compute throughput
	server->throughtput = server->num_jobs_serviced / ((double)total_time *.1);
	
	//print summary to console
	printf("final queue size: %d\n", server->serverQueue->size);
	printf("average queue size: %u\n", server->average_queue_size);
	printf("max queue size: %u\n", server->max_queue_size);
	printf("server utilization: %.1f%%\n", server->server_utilization);
	printf("average response time: %u\n", server->average_response_time);
	printf("max response time: %u\n", server->max_response_time);
	printf("throughput at 10%% of total time: %.2f\n", server->throughtput);

	//free memory allocated by server
	free(server->serverQueue);
	free(server);
}
#endif