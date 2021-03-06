#include <xen3-public/arch-x86_32.h>
#include <os.h>

//----------------------------------------------------------------------
//  $Id: entry.S,v 1.3 2006/01/29 11:44:26 nightcreature Exp $
//----------------------------------------------------------------------
//
//  $Log: head.S,v $
//
//----------------------------------------------------------------------

// Based on i386/kernel/entry.S from xen sparse tree
// Unclear if the above file falls under copyright of the linux source
// GPL v2 or the BSD Xen license. See COPYING in the xen-3.0-testing
// source tree for more details. 
 
ES		= 0x20
ORIG_EAX	= 0x24
EIP		= 0x28
CS		= 0x2C

#define ENTRY(X) .globl X ; X :

#define SAVE_ALL \
	cld; \
	pushl %es; \
	pushl %ds; \
	pushl %eax; \
	pushl %ebp; \
	pushl %edi; \
	pushl %esi; \
	pushl %edx; \
	pushl %ecx; \
	pushl %ebx; \
	movl $(__KERNEL_DS),%edx; \
	movl %edx,%ds; \
	movl %edx,%es;

#define RESTORE_ALL	\
	popl %ebx;	\
	popl %ecx;	\
	popl %edx;	\
	popl %esi;	\
	popl %edi;	\
	popl %ebp;	\
	popl %eax;	\
	popl %ds;	\
	popl %es;	\
	addl $4,%esp;	\
	iret;		\

ENTRY(divide_error)
	pushl $0		# no error code
	pushl $do_divide_error
 
do_exception:
	pushl %ds
	pushl %eax
	xorl %eax, %eax
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	decl %eax			# eax = -1
	pushl %ecx
	pushl %ebx
	cld
	movl %es, %ecx
	movl ES(%esp), %edi		# get the function address
	movl ORIG_EAX(%esp), %edx	# get the error code
	movl %eax, ORIG_EAX(%esp)
	movl %ecx, ES(%esp)
	movl $(__KERNEL_DS), %ecx
	movl %ecx, %ds
	movl %ecx, %es
	movl %esp,%eax			# pt_regs pointer
  pushl %edx
  pushl %eax
	call *%edi
  addl $8,%esp
           
ret_from_exception:
  movb CS(%esp),%cl
	test $2,%cl          # slow return to ring 2 or 3
	jne  safesti
  RESTORE_ALL

# A note on the "critical region" in our callback handler.
# We want to avoid stacking callback handlers due to events occurring
# during handling of the last event. To do this, we keep events disabled
# until weve done all processing. HOWEVER, we must enable events before
# popping the stack frame (cant be done atomically) and so it would still
# be possible to get enough handler activations to overflow the stack.
# Although unlikely, bugs of that kind are hard to track down, so wed
# like to avoid the possibility.
# So, on entry to the handler we detect whether we interrupted an
# existing activation in its critical region -- if so, we pop the current
# activation and restart the handler using the previous one.
ENTRY(hypervisor_callback)
        pushl %eax
        SAVE_ALL
        movl EIP(%esp),%eax
        cmpl $scrit,%eax
        jb   11f
        cmpl $ecrit,%eax
        jb   critical_region_fixup
11:     push %esp
        call do_hypervisor_callback
        add  $4,%esp
        movl shared_info,%esi #see head.S
        xorl %eax,%eax
        movb CS(%esp),%cl
    	test $2,%cl          # slow return to ring 2 or 3
        jne  safesti
safesti:movb $0,1(%esi)     # reenable event callbacks
scrit:  /**** START OF CRITICAL REGION ****/
        testb $0xFF,(%esi)
        jnz  14f              # process more events if necessary...
        RESTORE_ALL
14:     movb $1,1(%esi)
        jmp  11b
ecrit:  /**** END OF CRITICAL REGION ****/
# [How we do the fixup]. We want to merge the current stack frame with the
# just-interrupted frame. How we do this depends on where in the critical
# region the interrupted handler was executing, and so how many saved
# registers are in each frame. We do this quickly using the lookup table
# 'critical_fixup_table'. For each byte offset in the critical region, it
# provides the number of bytes which have already been popped from the
# interrupted stack frame. 
critical_region_fixup:
        addl $critical_fixup_table-scrit,%eax
        movzbl (%eax),%eax    # %eax contains num bytes popped
        mov  %esp,%esi
        add  %eax,%esi        # %esi points at end of src region
        mov  %esp,%edi
        add  $0x34,%edi       # %edi points at end of dst region
        mov  %eax,%ecx
        shr  $2,%ecx          # convert words to bytes
        je   16f              # skip loop if nothing to copy
15:     subl $4,%esi          # pre-decrementing copy loop
        subl $4,%edi
        movl (%esi),%eax
        movl %eax,(%edi)
        loop 15b
16:     movl %edi,%esp        # final %edi is top of merged stack
        jmp  11b
         
critical_fixup_table:        
        .byte 0x00,0x00,0x00                  # testb $0xff,(%esi)
        .byte 0x00,0x00                       # jne  14f
        .byte 0x00                            # pop  %ebx
        .byte 0x04                            # pop  %ecx
        .byte 0x08                            # pop  %edx
        .byte 0x0c                            # pop  %esi
        .byte 0x10                            # pop  %edi
        .byte 0x14                            # pop  %ebp
        .byte 0x18                            # pop  %eax
        .byte 0x1c                            # pop  %ds
        .byte 0x20                            # pop  %es
        .byte 0x24,0x24,0x24                  # add  $4,%esp
        .byte 0x28                            # iret
        .byte 0x00,0x00,0x00,0x00             # movb $1,1(%esi)
        .byte 0x00,0x00                       # jmp  11b
       
# Hypervisor uses this for application faults while it executes.
ENTRY(failsafe_callback)
      pop  %ds
      pop  %es
      pop  %fs
      pop  %gs
      iret
                
ENTRY(coprocessor_error)
	pushl $0
	pushl $do_coprocessor_error
	jmp do_exception

ENTRY(simd_coprocessor_error)
	pushl $0
	pushl $do_simd_coprocessor_error
	jmp do_exception

ENTRY(device_not_available)
        iret

ENTRY(debug)
	pushl $0
	pushl $do_debug
	jmp do_exception

ENTRY(int3)
	pushl $0
	pushl $do_int3
	jmp do_exception

ENTRY(overflow)
	pushl $0
	pushl $do_overflow
	jmp do_exception

ENTRY(bounds)
	pushl $0
	pushl $do_bounds
	jmp do_exception

ENTRY(invalid_op)
	pushl $0
	pushl $do_invalid_op
	jmp do_exception

ENTRY(coprocessor_segment_overrun)
	pushl $0
	pushl $do_coprocessor_segment_overrun
	jmp do_exception

ENTRY(invalid_TSS)
	pushl $do_invalid_TSS
	jmp do_exception

ENTRY(segment_not_present)
	pushl $do_segment_not_present
	jmp do_exception

ENTRY(stack_segment)
	pushl $do_stack_segment
	jmp do_exception

ENTRY(general_protection)
	pushl $do_general_protection
	jmp do_exception

ENTRY(alignment_check)
	pushl $do_alignment_check
	jmp do_exception

# This handler is special, because it gets an extra value on its stack,
# which is the linear faulting address.
# fastcall register usage:  %eax = pt_regs, %edx = error code,
#			    %ecx = fault address
ENTRY(page_fault)
	pushl %ds
	pushl %eax
	xorl %eax, %eax
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	decl %eax			/* eax = -1 */
	pushl %ecx
	pushl %ebx
	cld
	movl %es,%edi
	movl ES(%esp), %ecx		/* get the faulting address */
	movl ORIG_EAX(%esp), %edx	/* get the error code */
	movl %eax, ORIG_EAX(%esp)
	movl %edi, ES(%esp)
	movl $(__KERNEL_DS),%eax
	movl %eax, %ds
	movl %eax, %es
	movl %esp,%eax			/* pt_regs pointer */
	call do_page_fault
	jmp ret_from_exception

ENTRY(machine_check)
	pushl $0
	pushl $do_machine_check
	jmp do_exception

ENTRY(spurious_interrupt_bug)
	pushl $0
	pushl $do_spurious_interrupt_bug
	jmp do_exception
