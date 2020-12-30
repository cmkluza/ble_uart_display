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

    while (1) {
        ble::process_events();
    }
}
