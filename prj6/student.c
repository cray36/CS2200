/*
 * student.c
 * Multithreaded OS Simulation for CS 2200, Project 6
 * Summer 2015
 *
 * This file contains the CPU scheduler for the simulation.
 * Name:
 * GTID:
 */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "os-sim.h"


/*
 * current[] is an array of pointers to the currently running processes.
 * There is one array element corresponding to each CPU in the simulation.
 *
 * current[] should be updated by schedule() each time a process is scheduled
 * on a CPU.  Since the current[] array is accessed by multiple threads, you
 * will need to use a mutex to protect it.  current_mutex has been provided
 * for your use.
 */
static pcb_t **current;
static pthread_mutex_t current_mutex;
static pthread_mutex_t ready_queue_mutex;
static pthread_cond_t ready_queue_not_empty;
static pcb_t *ready_queue_head;
static pcb_t *ready_queue_tail;
static int ready_queue_size;
static int scheduling_algorithm;
static int rr_timeslice;
static int count_cpu;


/*
 * schedule() is your CPU scheduler.  It should perform the following tasks:
 *
 *   1. Select and remove a runnable process from your ready queue which 
 *	you will have to implement with a linked list or something of the sort.
 *
 *   2. Set the process state to RUNNING
 *
 *   3. Call context_switch(), to tell the simulator which process to execute
 *      next on the CPU.  If no process is runnable, call context_switch()
 *      with a pointer to NULL to select the idle process.
 *	The current array (see above) is how you access the currently running
 *	process indexed by the cpu id. See above for full description.
 *	context_switch() is prototyped in os-sim.h. Look there for more information 
 *	about it and its parameters.
 */

static pcb_t* pop_ready_queue()
{
	pcb_t* retVal;
	printf("Dequeue\n");
	pthread_mutex_lock(&ready_queue_mutex);
	while (ready_queue_size == 0 || ready_queue_head == NULL || ready_queue_tail == NULL)
	{
		printf("Empty queue\n");
		pthread_cond_wait(&ready_queue_not_empty, &ready_queue_mutex);
		printf("Waking up from pop ready queue\n");
	}
	retVal = ready_queue_head;
	ready_queue_head = ready_queue_head->next;
	ready_queue_size--;
	if (ready_queue_size == 0) {
		ready_queue_head = NULL;
		ready_queue_tail = NULL;
	}
	pthread_mutex_unlock(&ready_queue_mutex);
	return retVal;
}

static void push_ready_queue(pcb_t* process)
{
	pcb_t *temp;
	process->next = NULL;
	printf("Enqueue\n");
	pthread_mutex_lock(&ready_queue_mutex);
	if (0 == ready_queue_size || ready_queue_head == NULL || ready_queue_tail == NULL)
	{
		ready_queue_head = process;
		ready_queue_tail = process;
	}
	else {
		/*FCFS*/
		if (scheduling_algorithm == 0) {
			temp = ready_queue_head;
			/*
			 * if process's pid is less than head's pid
			 * put it as head
			 */
			if (temp->pid > process->pid) {
				process->next = temp;
				ready_queue_head = process;
			}
			else {
				/* find the first process that has higher pid
				 * then insert before it
				 */
				while (temp->next!=NULL && temp->next->pid <= process->pid)
					temp = temp->next;

				process->next = temp->next;
				temp->next = process;
			}
		} /*static priority*/
		else if (scheduling_algorithm == 1) {
			temp = ready_queue_head;
			if (temp->static_priority <= process->static_priority) {
				process->next = temp;
				ready_queue_head = process;
			}
			else {
				while (temp->next != NULL && temp->next->static_priority > process->static_priority)
					temp = temp->next;
				/*if there are no lower priority place it at the end*/

				process->next = temp->next;
				temp->next = process;
			}
		} /*round robin*/
		else {
			ready_queue_tail->next = process;
			ready_queue_tail = process;
		}
	}
	ready_queue_size++;
	pthread_cond_broadcast(&ready_queue_not_empty);
	pthread_mutex_unlock(&ready_queue_mutex);
}
static void schedule(unsigned int cpu_id)
{
    /* FIX ME */
	pcb_t* runnableProcess;
	printf("Schedule\n");
	pthread_mutex_lock(&current_mutex);
	if (ready_queue_size > 0) {
	/*get the head of ready queue, FIFO*/
    	runnableProcess = pop_ready_queue();
    	/*set process state to running*/
    	runnableProcess->state = PROCESS_RUNNING;
    	/*call context switch*/
    	if (scheduling_algorithm == 0)
    	{
    		/*FIFO*/
    		context_switch(cpu_id, runnableProcess, -1);
    	}
    	else if (scheduling_algorithm == 1)
    	{
    		/*Static priority*/
    		context_switch(cpu_id, runnableProcess, -1);
    	}
    	else if (scheduling_algorithm == 2)
    	{
    		/*Round robin*/
    		context_switch(cpu_id, runnableProcess, rr_timeslice);
    	}
        current[cpu_id] = runnableProcess;
    }
    else
    {
    	/*if the ready queue is empty, raise the signal*/

    	/*call context switch*/
    	if (scheduling_algorithm == 0)
    	{
    		context_switch(cpu_id, NULL, -1);
    	}
    	else if (scheduling_algorithm == 1)
    	{
    		context_switch(cpu_id, NULL, -1);
    	}
    	else if (scheduling_algorithm == 2)
    	{
    		context_switch(cpu_id, NULL, rr_timeslice);
    	}
    }
    pthread_mutex_unlock(&current_mutex);
}


/*
 * idle() is your idle process.  It is called by the simulator when the idle
 * process is scheduled.
 *
 * This function should block until a process is added to your ready queue.
 * It should then call schedule() to select the process to run on the CPU.
 */
extern void idle(unsigned int cpu_id)
{
    /* FIX ME */
    printf("Idle\n");
    pthread_mutex_lock(&ready_queue_mutex);
    while (ready_queue_size == 0 || ready_queue_head == NULL) {
    	pthread_cond_wait(&ready_queue_not_empty, &ready_queue_mutex);
    	       printf("Waking up from idle\n");
    }
    pthread_mutex_unlock(&ready_queue_mutex);
    schedule(cpu_id);

    /*
     * REMOVE THE LINE BELOW AFTER IMPLEMENTING IDLE()
     *
     * idle() must block when the ready queue is empty, or else the CPU threads
     * will spin in a loop.  Until a ready queue is implemented, we'll put the
     * thread to sleep to keep it from consuming 100% of the CPU time.  Once
     * you implement a proper idle() function using a condition variable,
     * remove the call to mt_safe_usleep() below.
     */
    //mt_safe_usleep(1000000);
}


/*
 * preempt() is the handler called by the simulator when a process is
 * preempted due to its timeslice expiring.
 *
 * This function should place the currently running process back in the
 * ready queue, and call schedule() to select a new runnable process.
 */
extern void preempt(unsigned int cpu_id)
{
    	pcb_t* process;
	printf("Preempt\n");
	pthread_mutex_lock(&current_mutex);
	process = current[cpu_id];
	pthread_mutex_unlock(&current_mutex);
	push_ready_queue(process);
	schedule(cpu_id);
    /* FIX ME */
}


/*
 * yield() is the handler called by the simulator when a process yields the
 * CPU to perform an I/O request.
 *
 * It should mark the process as WAITING, then call schedule() to select
 * a new process for the CPU.
 */
extern void yield(unsigned int cpu_id)
{
    /* FIX ME */
	printf("Yield\n");
	/*pop the current process out of */
	pthread_mutex_lock(&current_mutex);
	current[cpu_id]->state = PROCESS_WAITING;
	pthread_mutex_unlock(&current_mutex);
	schedule(cpu_id);
}


/*
 * terminate() is the handler called by the simulator when a process completes.
 * It should mark the process as terminated, then call schedule() to select
 * a new process for the CPU.
 */
extern void terminate(unsigned int cpu_id)
{
    /* FIX ME */
	printf("Terminate\n");
	pthread_mutex_lock(&current_mutex);
	current[cpu_id]->state = PROCESS_TERMINATED;
	pthread_mutex_unlock(&current_mutex);
	schedule(cpu_id);
}


/*
 * wake_up() is the handler called by the simulator when a process's I/O
 * request completes.  It should perform the following tasks:
 *
 *   1. Mark the process as READY, and insert it into the ready queue.
 *
 *   2. If the scheduling algorithm is static priority, wake_up() may need
 *      to preempt the CPU with the lowest priority process to allow it to
 *      execute the process which just woke up.  However, if any CPU is
 *      currently running idle, or all of the CPUs are running processes
 *      with a higher priority than the one which just woke up, wake_up()
 *      should not preempt any CPUs.
 *	To preempt a process, use force_preempt(). Look in os-sim.h for 
 * 	its prototype and the parameters it takes in.
 */
extern void wake_up(pcb_t *process)
{
    /* FIX ME */
	printf("Wake up\n");
	process->state = PROCESS_READY;
	push_ready_queue(process);
	if (1 == scheduling_algorithm) {
		int i, lowest_process_priority, found_preemp_cpu_index, found_idle_cpu_index, process_priority;
		found_preemp_cpu_index = -1;
		found_idle_cpu_index = -1;

		lowest_process_priority = 11;
		process_priority = process->static_priority;
		pthread_mutex_lock(&current_mutex);
		for (i=0; i<count_cpu; i++)
		{
			/*if there exists idle cpu, we don't need to preemp anything*/
			if (NULL == current[i]) {
				found_idle_cpu_index = i;
				break;
			}
		if (process_priority > current[i]->static_priority) {
		  if (current[i]->static_priority < lowest_process_priority) {
					found_preemp_cpu_index = i;
					lowest_process_priority = current[i]->static_priority;
				}
			}
		}
	    pthread_mutex_unlock(&current_mutex);
		if (found_preemp_cpu_index >= 0 && found_idle_cpu_index < 0) {
			force_preempt(found_preemp_cpu_index);
		}
	}
}


/*
 * main() simply parses command line arguments, then calls start_simulator().
 * You will need to modify it to support the -r and -p command-line parameters.
 */
int main(int argc, char *argv[])
{
    int cpu_count;

    /* Parse command-line arguments */
    if (argc != 2)
    {
        fprintf(stderr, "CS 2200 Project 4 -- Multithreaded OS Simulator\n"
            "Usage: ./os-sim <# CPUs> [ -r <time slice> | -p ]\n"
            "    Default : FIFO Scheduler\n"
            "         -r : Round-Robin Scheduler\n"
            "         -p : Static Priority Scheduler\n\n");
        return -1;
    }
     else if (argc == 2)
    {
    	/*First in first out*/
    	scheduling_algorithm = 0;
    }
    else if (argc == 3)
    {
    	/*Static priority*/
    	scheduling_algorithm = 1;
    }
    else if (argc == 4)
    {
    	/*Round robin*/
    	scheduling_algorithm = 2;
    	rr_timeslice = atoi(argv[3]);
    }
    cpu_count = atoi(argv[1]);
    count_cpu = cpu_count;

    /* FIX ME - Add support for -r and -p parameters*/

    /* Allocate the current[] array and its mutex */
    current = malloc(sizeof(pcb_t*) * cpu_count);
    assert(current != NULL);
    pthread_mutex_init(&current_mutex, NULL);
    pthread_mutex_init(&ready_queue_mutex, NULL);

    /* Start the simulator in the library */
    start_simulator(cpu_count);
    free(current);

    return 0;
}


