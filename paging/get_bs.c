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

    restore(ps);
    return npages;
  }

}


