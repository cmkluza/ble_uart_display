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

#include <cstdio>

using ble::Role;

extern "C" int main_cpp()
{
    /* Hardware init done by the CubeMX-generated main.c,
     * only need to init app stuff here. */

    // Initialize the BLE as a server
    if (!ble::init(Role::SERVER)) {
        printf("BLE init failed, spinning\n");
        Error_Handler();
    }

    ble_uart uart {};

    if (!uart.init()) {
        printf("Couldn't init BLE UART\n");
        Error_Handler();
    }

    if (!uart.advertise("UART Test")) {
        printf("Couldn't start advertising UART service\n");
        Error_Handler();
    }

    printf("BLE UART setup\n");

    char in_buffer[128] {};
    char out_buffer[128] {};
    std::uint8_t out_buffer_idx {};

    while (1) {
        ble::process_events();

        auto c = getchar();
        if (c > 0) {
            out_buffer[out_buffer_idx] = static_cast<char>(c);
            if ((out_buffer[out_buffer_idx] == '\r' || out_buffer[out_buffer_idx] == '\n') &&
                    out_buffer_idx > 0) {
                out_buffer[out_buffer_idx] = '\0';
                printf("send: %s\n", out_buffer);
                uart.write(out_buffer, out_buffer_idx);
                out_buffer_idx = 0;
            } else {
                ++out_buffer_idx;
            }
        }

        if (uart.available() > 0) {
            auto read = uart.read(in_buffer, static_cast<std::uint8_t>(sizeof(in_buffer)));
            if (read > 0) {
                in_buffer[read] = '\0';
                printf("recv: %s\n", in_buffer);
            }
        }
    }
}
