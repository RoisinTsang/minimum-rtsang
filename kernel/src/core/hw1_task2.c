#include "minemu/platform.h"
#include "minemu/trap.h"
#include <stdint.h>
#include "minemu/irq.h"
void print_input(char* str);

extern char buff[500];



static volatile uint32_t interrupt_count;
static struct minemu_trap_frame task_b_storage __attribute__((aligned(8)));
static struct minemu_trap_frame *task_a_frame;
static struct minemu_trap_frame *task_b_frame;
static void task_b(void) {
    for (;;) {
        __asm__ volatile("nop");
    }
}
void print_input(char* str);
void command_prompt(){
    print_input("msh>\0");
}
/*//read from UART0 and save data somewhere
    //print_input("handler called\0");
    //MINEMU_UART0->rx_data != 0
    //
    while(MINEMU_UART0->status & MINEMU_UART_STATUS_RX_READY){
       //idk store it somewhere 
        buff[count] = (char)MINEMU_UART0->rx_data;
        if(count > 500){
            //print_input("you talk too much\0");
            return 1;
        }
        count++;
    }
    if(buff[count-1] != '\n'){
        uart0_interrupt_handler();
    }
    buff[count] = '\0';
    return 1;*/

int uart0_interrupt_handler(){
    
    if(MINEMU_UART0->status & MINEMU_UART_STATUS_RX_READY){
        //print_input("|handler called|");
       //idk store it somewhere 
        int count = 0;
        char c = buff[count];
        while(c != '\0'){
            count++;
            c=buff[count];
        }
        char byte = (char)MINEMU_UART0->rx_data;
        
        buff[count] = byte;
        
        if(count > 500){
            print_input("you talk too much\0");
            return 1;
        }
        return count;
    }
    //print_input("end handler called");
    return 1;
}

void interp(){
    print_input("interp called");
    print_input(buff);
    print_input("\n");
}

struct minemu_trap_frame *minemu_irq_dispatchi(struct minemu_trap_frame *frame) {
    //print_input("|dispatch called|");
    //print_input(buff);
    //print_input("s");
    //MINEMU_UART0->tx_data = (uint32_t)'i';
    uint32_t source = (uint32_t) frame->exception_id;
    //print_input("after");
    //++interrupt_count;
    if (source == MINEMU_IRQ_SYSTICK) {
        MINEMU_SYSTICK->ack = MINEMU_SYSTICK_ACK;
    }else if (source == MINEMU_IRQ_UART0) {
        int i = uart0_interrupt_handler();
        if((buff[i] == '\n')){
                interp();
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

