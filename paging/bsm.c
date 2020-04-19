/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */

/* initializing the array of backing stores */
 bs_map_t bsm_tab[NBSM];    /* declared in paging.h */

SYSCALL init_bsm()
{
    STATWORD ps;
    disable(ps);
    
    int i;
    for(i=0; i<NBSM; i++){
        bsm_tab[i].bs_status = BSM_UNMAPPED;
        int j;
        /* initializing for all processes */
	    for(j = 0; j < 50; j++){
            bsm_tab[i].bs_pid[j] = 0;
            bsm_tab[i].bs_vpno[j] = -1; 
            bsm_tab[i].bs_npages[j] = 0;
	    }	
	    bsm_tab[i].bs_sem = 0;
    }
    restore(ps);
    return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
    STATWORD ps;
    disable(ps);
    *avail = 0;

    int i;
    for(i=0; i<NBSM; i++){
        if(bsm_tab[i].bs_status = BSM_UNMAPPED){
            *avail = i;
            restore(ps);
            return OK;
        }
    }
    restore(ps);
    /* if there is no unmapped backing store available, then it should return error */
    return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
    STATWORD ps;
    disable(ps);
    /* check if bsid is bad or not */
    if(i<0 || i>=NBSM){
        restore(ps);
        return SYSERR;
    }

    /* initialize or reset all values to 0 or original values */\
    bsm_tab[i].bs_status = BSM_UNMAPPED;
    int j;
    /* initializing for all processes */
    for(j = 0; j < 50; j++){
        bsm_tab[i].bs_pid[j] = 0;
        bsm_tab[i].bs_vpno[j] = -1; 
        bsm_tab[i].bs_npages[j] = 0;
    }	
    bsm_tab[i].bs_sem = 0;

    restore(ps);
    return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
}


