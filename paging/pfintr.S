/* pfintr.S - pfintr */


    	   .text
pferrcode: .long 0
           .globl  pfintr,pferrcode 
pfintr:
    popl pferrcode
	pushfl
	cli	        /* to disable interrupt */
	pushal
	call pfint  /* to handle interrupt  */
	popal
	popfl
	iret


