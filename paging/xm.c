/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
  STATWORD ps;
	disable(ps);
	
	if(bsm_tab[source].bs_status== BSM_UNMAPPED){
		restore(ps);
		return SYSERR;
	}
  return SYSERR;
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
  /* xm unmap fucntions the same as bs_unmap */
  if(bsm_unmap(currpid, (virtpage), 0)!=OK){
	  return SYSERR;
  }
  return OK;
}
