#include "minemu/platform.h"
int rng_set_seed(unsigned int seed){
    MINEMU_RNG->seed = (uint32_t) seed;
    return 0;
}


int rng_get_data(unsigned int * data){
    MINEMU_RNG->data = (uint32_t)data;
    return 0;
}


int uart0_read(char* dst, int n){
    if(MINEMU_UART0->status & MINEMU_UART_STATUS_RX_READY){
        int count = 0;
        char c = dst[count];
        while(c != '\0'){
            count++;
            c=dst[count];
        }
        char byte = (char)MINEMU_UART0->rx_data;
        
        dst[count] = byte;
        
        if(count > n){
           return -1;
        }
        return count;
    }
    return 0;
}

int uart0_write(char* src, int n){
    for(int i = 0; i< n; i++){
        if(MINEMU_UART0->status & MINEMU_UART_STATUS_TX_READY){
            char byte = src[i];
            if(byte == '\0'){
                return i;
            }
            MINEMU_UART0->tx_data = (uint32_t)byte;
        }
    }
    return n;
}

int uart1_read(char* dst, int n){
    if(MINEMU_UART1->status & MINEMU_UART_STATUS_RX_READY){
        int count = 0;
        char c = dst[count];
        while(c != '\0'){
            count++;
            c=dst[count];
        }
        char byte = (char)MINEMU_UART1->rx_data;
        
        dst[count] = byte;
        
        if(count > n){
           return -1;
        }
        return count;
    }
    return 0;
}

int uart1_write(char* src, int n){
    for(int i = 0; i< n; i++){
        if(MINEMU_UART1->status & MINEMU_UART_STATUS_TX_READY){
            char byte = src[i];
            if(byte == '\0'){
                return i;
            }
            MINEMU_UART1->tx_data = (uint32_t)byte;
        }
    }
    return n;
}

int trace_eventi(unsigned int value){
    MINEMU_TRACE->event = value;
    return 0;
}