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
	if(sched_class == EXPDISTSCHED){
		current_sched_class = EXPDISTSCHED;
	}else if(sched_class == LINUXSCHED){
		current_sched_class = LINUXSCHED;
	}else{
		kprintf("\n Using normal scheduler............\n");
	}
}


int getschedclass()
{
	return current_sched_class;
}

/* Retrieve the next process in ready queue according to random value - For Exponential Scheduler */
int getNextProcess(int random)
{
	int next = rdyhead;
	int prev = next;
	next=q[next].qnext;

	while (next != rdytail && q[next].qkey <= random)
	{
		next = q[next].qnext;
	}	
	
	if(next == rdytail)
	{
		return q[rdytail].qprev;
	}else{
		prev = next;
		while (next != rdytail && q[next].qkey == q[prev].qkey){
			prev = next;
			next = q[next].qnext;
		}
	}

	return prev;
}

void next_linux()   /* to initialize next epoch for linux based scheduler  */
{
	int i;
	struct pentry *proc;

	for (i = 0; i < NPROC; i++) 
	{
		proc = &proctab[i];
		if (proc->pstate != PRFREE)
		{
			if (proc->counter == 0 || proc->counter == proc->quantum_val) /*if no unused previous epoch quantum or process did not run at all*/ 
			{
				proc->quantum_val = proc->pprio;
			} 
			else 
			{
				proc->quantum_val = proc->pprio + (proc->counter) / 2;
			}
			proc->counter = proc->quantum_val;
			proc->goodness =  proc->pprio + proc->counter;
		}
	}	
}