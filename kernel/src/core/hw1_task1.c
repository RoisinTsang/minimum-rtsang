#include "minemu/platform.h"

void print_input(char* str){
    /*Takes a char array ending in '0' and prints to terminal with no new line*/
    int count = 0;
    while(1){
        //int len = sizeof(str)/sizeof(str[0]);
        if(MINEMU_UART0->status & MINEMU_UART_STATUS_TX_READY){
            char byte = str[count];
            if(byte == '\0'){
                break;
            }
            MINEMU_UART0->tx_data = (uint32_t)byte;
            count++;
        }
    }
}

void print_char(char c){
    if(MINEMU_UART0->status & MINEMU_UART_STATUS_TX_READY){
            MINEMU_UART0->tx_data = (uint32_t)c;

    }
}

void print_int(int i){
    int n = 0;
    int tempi = i;
    while(tempi >0){
        n++;
        tempi = tempi / 10;
    }
    char str[n];
    for(int j = n; j > 0; j--){
        str[j-1] = (char)(i % (10*j)+48);
    }
    print_input(str);
}
