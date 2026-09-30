.syntax unified
.cpu cortex-a9
.arm

.global minemu_irq_trampoline
.type minemu_irq_trampoline, %function
.extern minemu_irq_dispatchi
/*struct minemu_trap_frame {
    uint32_t r[13];
    uint32_t return_lr;
    uint32_t spsr;
    int32_t exception_id;
    uint32_t fault_pc;
    uint32_t dfsr;
    uint32_t dfar;
    uint32_t user_sp;
    uint32_t user_lr;
    uint32_t reserved;
};


^^ trap frame layout for ref */

minemu_irq_trampoline:
    sub sp, sp, #88 /*allocate trap frame*/
    stmia sp, {r0-r12}
    str lr, [sp, #52] //return_lr
    mrs r1, spsr
    str r1, [sp, #56] //spsr
    ldr r1, =0x10000008 /*load source id from claim register into r1*/
    str r1, [sp, #60] //exception_id
    sub r1, lr, #4  //adjust lr to record interrupted instruction address
    str r1, [sp, #64] //fault_pc
    mov r1, #0
    str r1, [sp, #68] //save 0 to dfsr
    str r1, [sp, #72] //save 0 to dfar
    add r1, sp, #76 
    stmia r1, {sp, lr}^ 
    mov r1, #0 
    str r1, [sp, #84]
    mov r0, sp

    bl minemu_irq_dispatchi //call dispatch

    mov sp, r0  //recover sp
    ldr lr, [sp, #52] //recover return_lr
    ldr r1, [sp, #56] 
    msr spsr_cxsf, r1 //recorver spsr
    add r1, sp, #76
    ldmia r1, {sp, lr}^ 
    ldmia sp, {r0-r12} //recover regs
    add sp, sp, #88 //pop stack
    subs pc, lr, #4 