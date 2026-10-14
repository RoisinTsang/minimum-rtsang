#include <stdint.h>
#include "minemu/user_abi.h"
void minemu_hello_uart_putchar(uint8_t msg);

void minemu_user_main(void) {
    
    char* msg = "hello world\n";
    ioctl(MINEMU_FD_UART_IN, MINEMU_IOCTL_UART_WRITE, msg, 12);
    
    for (;;) {
        __asm__ volatile("nop");
    }
}
