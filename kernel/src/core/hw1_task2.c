#include "minemu/platform.h"
#include "minemu/trap.h"
#include <stdint.h>
#include "minemu/irq.h"
void print_input(char* str);
void print_char(char c);
void cmd_nt_fnd(char* str);
int find_start(int len);
int find_word(int i, int f);
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

void interp(int len){
    int start = find_start(len);
    int end_first_word = find_word(start, len);
    char first_word[start-end_first_word+1];
    for(int a = start; a<end_first_word; a++){
        first_word[a] = buff[a];
    }
    first_word[end_first_word+1] = '\0';
    if ((buff[start] != 'e') | (buff[start+1]  != 'c') | (buff[start+2] != 'h') | (buff[start+3] != 'o') | (buff [start+4] != ' ')){
        cmd_nt_fnd(first_word);
    }
    else{
        for(int i = start+5; i < len; i++){
            print_char(buff[i]);
        }
        print_input("\n");
            
    }
        
    for(int i = 0; i<500; i++){
        buff[i] = '\0'; //clear the buffer for next time
    }
    command_prompt();
}

int find_start(int len){
    int i = 0;
    char c = buff[i];
    while((c == ' ')& (i < len) ){
        i++;
        c= buff[i];
    }
    return i;
}
int find_word(int start, int len){
    for(int i = start; i<len; i++){
        if(buff[i] == ' '){
            return i;
        }
    }
    return len;
}

void cmd_nt_fnd(char* str){
    print_input("command not found: ");
    print_input(str);
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
                interp(i);
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

