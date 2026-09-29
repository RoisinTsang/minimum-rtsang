#include "minemu/platform.h"
#include "minemu/trap.h"
#include <stdint.h>
#include "minemu/irq.h"
extern char buf[500];

void print_input(char* str);

void command_prompt(){
    print_input("msh>\0");
}

int uart0_interrupt_handler(){
    //read from UART0 and save data somewhere
    int count = 0;
    while(MINEMU_UART0->rx_data != 0){
       //idk store it somewhere 
        buf[count] = MINEMU_UART0->rx_data;
        if(count > 500){
            print_input("you talk too much0");
            return 1;
        }
    }
    return 1;
}

//stuff from ex:??
static volatile uint32_t interrupt_count;
static struct minemu_trap_frame task_b_storage __attribute__((aligned(8)));
static struct minemu_trap_frame *task_a_frame;
static struct minemu_trap_frame *task_b_frame;

static void task_b(void) {
    for (;;) {
        __asm__ volatile("nop");
    }
}
struct minemu_trap_frame *minemu_irq_dispatch(struct minemu_trap_frame *frame) {
    uint32_t source = (uint32_t)frame->exception_id;
    ++interrupt_count;
    if (source == MINEMU_IRQ_SYSTICK) {
        MINEMU_SYSTICK->ack = MINEMU_SYSTICK_ACK;
    } else if (source == MINEMU_IRQ_UART0) {
        if (uart0_interrupt_handler() != 1){
            //panic?
        }
        //(void)MINEMU_UART0->rx_data;
    } else if (source == MINEMU_IRQ_UART1) {
        (void)MINEMU_UART1->rx_data;
    } else if (source == MINEMU_IRQ_BLOCK) {
        MINEMU_BLOCK->ack = MINEMU_BLOCK_ACK;
    }
    MINEMU_INTERRUPT->eoi = source;
    if (task_b_frame == 0) {
        task_a_frame = frame;
        task_b_frame = &task_b_storage;
        *task_b_frame = (struct minemu_trap_frame){0};
        task_b_frame->return_lr = (uint32_t)(uintptr_t)task_b + 4;
        task_b_frame->spsr = frame->spsr;
        task_b_frame->exception_id = (int32_t)source;
        return task_b_frame;
    }
    task_b_frame = frame;
    return task_a_frame;
}

