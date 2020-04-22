/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{

	STATWORD ps;
	struct mblock *p, *q, *rem;
	disable(ps);

	if(nbytes == 0){
		restore(ps);
		return( (WORD *)SYSERR);
	}

	struct mblock *vptr = proctab[currpid].vmemlist;
	if(vptr->mnext ==  (struct mblock *) NULL){
		restore(ps);
		return( (WORD *)SYSERR);
	}

	nbytes = (unsigned int) roundmb(nbytes);
	for (q= vptr, p=vptr->mnext ; 
		 p != (struct mblock *) NULL ; 
		 q=p, p=p->mnext)
		if ( q->mlen == nbytes) {
			
			q->mlen =0;
			q->mnext = (struct mblock*)((unsigned)p + nbytes);
			
			restore(ps);
			return( (WORD *)p );
		} 
		else if ( q->mlen > nbytes ) {
			
			rem = (struct mblock *)( (unsigned)p + nbytes );
			q->mlen = q->mlen - nbytes;
			q->mnext = rem;
			
			restore(ps);
			return( (WORD *)p );
		}

	restore(ps);
	return( SYSERR );
}


