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
	
	if(bsm_tab[source].bs_status == BSM_UNMAPPED){
		restore(ps);
		return SYSERR;
	}
  
  /* if the bs is private, then it cannot be used to map  */
  if(bsm_tab[source].priv_bs == 1){
    restore(ps);
		return SYSERR;
  }

  /* if the npages is less than the bs total pages used update the variables of bs*/
  if(bsm_tab[source].shared_pages >= npages){
    bsm_tab[source].bs_npages[currpid] = npages;
    bsm_tab[source].bs_pid[currpid] = 1;
    bsm_tab[source].bs_vpno[currpid] = virtpage;

    restore(ps); 
    return OK;
  }

  restore(ps); 
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
