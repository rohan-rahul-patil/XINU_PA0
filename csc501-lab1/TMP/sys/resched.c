/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <sched.h>
#include <math.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */

int sched_class;

//static int c = 0;

static int epoch = 0;

int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

	/* no switch needed if current process priority higher than next*/
	
	if(sched_class == EXPDISTSCHED) {
		//kprintf("itertion %d\n", c++);
		
		int random_priority;
		int new_process, next;		

		random_priority = (int)expdev(0.1);

		//kprintf("priority = %d\n", random_priority);
		
		/*next = q[rdyhead].qnext;
		kprintf("ready queue:");
                while (q[next].qkey < MAXINT) {      
			kprintf(" %s ", proctab[next].pname);
                        next = q[next].qnext;
		}
		kprintf("\n");*/

		if(currpid == 0 && (q[rdyhead].qnext == rdytail))
			return(OK);

		new_process = q[rdyhead].qnext;
		while(q[new_process].qkey <= random_priority) {
			new_process = q[new_process].qnext;
		}
		
		if ( ( (optr= &proctab[currpid])->pstate == PRCURR) && 
		((random_priority < optr->pprio) && (optr->pprio < q[new_process].qkey))) {
		
			 #ifdef  RTCLOCK
	                preempt = QUANTUM;              /* reset preemption counter     */
        	        #endif

			return(OK);
		}
		
		/* force context switch */

		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}

		/* remove selected new process */

		if(lastkey(rdytail) <= random_priority) {
			//kprintf("entered here\n");
			nptr = &proctab[ (currpid = getlast(rdytail)) ];
		}
		else {
			nptr = &proctab[ (currpid = dequeue(new_process)) ];
		}
	
		nptr->pstate = PRCURR;		/* mark it currently running	*/
		#ifdef	RTCLOCK
		preempt = QUANTUM;		/* reset preemption counter	*/
		#endif
	
		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
		/* The OLD process returns here when resumed. */
		return OK;
	}

	else if(sched_class == LINUXSCHED) {
		
		int new_epoch = 1, new_process, next, i, max, found_first;
	 /* no switch needed if preemt timer value is greater than zero*/

              	 if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
                (preempt > 0)) {
                        return(OK);
                }
		
		optr = &proctab[currpid];
		
		if(q[rdyhead].qnext == rdytail)
			return(OK);

                /* force context switch */

		next = q[rdyhead].qnext;
		while(q[next].qkey < MAXINT) {
			if(proctab[next].goodness != 0)
				new_epoch = 0;
			next = q[next].qnext;
		}


		if(new_epoch == 1) {
			//kprintf("\n\n\n entered new epoch\n\n");
			for(i = 0; i < NPROC; i++) {
				proctab[i].quantum = proctab[i].counter / 2 + proctab[i].pprio;
                                proctab[i].goodness = proctab[i].counter + proctab[i].pprio;
			}
		}
		                                                                                                                                                                    
		max = MININT;

                next = q[rdyhead].qnext;
                while (q[next].qkey < MAXINT) {
                        if(next == 0){
                                next = q[next].qnext;
                                continue;
                        }
                        if(proctab[next].goodness > max) {
                                max = proctab[next].goodness;
                                new_process = next;
                        }
                        next = q[next].qnext;
                }

		//kprintf("entered here\n");

		if(max == MININT) {
                  //      kprintf("entered NULL handler\n");
                        new_process = 0;
                }

                else if(max == proctab[currpid].goodness) {
                        next = q[rdyhead].qnext;
                        found_first = 0;
                        while(q[next].qkey < MAXINT) {
                                if((proctab[next].goodness == max) && (found_first == 0)) {
                                        found_first = 1;
                                        new_process = next;
                                }
                                if((proctab[next].goodness == max) && (next > currpid)) {
                                        new_process = next;
                                        break;
                                }
                        }
                }



                if (optr->pstate == PRCURR) {
			//kprintf("current = %s  new_process = %s\n", optr->pname, proctab[new_process].pname);
			optr->goodness = 0;
			optr->quantum = 0;
                        optr->pstate = PRREADY;
                        insert(currpid,rdyhead,optr->pprio);
                }

		if(preempt > 0) 
			optr->counter = preempt;
		else
			optr->counter = 0;

		if (new_epoch == 1) {
			optr->quantum = optr->counter / 2 + optr->pprio;
                        optr->goodness = optr->counter + optr->pprio;
		}
                /* remove selected new process at end of ready list */

                nptr = &proctab[ (currpid = dequeue(new_process)) ];
                nptr->pstate = PRCURR;          /* mark it currently running    */


                #ifdef  RTCLOCK
		if(currpid == 0) 
                	preempt = QUANTUM;              /* reset preemption counter     */
               	else
			preempt = nptr->quantum;
		 #endif


                ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

                /* The OLD process returns here when resumed. */
                return OK;




	}	

	else {
		/* no switch needed if current process priority higher than next*/

		if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
	   	(lastkey(rdytail)<optr->pprio)) {
			return(OK);
		}
	
		/* force context switch */

		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}	

		/* remove highest priority process at end of ready list */

		nptr = &proctab[ (currpid = getlast(rdytail)) ];
		nptr->pstate = PRCURR;		/* mark it currently running	*/

                #ifdef  RTCLOCK
                preempt = QUANTUM;              /* reset preemption counter     */
                #endif


                ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

                /* The OLD process returns here when resumed. */
                return OK;

	}

	return OK;
}
