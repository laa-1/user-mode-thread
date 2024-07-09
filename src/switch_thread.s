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
    /* 第1个参数next，存在%rdi中 */
    /* 第2个参数cur_tcb_value，存在%rsi中 */
    /* 第3个参数cur_tcb_addr，存在%rdx中 */
    /* 第4个参数interrupt_addr，存在%rcx中 */
    movq %rsp, (%rsi) /* 把栈顶指针%rsp存到当前tcb中的rsp成员 */
    movq (%rdi), %rsp /* 把next指向的tcb中的rsp成员存到栈顶指针%rsp中 */
    movq %rdi, (%rdx) /* 把next赋值给cur_tcb */

    /* 开中断 */
    call open_interrupt

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
