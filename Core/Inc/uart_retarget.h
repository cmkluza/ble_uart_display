/*
 * uart_retarget.h
 *
 *  Created on: Dec 30, 2020
 *      Author: cmklu
 */

#ifndef INC_UART_RETARGET_H_
#define INC_UART_RETARGET_H_

#include "stm32l5xx_hal.h"

void uart_retarget_init(UART_HandleTypeDef *uart);

#endif /* INC_UART_RETARGET_H_ */
