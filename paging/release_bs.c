#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {

  /* release the backing store with ID bs_id */
    
  STATWORD ps;
  disable(ps);

  if(bs_id < 0 || bs_id >= NBSM){
    restore(ps);
    return SYSERR;
  }

  if(bsm_tab[bs_id].bs_status == BSM_UNMAPPED){
    restore(ps);
    return SYSERR;
  }

  if(bsm_tab[bs_id].priv_bs == 1){		
		
    free_bsm(bs_id);
		restore(ps);
		return OK;
	}

  /*initialize to original default values */
  bsm_tab[bs_id].bs_npages[currpid] = 0;
  bsm_tab[bs_id].bs_pid[currpid] = 0;
  bsm_tab[bs_id].bs_vpno[currpid] = 4096;

  if(proctab[currpid].bs_to_proc[bs_id] == 1){
    
    bsm_tab[bs_id].proc_cnt--;
    proctab[currpid].bs_to_proc[bs_id] = 0;
  }

  if(bsm_tab[bs_id].proc_cnt == 0){
    free_bsm(bs_id);
  }

  restore(ps);
  return OK;

}

