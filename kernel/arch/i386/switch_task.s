.section .data
/*
* saved pbc
*/
/*
last_pbc:
.skip 4096
*/

/*
new_stack_bottom:
.skip 16384
new_stack_top:
*/
.global task2_esp
task2_esp:
    .long 0
.global task2_eip
task2_eip:
    .long 0
.global task2_ebp
task2_ebp:
    .long 0
.global task1_esp
task1_esp:
    .long 0
.global task1_eip
task1_eip:
    .long 0
.global task1_ebp
task1_ebp:
    .long 0

.extern (cur_ktask)

.section .text
.global init_task
.type init_task, @function
init_task:
    /*at this point, the esp is retaddr*/
    movl 0x4(%esp), %eax // get param1 can't use pop, it would pop the ret address
    movl 0x8(%esp),%ebx // get param2
    movl %esp, %edx
/*
    push %eax
    call print_asm_u32
    push %ebx
    call print_asm_u32
    pop %ebx
    pop %eax
*/    
    cli
    movl %ebx, %esp // set new task stack(esp)
    //movl %eax, %esi // set new task entrance
    push %ebp
    push %esp
    push %esi
    push %edi
    push %eax
    push %ebx
    push %ecx
    push %edx
/*
    push %esp
    call print_asm_u32
    pop %eax
*/
    movl %edx, %esp // restore esp
    sti
    ret

.section .text
.global run_task
.type run_task, @function
run_task:
    // save cur task register
    mov 0x4(%esp), %ebx
    movl 0x8(%esp), %eax
//    push %eax
//    call print_asm_u32
//    pop %eax
    push %ebp
    push %esp
    push %esi
    push %edi
    push %eax
    push %ebx
    push %ecx
    push %edx
   //movl $new_stack_top, %esp;  
   //movl %ebx, %esp
    movl $0x3000, %ebp
    movl %eax, %esp // switch to new task stack
//    push %eax
//    call print_asm_u32
//    pop %eax
    sub  $0x20, %esp
//    push %ebx
//    call print_asm_u32
//    pop %ebx

    // I dont know why the eax become zero after sub esp
//    push %eax
//    call print_asm_u32
//    pop %eax

    // ret
    //call test_task
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
    // restore the register
    pop %edx
    pop %ecx
    pop %eax
    pop %eax
    pop %edi
    pop %esi
//    push %esi
//    call print_asm_u32
//    pop %esi
    pop %esp// %esp
    pop %ebp
    // jmp %eax
    //jmp (%eax)
    //jmp *(%eax)
//    push %ebx
//    call print_asm_u32
//    pop %ebx
    sti // enable interrupt should befor jmp
    jmp *%ebx //test_task2
    ret;

.section .text
.global read_eip
.type read_eip, @function
read_eip:
    pop %eax
    jmp %eax

// task2 switch to task1
.section .text
.global run_task2
.type run_task2, @function
run_task2:
    // save cur task register
    mov 0x4(%esp), %ebx
    movl 0x8(%esp), %eax
//    push %eax
//    call print_asm_u32
//    pop %eax
    push %ebp
    push %esp
    push %esi
    push %edi
    push %eax
    push %ebx
    push %ecx
    push %edx
   //movl $new_stack_top, %esp;  
   //movl %ebx, %esp
    movl task1_ebp, %ecx
    movl $0xC0000000, %edx
    cmp %edx, %ecx
    jb use_ebp
     movl $2000, %ebp
    movl %eax, %esp // switch to new task stack
    sub  $0x20, %esp
    jmp begin_switch
use_ebp: 
    movl task1_ebp, %ebp
    movl task1_esp, %esp
begin_switch:
//    push %eax
//    call print_asm_u32
//    pop %eax
    //ret
//    push %ebx
//    call print_asm_u32
//    pop %ebx
    // I dont know why the eax become zero after sub esp
//    push %eax
//    call print_asm_u32
//    pop %eax

    //ret
    //call test_task
    // restore the register
    pop %edx
    pop %ecx
    pop %eax
    pop %eax
    pop %edi
    pop %esi
    pop %ecx// %esp
    pop %ebp

/*    push %eax
    call print_asm_u32
    pop %eax
*/    
    //sti
    //jmp *%ebx //jmp to task1
    movl task1_ebp, %ecx
    sti
    cmp $0xC0000000, %ecx
    jb use_eip
    //cmpl $0x18e4, %ecx
    //je use_eip
    jmp *%ebx
    ret
use_eip:
/*
    push $1
    call print_asm_u32
    pop %eax
*/    
    jmp *task1_eip

/*    
    if task1_eip > 0xc0000
        jmp *%ebx
    else:
        jmp *task1_eip
    jmp *task1_eip
*/
    ret;


// task1 switch task2
.section .text
.global run_task3
.type run_task3, @function
run_task3:
    // save cur task register
    mov 0x4(%esp), %ebx // para1 eip
    movl 0x8(%esp), %ecx // para2 esp
//    push %eax
//    call print_asm_u32
//    pop %eax
    push %ebp
    push %esp
    push %esi
    push %edi
    push %eax
    push %ebx
    push %ecx
    push %edx
    movl %eax, task2_ebp
/*
    push %eax
    call print_asm_u32
    pop %eax
*/
    // save task1 esp
    movl %esp, task1_esp(,1)
    movl %ebp, task1_ebp(,1)
    call read_eip
    cmpl $0x1000, cur_ktask
    je switch_
    //cmpl $0xC0000000, %esp
    //jge switch_
    cmp $0xC0000000, %esp
    ja switch_
/*
    push $0x1234
    call print_asm_u32
    pop %edx
*/    
    //add $0x10, %esp
    ret
/*    
    //pop %edx
    //jmp %edx
    addl $8, %edx
    movl %edx, task1_eip(,1)
    movl cur_ktask, %ecx
    cmp $0x1000, %ecx
    je switch_
    ret
*/    
switch_:
    // after call read_eip, the %eax is eip
    movl %eax, task1_eip(,1)
    //movl $0xc01022d7, task1_eip(,1)
    /*
    if (cur task is test_task2) ret
    if cur task is test_task1 continue switch
    */
    //movl $0xc010376b, task1_eip(,1)
   //movl $new_stack_top, %esp;  
   //movl %ebx, %esp
    movl $0x3000, %ebp
    movl %ecx, %esp // switch to new task stack
    sub  $0x20, %esp
//    push %ebx
//    call print_asm_u32
//    pop %ebx

    // I dont know why the eax become zero after sub esp
//    push %eax
//    call print_asm_u32
//    pop %eax

    // ret
    //call test_task
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
    // restore the register
    pop %edx
    pop %ecx
    pop %eax
    pop %eax
    pop %edi
    pop %esi
//    push %esi
//    call print_asm_u32
//    pop %esi
    pop %esp// %esp
    pop %ebp
    // jmp %eax
    //jmp (%eax)
    //jmp *(%eax)
//    push %ebx
//    call print_asm_u32
//    pop %ebx
    sti // enable interrupt should befor jmp
    jmp *%ebx //test_task2
    ret;


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

