!************************************************************************!
!									 !
! general calling convention:						 !
!									 !
! (1) Register usage is as implied in the assembler names		 !
!									 !
! (2) Stack convention							 !
!									 !
!	  The stack grows towards higher addresses.  The stack pointer	 !
!	  ($sp) points to the next available (empty) location.		 !
!									 !
! (3) Mechanics								 !
!									 !
!	  (3a) Caller at call time:					 !
!	       o  Write any caller-saved stuff not saved at entry to	 !
!		  space on the stack that was reserved at entry time.	 !
!	       o  Do a JALR leaving the return address in $ra		 !
!									 !
!	  (3b) Callee at entry time:					 !
!	       o  Reserve all stack space that the subroutine will need	 !
!		  by adding that number of words to the stack pointer,	 !
!		  $sp.							 !
!	       o  Write any callee-saved stuff ($ra) to reserved space	 !
!		  on the stack.						 !
!	       o  Write any caller-saved stuff if it makes sense to	 !
!		  do so now.						 !
!									 !
!	  (3c) Callee at exit time:					 !
!	       o  Read back any callee-saved stuff from the stack ($ra)	 !
!	       o  Deallocate stack space by subtract the number of words !
!		  used from the stack pointer, $sp			 !
!	       o  return by executing $jalr $ra, $zero.			 !
!									 !
!	  (3d) Caller after return:					 !
!	       o  Read back any caller-saved stuff needed.		 !
!									 !
!************************************************************************!

!vector table
 vector0: .fill 0x00000000 !0
 .fill 0x00000000 !1
 .fill 0x00000000 !2
 .fill 0x00000000
 .fill 0x00000000 !4
 .fill 0x00000000
 .fill 0x00000000
 .fill 0x00000000
 .fill 0x00000000 !8
 .fill 0x00000000
 .fill 0x00000000
 .fill 0x00000000
 .fill 0x00000000
 .fill 0x00000000
 .fill 0x00000000
 .fill 0x00000000 !15
!end vector table


main:
	addi $sp, $zero, initsp ! initialize the stack pointer
	lw $sp, 0($sp)

	! Install timer interrupt handler into the vector table
	la $a0, vector0				! initialize interrupt vector table pointer to $t0
	la $a1, ti_inthandler		! load handler's address to $t1 
	sw $a1, 1($a0)				! set handler's address to interrupt vector table
    !FIX ME
	ei 			!Don't forget to enable interrupts...

		
	addi $a0, $zero, 2	!load base for pow
	addi $a1, $zero, 10	!load power for pow
	la $at, POW			!load address of pow
	jalr $at, $ra		!run pow
	lw $v0, 0($sp)		! load return value to $v0	
	la $a0, vector0		! load address
	sw $v0, 0($a0)		! store pow result
	halt	
		
		

POW: 
  addi $sp, $sp, 2   ! push 2 slots onto the stack
  sw $ra, -1($sp)   ! save RA to stack
  sw $a0, -2($sp)   ! save arg 0 to stack
  beq $zero, $a1, RET1 ! if the power is 0 return 1
  beq $zero, $a0, RET0 ! if the base is 0 return 0
  addi $a1, $a1, -1  ! decrement the power
  la $at, POW	! load the address of POW
  jalr $at, $ra   ! recursively call POW
  add $a1, $v0, $zero  ! store return value in arg 1
  lw $a0, -2($sp)   ! load the base into arg 0
  la $at, MULT		! load the address of MULT
  jalr $at, $ra   ! multiply arg 0 (base) and arg 1 (running product)
  lw $ra, -1($sp)   ! load RA from the stack
  addi $sp, $sp, -2  ! pop the RA and arg 0 off the stack
  jalr $ra, $zero   ! return
RET1: addi $v0, $zero, 1  ! return a value of 1
  addi $sp, $sp, -2
  jalr $ra, $zero
RET0: add $v0, $zero, $zero ! return a value of 0
  addi $sp, $sp, -2
  jalr $ra, $zero		
	
MULT: add $v0, $zero, $zero ! zero out return value
AGAIN: add $v0,$v0, $a0  ! multiply loop
  addi $a1, $a1, -1
  beq $a1, $zero, DONE ! finished multiplying
  beq $zero, $zero, AGAIN ! loop again
DONE: jalr $ra, $zero	
		
incmin: 
	addi $s0, $zero, 0		! set second to zero
	addi $s1, $s1, 1		! increase min
	beq $s1, $a1, inchr		! if minute reaches 60, goto inchr
	beq $zero, $zero, wb	! else write back to memory
	
inchr:
	addi $s1, $zero, 0		! set minute to zero
	addi $s2, $s2, 1		! increase hour	
	beq $zero, $zero, wb	! write back to memory
			
ti_inthandler:
	!FIX ME
	addi $sp, $sp, 1		! push stack ptr		
	sw $k0, -1($sp)			! save k0
	ei						! enable interrupt
	addi $sp, $sp, 5		! push stack ptr
	sw $s0, -5($sp)			! save s0
	sw $s1, -4($sp)			! save s1
	sw $s2, -3($sp)			! save s2
	sw $a0, -2($sp)			! save a0
	sw $a1, -1($sp)			! save a1
	la $a0, secptr			! load address of second to $a0
	lw $s0, 0($a0)			! load second
	lw $s1, 1($a0)			! load minute
	lw $s2, 2($a0)			! load hour
	addi $a1, $zero, 60		! set $a1 equals 60 for checking
	addi $s0, $s0, 1		! increase second
	beq $s0, $a1, incmin	! if second reaches 60, goto incmin
wb: 
	la $a0, secptr			! load address of second to $a0
	sw $s0, 0($a0)			! save second		
	sw $s1, 1($a0)			! save min		
	sw $s2, 2($a0)			! save hour
	
	
	lw $s0, -5($sp)			! restore $s0
	lw $s1, -4($sp)			! restore $s1
	lw $s2, -3($sp)			! restore $s2
	lw $a0, -2($sp)			! restore $a0
	lw $a1, -1($sp)			! restore $a1
	addi $sp, $sp, -5		! pop 5 from stack
	di						! disable interrupt
	addi $sp, $sp, -1		! pop $k0
	lw $k0, 0($sp)			! restore $k0
	reti					! return from interrupt
	secptr:	.fill 0xF00000	! second memory address
	

	
initsp: .fill 0xA00000
ivtptr: .fill 0x000001

