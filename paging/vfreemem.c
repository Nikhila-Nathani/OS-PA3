/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
	struct	mblock	*block;
	unsigned size;
{
	STATWORD ps;
	disable(ps);

	unsigned top;
	struct pentry *pptr;
	struct mblock *p, *q;
	size = (unsigned)roundmb(size);

	if(size == 0){
		restore(ps);
		return(SYSERR);
	}

	if(((unsigned)block+size) > (proctab[currpid].vmemlist)->mnext){
		restore(ps);
		return(SYSERR);
	}
	else if(((unsigned)block) < ((unsigned) &end)){
		restore(ps);
		return SYSERR;
	}

	if((((unsigned)block+size) == (proctab[currpid].vmemlist)->mnext)){
		
		proctab[currpid].vmemlist->mlen += size; 
		proctab[currpid].vmemlist->mnext = (unsigned)block;
		
		restore(ps);
		return OK;
	}
	else if(((unsigned)block+size) < (proctab[currpid].vmemlist)->mnext){
		
		proctab[currpid].vmemlist->mlen += size; 
		proctab[currpid].vmemlist->mnext = proctab[currpid].vmemlist->mnext - size;
		
		restore(ps);
		return OK;
	}
	return(OK);
}
