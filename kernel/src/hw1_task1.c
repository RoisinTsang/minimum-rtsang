#include "minemu/platform.h"

void print_input(char* str){
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