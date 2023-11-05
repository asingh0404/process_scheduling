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
	int ptr = q[rdytail].qprev;	
	int proc = 0;	
	int max = 0;
	int next = 0;
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
	
		optr->goodness = optr->goodness + preempt - optr->counter; 
		optr->counter = preempt;
		
		if (preempt == 0)
		{
			optr->goodness = 0;
		}
		if (currpid == NULLPROC){
			optr->counter = 0;
            optr->goodness = 0;
		}
		else{
			optr->counter = preempt;
		}

		while(ptr != rdyhead){

			if(proctab[ptr].goodness > max){

				next = ptr;
				max = proctab[ptr].goodness;
			}

			ptr = q[ptr].qprev;
		}
		
		/* next epoch because there's no runnable process with goodness > 0 and next state of current process is not PRCURR */	
		if (optr->pstate != PRCURR || optr->counter == 0 || optr->goodness < max)
		{
            if (optr->goodness !< max && max == 0)
            {
                get_next_process_linux();  

                if (currpid != NULLPROC)
                {
                    if (optr->pstate == PRCURR) /* next state is PRCURR but counter = 0 */
                    {
                        optr->pstate = PRREADY;
                        insert(currpid, rdyhead, optr->pprio);
                    }

                    currpid = NULLPROC;
                    nptr = &proctab[NULLPROC];
                    nptr->pstate = PRCURR;
                    dequeue(NULLPROC);

                    #ifdef RTCLOCK
                        preempt = QUANTUM;
                    #endif		
                }

                else return OK;
            }

			else if(max > 0){

				if (optr->pstate == PRCURR){

					optr->pstate = PRREADY;
					insert(currpid, rdyhead, optr->pprio);

				}

				nptr = &proctab[currpid];
				nptr->pstate = PRCURR;
				dequeue(next);
				currpid = next;
				preempt = nptr->counter;

			} 

		}
 
		/* no preemption of current process as its goodness value is highest */
		else if (optr->pstate == PRCURR && optr->goodness > 0 && optr->goodness >= max)
		{
			return OK;
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
