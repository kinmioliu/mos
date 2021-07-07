.section .data
/*
* saved pbc
*/
last_pbc:
.skip 4096


new_stack_bottom:
.skip 16384
new_stack_top:


.section .text
.global switch_task
.type switch_task, @function
switch_task:
    push %ebp
    push %esp
    push %esi
    push %edi
    push %eax
    push %ebx
    push %ecx
    push %edx
    /*
    movl %ebp, last_pbc
    movl %esp, last_pbc+4
    movl %esi, last_pbc+8
    movl %edi, last_pbc+12
    movl %eax, last_pbc+16
    movl %ebx, last_pbc+20
    movl %ecx, last_pbc+24
    movl %edx, last_pbc+28
    */
    // create a new stack
    //pop %eax;
    //pop %ebx;
    //push %ebx;
/*    
    push -0x10(%ebp) // this is new_stack
    call print_asm_u32
    push 0x28(%esp) // this is test_task
    call print_asm_u32
    pop %eax
    pop %eax
*/    
    //movl $new_stack_top, %esp;
    //movl %ebx, %esp
    movl -0x10(%ebp), %eax
    addl $0x500, %eax
    movl %esp, %ebx
    movl %eax, %esp // create a new stack for test_task
    push %esp
    call print_asm_u32
    pop %eax
    call test_task
    //call $(0x24(%esp))
    movl %ebx, %esp // recovery esp
    /*
    movl last_pbc+28, %edx
    movl last_pbc+24, %ecx
    movl last_pbc+20, %ebx
    movl last_pbc+16, %eax
    movl last_pbc+12, %edi
    movl last_pbc+8, %esi
    movl last_pbc+4, %esp
    movl last_pbc, %ebp
    */
    pop %edx
    pop %ecx
    pop %ebx
    pop %eax
    pop %edi
    pop %esi
    pop %esp
    pop %ebp
    ret;

