//make dispatch for svc 

#include <stdarg.h>
#include <stdint.h>
#include "minemu/user_abi.h"
#include "minemu/boot.h"
#include "minemu/syscall.h"
#include "ioctl.c"
#include <stdbool.h>
#include <stddef.h>
#include "minemu/mmu.h"
#include "minemu/platform.h"
#include "minemu/trace.h"
#include "minemu/trap.h"
#define BOOTSTRAP_PAGE_DIRECTORY_PADDR UINT32_C(0x40010000)
#define USER_IMAGE_BASE UINT32_C(0x00400000)
#define USER_STACK_BOTTOM UINT32_C(0x007ff000)
#define USER_STACK_TOP UINT32_C(0x00800000)
#define MAX_USER_IMAGE_PAGES 16U
#define SYSTEM_ROM_TABLE_COUNT 4U
#define CPSR_MODE_MASK UINT32_C(0x1f)
#define CPSR_MODE_USR UINT32_C(0x10)
#define CPSR_THUMB UINT32_C(0x20)
#define A32_SVC_ZERO UINT32_C(0xef000000)
#define TRACE_USER_SYSCALL UINT32_C(0x48454c10)

enum { frame_words = sizeof(struct minemu_trap_frame) / sizeof(uint32_t), task_stack_words = 256 };

static uint32_t task_b_stack[task_stack_words] __attribute__((aligned(8)));
static uint32_t *task_a_frame;
static uint32_t *task_b_frame;

static void task_b(void) {
    for (;;) {
        __asm__ volatile("svc #0" : : : "memory");
    }
}

struct minemu_trap_frame *minemu_svc_dispatch(struct minemu_trap_frame *frame) {
    uint32_t mode = frame->spsr & CPSR_MODE_MASK;
    if (frame->exception_id != MINEMU_EXCEPTION_SVC || mode != CPSR_MODE_USR ||
        (frame->spsr & CPSR_THUMB) != 0) {
        frame->r[0] = (uint32_t)MINEMU_HELLO_SYSCALL_ERROR;
        return frame;
    }

    uint32_t instruction = *(const uint32_t *)(uintptr_t)frame->fault_pc;
    if (instruction != A32_SVC_ZERO || frame->r[7] != MINEMU_HELLO_SYSCALL_UART_PUTCHAR ||
        frame->r[0] > UINT8_MAX) {
        frame->r[0] = (uint32_t)MINEMU_HELLO_SYSCALL_ERROR;
        return frame;
    }

    if (!user_syscall_observed) {
        minemu_trace_event(TRACE_USER_SYSCALL);
        user_syscall_observed = true;
    }
    MINEMU_UART0->tx_data = frame->r[0];
    frame->r[0] = 0;
    return frame;
}




int32_t ioctl(int fd, uint32_t op, ...){
    va_list params;
    va_start(params, op);
    
    if(fd == 0x10003000){
        /*RNG*/
        if(op == 0){
            /*update seed*/
            unsigned int seed = va_arg(params, unsigned int);
            if(rng_set_seed(seed) ==0){
                return 0;
            }
            else{
                return MINEMU_IOCTL_ERROR;
            }
        }
        else if(op == 1){
            /*data pointer*/
            unsigned int * data = va_arg(params, unsigned int*);
            if(rng_get_data(data)==0){
                return 0;
            }
            else{
                return MINEMU_IOCTL_ERROR;
            }
        }
        else{
            return MINEMU_IOCTL_ERROR;
        }

    }
    else if(fd == 0x10004000){
        /*UART0*/
        if(op == 0){
            /*read*/
            char* dst = va_arg(params, char*);
            int n =va_arg(params, int);
            if(uart0_read(dst, n)){
                return 1;
            }
            else{
                return MINEMU_IOCTL_ERROR;
            }
        }
        else if(op == 1){
            /*write*/
            char* src = va_arg(params, char*);
            int n = va_arg(params, int);
            if(uart0_write(src, n)){
                return 1;
            }
            else{
                return MINEMU_IOCTL_ERROR;
            }
        }
        else{
            return MINEMU_IOCTL_ERROR;
        }
    }
    else if(fd == 0x10005000){
        /*UART1*/
        if(op == 0){
            /*read*/
            char* dst = va_arg(params, char*);
            int n =va_arg(params, int);
            if(uart1_read(dst, n)){
                return 1;
            }
            else{
                return MINEMU_IOCTL_ERROR;
            }

        }
        else if(op == 1){
            /*write*/
            char* src = va_arg(params, char*);
            int n = va_arg(params, int);
            if(uart1_write(src, n)){
                return 1;
            }
            else{
                return MINEMU_IOCTL_ERROR;
            }
        }
        else{
            return MINEMU_IOCTL_ERROR;
        }
    }
    else if(fd == 0x1000f000){
        /*TRACE*/
        if(op == 0){
            /*update seed*/
            unsigned int value = va_arg(params, unsigned int);
            trace_eventi(value);
        }
        else{
            /*invalid op*/
            return MINEMU_IOCTL_ERROR;
        }

    }
    else{
        return MINEMU_IOCTL_ERROR;
    }
    return 0;
}