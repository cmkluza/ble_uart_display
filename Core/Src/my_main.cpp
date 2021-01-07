/*
 * my_main.cpp
 *
 * Main module for BLE UART displayed on LCD demo.
 *
 * Copyright (c) 2020 Cameron Kluza
 * Distributed under the MIT license (see LICENSE or https://opensource.org/licenses/MIT)
 */

#include "main.h"
#include "ble.h"
#include "bleuart.h"

#include <cmsis_os.h>

#include <cstdio>

using ble::Role;

static void user_input_thread(void *arg);

static char g_in_buffer[128] {};
static char g_out_buffer[128] {};
static std::uint8_t g_out_buffer_idx {};

// TODO: implement deferred logger and stop using printf so stacks can shrink
const osThreadAttr_t ble_thread_attr = {
    .name = "ble_thread",
    .stack_size = 1024,
    .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t uin_thread_attr = {
    .name = "uin_thread",
    .stack_size = 1024,
    .priority = (osPriority_t) osPriorityNormal,
};

static ble_uart g_ble_uart {};

extern "C" int main_cpp()
{
    /* Hardware init done by the CubeMX-generated main.c,
     * only need to init app stuff here. */

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

    osKernelStart();

    while (true) {
        /* never to be reached... */
    }
}

static void user_input_thread(void *arg) {
    auto *uart = reinterpret_cast<ble_uart *>(arg);

    while (1) {
        auto c = getchar();
        if (c > 0) {
            g_out_buffer[g_out_buffer_idx] = static_cast<char>(c);
            if ((g_out_buffer[g_out_buffer_idx] == '\r' || g_out_buffer[g_out_buffer_idx] == '\n') &&
                    g_out_buffer_idx > 0) {
                g_out_buffer[g_out_buffer_idx] = '\0';
                printf("send: %s\n", g_out_buffer);
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
                printf("recv: %s\n", g_in_buffer);
            }
        }
    }
}
