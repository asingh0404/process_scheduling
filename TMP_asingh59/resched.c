/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lab2.h>
#include <math.h>

unsigned long currSP; /* REAL sp of current process */
extern int ctxsw(int, int, int, int);


/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
	register struct pentry *optr; /* pointer to old process entry */
	register struct pentry *nptr; /* pointer to new process entry */
	optr = &proctab[currpid];

	/* no switch needed if current process priority higher than next*/
// Check if the current scheduling class is EXPDISTSCHED
if (getschedclass() == EXPDISTSCHED)
{
    // If the current process is running, set its state to PRREADY
    // and add it to the ready queue with its priority
    if (optr->pstate == PRCURR)
    {
        optr->pstate = PRREADY;
        insert(currpid, rdyhead, optr->pprio);
    }

    // Generate a random value based on an exponential distribution
    int random_val = (int)expdev(LAMBDA);

    // Select the next process based on the random value
    int newpid = get_next_process_exp(random_val);

    // Remove the next process from the ready queue
    currpid = dequeue(newpid);

    // Set the state of the next process to PRCURR
    nptr = &proctab[currpid];
    nptr->pstate = PRCURR; /* mark it currently running	*/

    // Jump to the end of the statement
    goto endstatement;
}
	else if (getschedclass() == LINUXSCHED)
	{
		/*
			1. The function takes a pointer to a process control block (optr) as an argument.
			2. It initializes a variable max to zero, which will be used to keep track of the maximum
			   goodness value among all processes in the ready queue.
			3. It sets up a loop that will iterate over all processes in the ready queue.
			4. For each process in the ready queue, it checks whether the current process has 
			   a higher priority than the maximum goodness value found so far. If it does, it updates 
			   the maximum goodness value and stores the process ID in a variable called next.
			5. After iterating over all processes in the ready queue, it checks whether the current 
			   process has a non-zero goodness value. If it doesn't, it means that there are no processes 
			   with a higher priority than the current process, so it returns OK.
			6. If the current process has a non-zero goodness value, it means that there is at least 
			   one process with a higher priority that should be executed instead. In this case, 
			   the function performs some bookkeeping operations to update the counters and goodness 
			   values of the affected processes.
			
			Finally, it dequeues the current process from the ready queue and enqueues the 
		    next process with the highest priority. It then updates the process state and performs a 
			context switch to start executing the new process.
		*/
		int max, next;
		next = 0;
		max = 0;
	
		int curr_ptr = q[rdytail].qprev;
		for(;curr_ptr != rdyhead;curr_ptr = q[curr_ptr].qprev){
			int g= proctab[curr_ptr].goodness; 
			if(g > max){
				next = curr_ptr;
				max = g;
			}
		}

		optr->goodness =optr->goodness +  preempt - optr->counter;

		if (preempt == 0) {
			optr->goodness = 0;
		}
		if(currpid == NULLPROC){
			optr->counter = 0;
			optr->goodness = 0;
		}
		else optr->counter = preempt;

		if (optr->goodness > 0) {
			if(optr->goodness >= max){
				if(optr->pstate == PRCURR){
					return OK;
				}
			}
		}
		if(max == 0){
			if(optr->pstate != PRCURR || optr->counter == 0){

				if(NPROC > 0){
					int procid;
					struct pentry *curr_process;
						do{
							curr_process = &proctab[procid];
							if(curr_process->pstate != PRFREE){
								curr_process->counter = curr_process->pprio + (curr_process->counter)/2;
								curr_process->goodness = curr_process->pprio + curr_process->counter;
							}
							procid++;
						}while(procid < NPROC);
				}
				
				if (max == 0) {
					if (currpid) {

						
						if (optr->pstate == PRCURR) {
							optr->pstate = PRREADY;
							insert(currpid, rdyhead, optr->pprio);
						}

						nptr = &proctab[NULLPROC];
						nptr->pstate = PRCURR;
						currpid = NULLPROC;
						dequeue(currpid);
						goto endstatement;
					}else{
						return OK;
					}
				}
			}
		}else if (max > 0){
			if(optr->pstate != PRCURR) {
				
				if (optr->pstate == PRCURR) {
					optr->pstate = PRREADY;
					insert(currpid, rdyhead, optr->pprio);
				}
				
				nptr = &proctab[next];
				nptr->pstate = PRCURR;
				currpid = next;
				dequeue(currpid);

				preempt = nptr->counter;
				ctxsw((int) &optr->pesp, (int) optr->pirmask, (int) &nptr->pesp, (int) nptr->pirmask);
				return OK;
			}
			if(optr->counter == 0) {
				
				if (optr->pstate == PRCURR) {
					optr->pstate = PRREADY;
					insert(currpid, rdyhead, optr->pprio);
				}
				
				nptr = &proctab[next];
				nptr->pstate = PRCURR;
				currpid = next;
				dequeue(currpid);

				preempt = nptr->counter;
				ctxsw((int) &optr->pesp, (int) optr->pirmask, (int) &nptr->pesp, (int) nptr->pirmask);
				return OK;
			}
			if(optr->goodness < max) {
				
				if (optr->pstate == PRCURR) {
					optr->pstate = PRREADY;
					insert(currpid, rdyhead, optr->pprio);
				}
				
				nptr = &proctab[next];
				nptr->pstate = PRCURR;
				currpid = next;
				dequeue(currpid);

				preempt = nptr->counter;
				ctxsw((int) &optr->pesp, (int) optr->pirmask, (int) &nptr->pesp, (int) nptr->pirmask);
				return OK;
			}
		}
	}else{

		if ((optr->pstate == PRCURR) && (lastkey(rdytail) < optr->pprio))
			return (OK);

		/* force context switch */

		if (optr->pstate == PRCURR)
		{
			optr->pstate = PRREADY;
			insert(currpid, rdyhead, optr->pprio);
		}

		/* remove highest priority process at end of ready list */

		nptr = &proctab[(currpid = getlast(rdytail))];

		nptr->pstate = PRCURR; /* mark it currently running	*/
		goto endstatement;
	}

	endstatement:
		#ifdef	RTCLOCK
			preempt = QUANTUM;
		#endif
		ctxsw((int) &optr->pesp, (int) optr->pirmask, (int) &nptr->pesp, (int) nptr->pirmask);
		return OK;

}
