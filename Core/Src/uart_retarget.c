/*
 * uart_retarget.c
 *
 *  Created on: Dec 30, 2020
 *      Author: cmklu
 */

#include "uart_retarget.h"

#include <stdio.h>
#include <string.h>

static UART_HandleTypeDef *g_uart;

void uart_retarget_init(UART_HandleTypeDef *uart) {
    g_uart = uart;
    setvbuf(stdout, NULL, _IONBF, 0); // no buffer
    setvbuf(stdin, NULL, _IONBF, 0); // no buffer
}

/* Override the weak symbols defined in syscalls.h */

int _read(int file, char *ptr, int len) { // non-blocking
    if (!(g_uart->Instance->ISR & USART_ISR_RXNE_RXFNE)) {
        return -1; // nothing to read
    }

    int read = 0;
    while ((g_uart->Instance->ISR & USART_ISR_RXNE_RXFNE)) {
        ptr[read] = (char) g_uart->Instance->RDR;
        ++read;
    }
    return read;
}

int _write(int file, char *ptr, int len) { // blocking
    HAL_StatusTypeDef ret = HAL_UART_Transmit(g_uart, (uint8_t *) ptr, (uint16_t) len, HAL_MAX_DELAY);

    if (ret == HAL_OK) {
        return len;
    } else {
        return 0;
    }
}
