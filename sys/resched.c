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

	/* no switch needed if current process priority higher than next*/
	if (getschedclass() == EXPDISTSCHED)
	{

		/* Force Context Switch */
		optr = &proctab[currpid];

		if (optr->pstate == PRCURR)
		{
			optr->pstate = PRREADY;
			insert(currpid, rdyhead, optr->pprio);
		}

		/* Generate random value based on exponential distribution */
		int exp_rand = (int)expdev(LAMBDA);

		/* Select next process based on random value */
		int newpid = getNextProcess(exp_rand);

		// if(newpid == EMPTY){
		// 	currpid = NULLPROC;
		// }
		// else{
		// 	currpid = dequeue(newpid);
		// }

		currpid = dequeue(newpid);

		nptr = &proctab[currpid];
		nptr->pstate = PRCURR; /* mark it currently running	*/
		#ifdef RTCLOCK
				preempt = QUANTUM; /* reset preemption counter	*/
		#endif
	}

// 	else if(getschedclass() == LINUXSCHED)
// 	{
// 		int exec_time, temp;
// 		/* While printing preempt values, preempt sometimes holds negative values. */
// 		if(preempt < 0)
// 		{
// 			temp = 0;
// 			exec_time = schedtab[currpid].counter;
// 			// schedtab[currpid].counter = 0;
// 		}else{
// 			/* If there was some value left in preempt, means that process went to sleep or something therefore decrementing it from counter
//  * 		 	   as it does not contribute toward execution time of process */
// 			temp = preempt;
// 			exec_time = schedtab[currpid].counter - temp;
// 			// schedtab[currpid].counter = preempt;
// 		}	
		
// 		/* Decrement execution time from process epoch */
// 		current_epoch_time = current_epoch_time - exec_time;
		
// 		/* Updating the old process's counter with preempt value or 0 if it used up its time quantum */
// 		schedtab[currpid].counter = temp;
		
// 		/* Checking if process has used up its quantum to determine its goodness value. */
// 		if(schedtab[currpid].counter == 0)
// 		{
// 			schedtab[currpid].goodness = 0;
// 		}
// 		else
// 		{
// 			schedtab[currpid].goodness = schedtab[currpid].counter + schedtab[currpid].pprio;
// 		}
		
// 		/* Force Context Switch based on Goodness value ensuring Round Robin */
// 		optr = &proctab[currpid];
// 		if (optr->pstate == PRCURR)
// 		{
// 			optr->pstate = PRREADY;
// 			insert(currpid, rdyhead, schedtab[currpid].goodness);
// 		}
		
// 		/* Check if any runnable process exists. Start new epoch if none exist */
// 		if(!runnableExists())
// 		{
// 			/* Starting new epoch */
// 			current_epoch_time = 0;

// 			int i;

// 			/* Checking all the processes in the proctab queue if they can be scheduled in the new epoch */
// 			for(i = 0; i < NPROC; i++)
// 			{
// 				/* Check if any new process is ready to execute */
// 				if(proctab[i].pstate != PRFREE)
// 				{	
// 					/* Note process priorities in the start of epoch */
// 					schedtab[i].pprio = proctab[i].pprio;
					
// 					/* If process was scheduled in previous epoch, then update counter and goodness accordingly */
// 					if(schedtab[i].epoch_flag)
// 					{
// 						schedtab[i].goodness = schedtab[i].pprio + schedtab[i].counter;
// 						schedtab[i].counter =  schedtab[i].pprio + (schedtab[i].counter/2); 
// 					}
// 					else /* If process was created in the previous epoch, initialize counter and goodness for the process */
// 					{
// 						schedtab[i].goodness = schedtab[i].pprio;
// 						schedtab[i].counter = schedtab[i].pprio;
// 					}
					
// 					/* Allow the process to run in new epoch */
// 					schedtab[i].epoch_flag = 1;
					
// 					/* Calculate the new epoch time based on the updated counter values of each process */
// 					current_epoch_time = current_epoch_time + schedtab[i].counter;
// 				}
// 				else
// 				{	/* Non-existent process slots which will not be considered in new epoch */
// 					schedtab[i].epoch_flag = 0;
// 				}
// 			}	
				
// 		}
		
// 		/* Schedule new process based on highest goodness value among processes with highest goodness value*/	
// 		nptr = &proctab[(currpid = dequeue(getNextProcessLinux()))];

// 		/* New process should run only for ticks specified by counter in current epoch */
// 		if(currpid==0)
// 		{
// 			/* If NULL process is the only process in ready queue, its counter should not be used */
// 			preemptionVal = QUANTUM;
// 		}
// 		else
// 		{
// 			preemptionVal = schedtab[currpid].counter;
// 		}

// 		nptr->pstate = PRCURR;
// 		#ifdef	RTCLOCK
// 			preempt = preemptionVal;		/* reset preemption counter	*/
// 		#endif
// 	}

	else if(getschedclass() == LINUXSCHED)
	{	
		
		optr = &proctab[currpid];
		
		optr->goodness += preempt - optr->counter;   /* dynamic priority*/
		optr->counter = preempt;
		
		if (preempt <= 0 || currpid == NULLPROC) /* if process used up its time quantum */
		{
			optr->counter = 0;
			optr->goodness = 0;
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
		
		/* next epoch as there is no runnable process having goodness value > 0 and next state of current process is not PRCURR */	
		if ((optr->pstate != PRCURR || optr->counter == 0) && max == 0)
		{
			next_epoch();  
			preempt = optr->counter;
			
			if (max == 0)
			{
				if (currpid == NULLPROC)
				{ return OK;}
				else
				{
					if (optr->pstate == PRCURR) /* next state is PRCURR so process wants to continue but counter = 0 */
					{
						optr->pstate = PRREADY;
						insert(currpid,rdyhead,optr->pprio);
					}
		
				 	nptr = &proctab[ (currpid = dequeue(NULLPROC)) ];
                                        nptr->pstate = PRCURR;
					
					#ifdef RTCLOCK
                                                preempt = QUANTUM;
                                        #endif

                                        ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
                                        return OK;			
				}	
			}				
		}
 
		/* no preemption of current process as its goodness value is highest */
		else if (optr->pstate == PRCURR && optr->goodness > 0 && optr->goodness > max)
		{
			preempt = optr->counter;
			return(OK);
		}

		else if ((optr->counter == 0 || optr->pstate != PRCURR || optr->goodness < max) && max > 0)
		{
			if (optr->pstate == PRCURR)
                        {
                        	optr->pstate = PRREADY;
                                insert(currpid,rdyhead,optr->pprio);
                        }

			nptr = &proctab[ (currpid = dequeue(proc)) ];
			nptr->pstate = PRCURR;

                        
                        preempt = nptr->counter; 	
		}
	}

	else
	{

		if (((optr = &proctab[currpid])->pstate == PRCURR) &&
			(lastkey(rdytail) < optr->pprio))
		{
			return (OK);
		}

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
