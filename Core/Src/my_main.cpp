/*
 * my_main.cpp
 *
 * Main module for BLE UART displayed on LCD demo.
 *
 * Copyright (c) 2020 Cameron Kluza
 * Distributed under the MIT license (see LICENSE or https://opensource.org/licenses/MIT)
 */

// TODO: implement semaphore on BLE interrupt so BLE can rest when doing nothing
//  NOTE: BLE IRQs come in before scheduler is running, must be wary of how this is implemented

////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "main.h"

#include "stm32l562e_discovery.h"

#include "ble.h"
#include "bleuart.h"
#include "logger.h"

#include <cmsis_os.h>

#include <cstdio>

using ble::Role;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Private prototypes
////////////////////////////////////////////////////////////////////////////////////////////////////

static void user_input_thread(void *arg);

////////////////////////////////////////////////////////////////////////////////////////////////////
// Private data
////////////////////////////////////////////////////////////////////////////////////////////////////

static char g_in_buffer[128] {};
static char g_out_buffer[128] {};
static std::uint8_t g_out_buffer_idx {};

// TODO: organize threads so they sleep and can be prioritized

static const osThreadAttr_t ble_thread_attr = {
    .name = "ble_thread",
    .stack_size = 512,
    .priority = (osPriority_t) osPriorityNormal,
};

static const osThreadAttr_t uin_thread_attr = {
    .name = "uin_thread",
    .stack_size = 1024,
    .priority = (osPriority_t) osPriorityNormal,
};

static const osThreadAttr_t log_thread_attr = {
    .name = "log_thread",
    .stack_size = 1024,
    .priority = (osPriority_t) osPriorityNormal,
};

static ble_uart g_ble_uart {};

/* Grab the UART from main.c */
extern UART_HandleTypeDef huart1;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public implementations
////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" int main_cpp()
{
    /* Hardware init done by the CubeMX-generated main.c,
     * only need to init app stuff here. */

    logger::init(&hcom_uart[COM1]);

    // Initialize the BLE as a server
    if (!ble::init(Role::SERVER)) {
        printf("BLE init failed, spinning\n");
        Error_Handler();
    }

    if (!g_ble_uart.init()) {
        printf("Couldn't init BLE UART\n");
        Error_Handler();
    }

    if (!g_ble_uart.advertise("UART Test")) {
        printf("Couldn't start advertising UART service\n");
        Error_Handler();
    }

    printf("BLE UART setup\n");

    osThreadNew(ble::thread, nullptr, &ble_thread_attr);
    osThreadNew(user_input_thread, &g_ble_uart, &uin_thread_attr);
    osThreadNew(logger::thread, nullptr, &log_thread_attr);

    osKernelStart();

    while (true) {
        /* never to be reached... */
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Private implementations
////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: this won't work with logger since buffer might get overwritten before it's logged
//  implement NRF-style string push?
static void user_input_thread(void *arg) {
    auto *uart = reinterpret_cast<ble_uart *>(arg);

    while (1) {
        auto c = getchar();
        if (c > 0) {
            g_out_buffer[g_out_buffer_idx] = static_cast<char>(c);
            if ((g_out_buffer[g_out_buffer_idx] == '\r' || g_out_buffer[g_out_buffer_idx] == '\n') &&
                    g_out_buffer_idx > 0) {
                g_out_buffer[g_out_buffer_idx] = '\0';
                logger::log("send: %s\n", g_out_buffer);
                uart->write(g_out_buffer, g_out_buffer_idx);
                g_out_buffer_idx = 0;
            } else {
                ++g_out_buffer_idx;
            }
        }

        if (uart->available() > 0) {
            auto read = uart->read(g_in_buffer, static_cast<std::uint8_t>(sizeof(g_in_buffer)));
            if (read > 0) {
                g_in_buffer[read] = '\0';
                logger::log("recv: %s\n", g_in_buffer);
            }
        }
    }
}
