#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


 /* requests a new mapping of npages with ID map_id */

int get_bs(bsd_t bs_id, unsigned int npages) {

  STATWORD ps;
  disable(ps);

  /* check if the npages are valid and bs_id to map is valid */
  if((bs_id < 0 || bs_id >= NBSM) || (npages <= 0 || npages > 128)){
      restore(ps);
      return SYSERR;
  }

 
  if(bsm_tab[bs_id].bs_status == BSM_UNMAPPED){
    /* if the bs is unmapped, then map it using bsm_map */
    if(bsm_map(currpid, 4096, bs_id, npages) == SYSERR){
			restore(ps);
			return SYSERR;
		}
    bsm_tab[bs_id].shared_pages = npages;

    restore(ps);
    return npages;
  }

  if(bsm_tab[bs_id].bs_status == BSM_MAPPED){
    /* if the backing store is private */
    if(bsm_tab[bs_id].priv_bs){
      restore(ps);
      return SYSERR;
    }
  }

  if(bsm_tab[bs_id].shared_pages < npages ){
    restore(ps);
    return bsm_tab[bs_id].shared_pages;
  }
  /* add process to shared backing store  */
  else{
   
    bsm_tab[bs_id].bs_vpno[currpid] = 4096;
    bsm_tab[bs_id].bs_pid[currpid] = 1;
		bsm_tab[bs_id].bs_npages[currpid] = npages; 
    bsm_tab[bs_id].proc_cnt++;
		
		restore(ps);
    return bsm_tab[bs_id].shared_pages;
  }

}


