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

	/* verify hsize is within range or 0 to 128*/
	if(hsize <= 0 || hsize > 128 ){
		restore(ps);
		return SYSERR;
	}

	/* get one of the unmapped backing stores */
	int bsid = 0;
	if(get_bsm(&bsid) == SYSERR){
		restore(ps);
		return SYSERR; 
	}

	/* create the process and then map it to a bs */
	if(bsm_map(create(procaddr,ssize,priority,name,nargs,args), 4096, bs_id, hsize) == SYSERR){
		restore(ps);
		return SYSERR;
	}

	/* update all the variable in proc struc and the bsm_tab */
	
	
	restore(ps);
	/* return true if creation of proc is successful */
	return OK;
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
