#include <stdio.h>
#include <lab2.h>
#include <q.h>
#include <kernel.h>
#include <proc.h>

/* Scheduler identifier */
int current_sched_class = 0;

/* Used for storing current epoch duration */
int current_epoch_time;

/* Similar to proctab to store counter and goodness for each value */

void setschedclass(int sched_class)
{
	// Set the scheduler class based on the input
	if(sched_class == EXPDISTSCHED){
		current_sched_class = EXPDISTSCHED;
	}else if(sched_class == LINUXSCHED){
		current_sched_class = LINUXSCHED;
	}else{
		// Default to normal scheduler if an unknown class is provided
		kprintf("\n Using normal scheduler............\n");
	}
}

int getschedclass()
{
	return current_sched_class; // Return the current scheduler class
}

/* Retrieve the next process in ready queue according to random value - For Exponential Scheduler */
int get_next_process_exp(int random)
{
	int next = rdyhead;
	int prev = next;
	next=q[next].qnext;

	// since we are starting execution from head, we iterate until the key's value is less than equal to random
	while (next != rdytail && q[next].qkey <= random)
	{
		next = q[next].qnext;
	}	
	
	// once a key's value exceeds the random value, we exit from the loop and check if the position is at tail
	if(next == rdytail)
	{
		return q[rdytail].qprev; // Return the previous process if the end of the queue is reached
	}else{
		// we find the farthest key that has exceeded the key value since we are iterating from head
		prev = next;
		while (next != rdytail && q[next].qkey == q[prev].qkey){
			prev = next;
			next = q[next].qnext;
		}
	}

	return prev; // Return the selected process ID
}
