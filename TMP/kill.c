/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <paging.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;
	int n;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	
	/* all the mappings are to be freed from the bs	*/
	for(n = 0; n < NBSM; n++){
		if(bsm_unmap(pid, bsm_tab[n].bs_vpno[pid], 1) == SYSERR && proctab[pid].bs_to_proc[n] == 1){
			restore(ps);
			return SYSERR;
		}
	}
	
	
	/* free all the page tables	*/
	for(n = 0; n < NFRAMES; n++){
		if(frm_tab[n].fr_pid[pid] == 1){
			if(frm_tab[n].fr_type == FR_TBL){
				free_frm(n);
			}
		}
	}

	/* reset the other frames	*/
	n = (proctab[pid].pdbr>>12)-FRAME0;
	reinit_frm(n);

	/* reset the pdbr variable	*/
	proctab[pid].pdbr = 0;

	

	
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}
