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
        bsm_tab[i].priv_bs = 0;
        bsm_tab[i].proc_cnt = 0;

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
    bsm_tab[i].priv_bs = 0;
    bsm_tab[i].proc_cnt = 0;

    restore(ps);
    return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
    STATWORD  ps;
    disable(ps);

    unsigned long vpno = vaddr/NBPG ;
    int i;
    for(i = 0; i < NBSM; i++){
        /* max range is the vpno or bs plus the npages */
        int max_range = bsm_tab[i].bs_vpno[pid] + bsm_tab[i].bs_npages[pid];
        if(vpno >= bsm_tab[i].bs_vpno[pid] && vpno < max_range){
            /* if the process exists in that bs */
            if(bsm_tab[i].bs_pid[pid] == 1){
                /* set the pageth and store variables */
                *pageth = vpno - bsm_tab[i].bs_vpno[pid];
                *store = i;
                restore(ps);
                return OK;
            }
        }
    }

    restore(ps);
    return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
    STATWORD  ps;
    disable(ps);
    /* source is the backing store number  */
    /* check if the npages are valid and source to map is valid */
    if((source < 0 || source >= NBSM) || (npages <= 0 || npages > 128)){
        restore(ps);
        return SYSERR;
    }
    /* map the process to the variables of bsm_tab */
    bsm_tab[source].bs_status = BSM_MAPPED;
	bsm_tab[source].bs_pid[pid] = 1;
    bsm_tab[source].bs_vpno[pid] = vpno;
	bsm_tab[source].bs_npages[pid] = npages;
	bsm_tab[source].proc_cnt = 1;
    
    /* map the process variables with the bs variables */
    proctab[pid].bs_to_proc[source] = 1;
    proctab[pid].store = source;

    restore(ps);
    return OK;
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
    STATWORD ps;
    disable(ps);

    unsigned long vaddr;
	pd_t *pd_off;
	pt_t *pt_off;


    /* check if its a bad process or not  */
    if(isbadpid(pid)){
        restore(ps);
        return SYSERR;
    }

    /* check if unmapped */
    if(bsm_tab[pid].bs_status == BSM_UNMAPPED){
        restore(ps);
        return SYSERR;
	}

    /* check with lookup */
    int store, pageth;
    if(bsm_lookup(pid, vpno*4096, &store, &pageth)==SYSERR){
		restore(ps);
		return SYSERR;
	}

    /* unmapping of pages from frame and move to bs */
    int i;
    int bs_pages = bsm_tab[store].bs_npages[pid];
    int next = bsm_tab[store].bs_vpno[pid];
    for(i = 0; i < bs_pages; i++){
        
        vaddr = (next)*NBPG; 
        
        int pd_range = (vaddr>>22)*4; /* the page directory which is the last 10 bits of vaddr multiplied by the 4 byte frame entry   */
        pd_off = (proctab[currpid].pdbr)+ pd_range;
        
        int pt_range = ((vaddr&(0x0003ff000))>>12)*sizeof(pd_t); /* the 10 bits representing page table in vaddr - 12 to 21 */
        pt_off = (pd_off->pd_base*4096) + pt_range;

        if(pt_off->pt_pres == 1 && pd_off->pd_pres == 1){

            if(frm_tab[pt_off->pt_base - 1024].fr_status == FRM_MAPPED){

                /* if the frame is mapped, then move the page table to bs   */
                write_bs(pt_off->pt_base * 4096, store, pageth);

               // delete_from_sc_q(pt_off->pt_base - FRAME0);

                /* free the frame   */
                free_frm(pt_off->pt_base - FRAME0);
            }
        }
        next++;
    }
    
    /* updating process and bs variables after unmap  - reinitializing  */
    bsm_tab[store].bs_pid[pid] = 0;
	bsm_tab[store].bs_npages[pid] = 0;
	bsm_tab[store].bs_vpno[pid] = 4096;

    if( proctab[pid].bs_to_proc[store] == 1){
        proctab[pid].bs_to_proc[store] = 0;
        bsm_tab[store].proc_cnt--;
    }

    /* check if no process uses that bs */
    if(bsm_tab[store].proc_cnt == 0){
		free_bsm(store);
	}

    restore(ps);
    return OK;

}


