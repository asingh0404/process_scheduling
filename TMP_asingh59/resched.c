/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lab2.h>
#include <math.h>

unsigned long currSP; /* REAL sp of current process */
extern int ctxsw(int, int, int, int);
int preemptionVal;
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
	int next = q[rdyhead].qnext;	
	int proc = 0;	
	int max = 0;
	optr = &proctab[currpid];

	/* no switch needed if current process priority higher than next*/
	if (getschedclass() == EXPDISTSCHED)
	{

		if (optr->pstate == PRCURR)
		{
			optr->pstate = PRREADY;
			insert(currpid, rdyhead, optr->pprio);
		}

		/* Generate random value based on exponential distribution */
		int exp_rand = (int)expdev(LAMBDA);

		/* Select next process based on random value */
		int newpid = getNextProcess(exp_rand);

		currpid = dequeue(newpid);

		nptr = &proctab[currpid];
		nptr->pstate = PRCURR; /* mark it currently running	*/
		#ifdef RTCLOCK
				preempt = QUANTUM; /* reset preemption counter	*/
		#endif
	}

	else if(getschedclass() == LINUXSCHED)
	{
		optr->goodness += preempt - optr->counter;   /* dynamic priority*/
		optr->counter = preempt;
		
		if (preempt < 0 || currpid == NULLPROC) /* if process used up its time quantum */
		{
			optr->counter = optr->goodness = 0;
		}

		while (next != rdytail && next < NPROC) 
		{
			if (proctab[next].goodness > max) 
			{
				proc = next;
				max = proctab[next].goodness;
			}
			next = q[next].qnext;
		}
		
		/* next epoch because there's no runnable process with goodness > 0 and next state of current process is not PRCURR */	
		if ((optr->pstate != PRCURR || optr->counter == 0) && max == 0)
		{
			next_linux();  
			preempt = optr->counter;
			
			if (max == 0)
			{
				if (currpid == NULLPROC) return OK;

				else
				{
					if (optr->pstate == PRCURR) /* next state is PRCURR but counter = 0 */
					{
						optr->pstate = PRREADY;
						insert(currpid, rdyhead, optr->pprio);
					}

					currpid = dequeue(NULLPROC);
				 	nptr = &proctab[currpid];

                    nptr->pstate = PRCURR;

					#ifdef RTCLOCK
                        preempt = QUANTUM;
                    #endif		
				}	
			}				
		}
 
		/* no preemption of current process as its goodness value is highest */
		else if (optr->pstate == PRCURR && optr->goodness > 0 && optr->goodness > max)
		{
			preempt = optr->counter;
			return(OK);
		}

		else if ((optr->counter == 0 || optr->pstate != PRCURR || optr->goodness < max))
		{
			if(max > 0){

				if (optr->pstate == PRCURR){

					optr->pstate = PRREADY;
					insert(currpid, rdyhead, optr->pprio);

				}

				currpid = dequeue(proc);
				nptr = &proctab[currpid];
				nptr->pstate = PRCURR;
				preempt = nptr->counter;

			} 	
		}
	}

	else
	{

		if ((optr->pstate == PRCURR) && (lastkey(rdytail) < optr->pprio)) return (OK);

		/* force context switch */

		if (optr->pstate == PRCURR)
		{
			optr->pstate = PRREADY;
			insert(currpid, rdyhead, optr->pprio);
		}

		/* remove highest priority process at end of ready list */

		nptr = &proctab[(currpid = getlast(rdytail))];

		nptr->pstate = PRCURR; /* mark it currently running	*/
		#ifdef RTCLOCK
			preempt = QUANTUM; /* reset preemption counter	*/
		#endif
	}

	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

	/* The OLD process returns here when resumed. */
	return OK;
}
