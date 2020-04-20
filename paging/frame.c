/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */

/* initializing the array of frames */
fr_map_t frm_tab[NFRAMES];

SYSCALL init_frm()
{
  STATWORD ps;
  disable(ps);

  int i;
  for(i = 0; i < NFRAMES; i++){

    frm_tab[i].fr_status = FRM_UNMAPPED;

    int j;
    for(j = 0; j < 50; j++)	{
      frm_tab[i].fr_pid[j] = 0;				
      frm_tab[i].fr_vpno[j] = 4096;
    }	
    				
    frm_tab[i].fr_refcnt = 0;			
    frm_tab[i].fr_type = FR_PAGE;	/* will initializa a page later */		
    frm_tab[i].fr_dirty = 0;

    /* initializing a page for each entry the frame */
    init_entries(i);
  }

  restore(ps);  
  return OK;
}

/* initializing the frame entries as pages - inverted page table */

void init_entries(int i){ 

  pt_t *pt_ptr = (pt_t *)((F_ENTRY+i)*NBPG);
  int j = 1;
  for(j=1 ; j < NFRAMES; j++){
    pt_ptr -> pt_pres = 0;		/* page is present?		*/
    pt_ptr -> pt_write = 0;		/* page is writable?		*/
    pt_ptr -> pt_user = 0;		/* is use level protection?	*/
    pt_ptr ->  pt_pwt	= 0;		/* write through for this page? */
    pt_ptr -> pt_pcd	= 0;		/* cache disable for this page? */
    pt_ptr -> pt_acc	= 0;		/* page was accessed?		*/
    pt_ptr -> pt_dirty = 0;		/* page was written?		*/
    pt_ptr -> pt_mbz	= 0;		/* must be zero			*/
    pt_ptr -> pt_global = 0;	/* should be zero in 586	*/	
    pt_ptr -> pt_avail = 0;	  /* for programmer's use		*/
    pt_ptr -> pt_base	= 0;    /* location of page?		*/
    pt_ptr++;
  }
}


/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
  STATWORD ps;
	disable(ps);	

	*avail = -1;
	int i;
	for(i = 0; i < NFRAMES; i++){
		if(frm_tab[i].fr_status == FRM_UNMAPPED){
		  *avail = i;
      restore(ps);
      return OK;
		}
	}
  restore(ps);
  /* if there is no unmapped frame available, then it should return error */
  return SYSERR;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
  STATWORD ps;
	disable(ps);	
  
  /* return SYSERR if the unmapped frame is to be freed */
  if(frm_tab[i].fr_status == FRM_UNMAPPED){
    restore(ps);
    return SYSERR;
  }
  else if(frm_tab[i].fr_type == FR_TBL || frm_tab[i].fr_type == FR_DIR){
    if(i>3){  /* if not global page tables nor page dir */
      /* set all values of frame and its entries to zero or original value */
      frm_tab[i].fr_status = FRM_UNMAPPED;

      int j;
      for(j = 0; j < 50; j++)	{
        frm_tab[i].fr_pid[j] = 0;				
        frm_tab[i].fr_vpno[j] = 4096;
      }	
              
      frm_tab[i].fr_refcnt = 0;			
      frm_tab[i].fr_type = FR_PAGE;	/* will initializa a page later */		
      frm_tab[i].fr_dirty = 0;

      /* initializing a page for each entry the frame */
      init_entries(i); 
    }

    restore(ps);
    return OK;

  } 

  /* if the frame is of only pages 
  else{

  }
  */
  restore(ps);
  return OK;
}



