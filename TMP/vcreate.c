/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	STATWORD ps;
	disable(ps);

	int bsid = 0;
	int pid;
	/* verify hsize is within range or 0 to 128*/
	if(hsize <= 0 || hsize > 128 ){
		restore(ps);
		return SYSERR;
	}

	/* get one of the unmapped backing stores */
	if(get_bsm(&bsid) == SYSERR){
		restore(ps);
		return SYSERR; 
	}

	/* create the process and then map it to a bs */
	pid = create(procaddr,ssize,priority,name,nargs,args);
	if(bsm_map(pid, 4096, bsid, hsize) == SYSERR){
		restore(ps);
		return SYSERR;
	}
	else{

		/* update all the variable in proc struc and the bsm_tab 	*/
		proctab[pid].store = bsid;
		proctab[pid].vhpno = 4096;
		proctab[pid].vhpnpages = hsize; /* the virtal heap has hsize number of pages	*/
		proctab[pid].bs_to_proc[bsid] = 1;		
		proctab[pid].priv_heap = 1;	

		bsm_tab[bsid].priv_bs = 1;

		struct mblock *vheaplist; 
		
		proctab[pid].vmemlist->mlen = hsize*4096;
		proctab[pid].vmemlist->mnext = 4096 * 4096;

		vheaplist = BACKING_STORE_BASE + (bsid * BACKING_STORE_UNIT_SIZE);
		
		vheaplist->mlen = hsize*NBPG;
		vheaplist->mnext = NULL;
		

		restore(ps);
		/* return true if creation of proc is successful */
		return pid;
	}

	restore(ps);
	return SYSERR;
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
