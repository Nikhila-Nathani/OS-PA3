/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */

fr_map_t frm_tab[NFRAMES];

SYSCALL pfint()
{

  STATWORD ps;
  disable(ps);

  unsigned long fault_addr = read_cr2();
  int pageth = -1;
  int store = -1;
  

  /* if the fault addr is not mapped to any legal addr kill the proc */
  if(bsm_lookup(currpid, fault_addr, &store, &pageth) == SYSERR){

    kill(currpid);
    restore(ps);
    return SYSERR;
  }

  virt_addr_t *vaddr;
  int pt_off = (fault_addr>>12)&(0x000003ff); /* the page table bits */
  int pd_off = (fault_addr>>22)&(0x000003ff); /* the page directory bits - last 10*/
  int new_pt_fno, new_pt_pno = 0;
  
  /* check for a page directory */
  pd_t *pd;
  pd = (pd_t *)(proctab[currpid].pdbr + ((pd_off)*4));

  if(pd->pd_pres == 0){
    /* if the page is not in MM, retrieve it from BS  */
    new_pt_fno = new_pt();

    /* if the page wasn't retrieved */
    if(new_pt_fno == -1){
      restore(ps);
      return SYSERR;
    }
    else{
      
      /* set the base frame */
      pd->pd_base = FRAME0 + new_pt_fno;
      /* update the pd variables */
      pd->pd_write = 1;
      pd->pd_pres =1;

      /* update the frame tab struc */
      frm_tab[new_pt_fno].fr_status = FRM_MAPPED;
      frm_tab[new_pt_fno].fr_pid[currpid] = 1;
      frm_tab[new_pt_fno].fr_type = FR_TBL;
      frm_tab[new_pt_fno].fr_refcnt = 0;   
      
    }
  }
  /* check for a page table */
  pt_t *pt;
  pt = (pd->pd_base * NBPG) + (pt_off*sizeof(pt_t));
  
  /* if the page isn't mapped to a frame return error */
  if(get_frm(&new_pt_pno) == -1){
    restore(ps);
    return SYSERR;
  }

  /* insert into SC queue if a new frame is returned  */
  if(sc_flag==1){
		sc_flag = 0;
		insert_sc_queue(new_pt_pno);	
	}
  
  frm_tab[(pd->pd_base)-FRAME0].fr_refcnt += 1;

  frm_tab[new_pt_pno].fr_status = FRM_MAPPED;
  frm_tab[new_pt_pno].fr_vpno[currpid] = fault_addr/NBPG;
  frm_tab[new_pt_pno].fr_dirty = 0;
  frm_tab[new_pt_pno].fr_type = FR_PAGE;
  frm_tab[new_pt_pno].fr_pid[currpid] = 1;
  frm_tab[new_pt_pno].fr_refcnt = 1;
  
  pt->pt_global=0;
  pt->pt_write =1;
  pt->pt_user=1;
  pt->pt_dirty=1;
  pt->pt_pres =1;
  
  pt->pt_base = FRAME0 + new_pt_pno;

  read_bs((char *)(pt->pt_base * NBPG),store, pageth);	// called from bsm_lookup
  write_cr3(proctab[currpid].pdbr);

  restore(ps);
  return OK;

}


/*-------------------------------------------------------------------------
 * create a page table for the process
 *-------------------------------------------------------------------------
 */
int new_pt(){
	int fr_no = -1;
  /* if not found */
	if(get_frm(&fr_no) == -1){
		return -1;
	}
	pt_t *pt_ptr = (pt_t *)((fr_no+FRAME0)*NBPG);
	

  frm_tab[fr_no].fr_type = FR_TBL;
  frm_tab[fr_no].fr_vpno[currpid] = 4096;
	frm_tab[fr_no].fr_status = FRM_MAPPED;
  frm_tab[fr_no].fr_dirty = 0;
	frm_tab[fr_no].fr_refcnt = 0;
	
	int i;
	for(i = 0; i < NPROC; i++){
    frm_tab[fr_no].fr_vpno[k] = 4096;
		frm_tab[fr_no].fr_pid[k]=0;
		
	}
	pt_ptr->pt_base = (FRAME0 + fr_no);
	pt_ptr++;
	i=0;
	for(i = 1; i < NFRAMES ; i++){
		pt_ptr->pt_pres = 0;
		pt_ptr->pt_write = 1;
		pt_ptr->pt_user = 1;
		pt_ptr->pt_pwt = 1;
		pt_ptr->pt_pcd = 0;
		pt_ptr->pt_acc = 0;
		pt_ptr->pt_dirty = 0;
		pt_ptr->pt_mbz = 0;
		pt_ptr->pt_global = 0;
		pt_ptr->pt_avail = 0;
		pt_ptr->pt_base = 0;

		pt_ptr++;
	}
  /* since the pt is created in a frame it should be returned */
	return fr_no;
}
