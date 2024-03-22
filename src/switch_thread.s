.extern current
.section .text
.global switch_thread
switch_thread:
    pushq %rbp
    movq %rsp, %rbp

    /* 保存现场 */
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    pushf

    /* 切换栈顶指针 */
    /* 第一个参数current（tcb的地址），存在%rdi中 */
    /* 第二个参数next（tcb的地址），存在%rsi中 */
    /* 第二个参数current的地址（current的地址），存在%rdx中 */
    movq %rsp, 16(%rdi)  /* 把栈顶指针%rsp存到current指向的tcb中的rsp成员（基址偏移16字节） */
    movq 16(%rsi), %rsp  /* 把next指向的tcb中的rsp成员（基址偏移16字节）存到栈顶指针%rsp中 */
    movq %rsi, (%rdx)    /* 把next赋值给current */

    /* 解除alarm信号的屏蔽 */
    /* 要先解除再恢复现场，因为open_alarm会更改寄存器状态*/
    call open_alarm

    /* 恢复现场 */
    popf
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    popq %rbp

    ret
