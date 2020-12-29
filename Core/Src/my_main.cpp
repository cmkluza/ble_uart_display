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

#include <cstdio>

using ble::Role;

extern "C" int main_cpp()
{
    /* Hardware init done by CubeMX generated main
     * only need to init app stuff here. */

    // Initialize the BLE as a server
    if (!ble::init(Role::SERVER)) {
        printf("BLE init failed, spinning\n");
        Error_Handler();
    }

    // advertise a name to see if we're even started yet
    if (!ble::advertising::add_name("Hello!")) {
        printf("Couldn't add BLE name, spinning\n");
        Error_Handler();
    }

    if (!ble::advertising::start()) {
        printf("Couldn't start BLE advertising, spinning\n");
        Error_Handler();
    }

    while (1) {
        ble::process_events();
    }
}
