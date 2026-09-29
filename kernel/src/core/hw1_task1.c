#include "minemu/platform.h"

void print_input(char* str){
    /*Takes a char array ending in '0' and prints to terminal with no new line*/
    int count = 0;
    while(1){
        //int len = sizeof(str)/sizeof(str[0]);
        if(MINEMU_UART0->status & MINEMU_UART_STATUS_TX_READY){
            char byte = str[count];
            if(byte == '0'){
                break;
            }
            MINEMU_UART0->tx_data = (uint32_t)byte;
            count++;
        }
    }
}