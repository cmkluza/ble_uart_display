/*
 * bleuart.cpp
 *
 * Nordic UART BLE service implementation using STM CubeMX BLE API. Borrows some interface ideas
 * from Arduino. For RX, uses a single-producer, single-consumer queue filled by BLE. So, there
 * should only be one owner reading from a given BLE UART instance at any given time.
 *
 * Copyright (c) 2020 Cameron Kluza
 * Distributed under the MIT license (see LICENSE or https://opensource.org/licenses/MIT)
 */

#include "bleuart.h"

#include "ble.h"

extern "C" {
    #include <bluenrg_gatt_aci.h>
}

#include <cstdio>
#include <cstring>

ble_uart::ble_uart() {

}

ble_uart::~ble_uart() {

}

bool ble_uart::init() {
    auto ret = aci_gatt_add_serv(UUID_TYPE_128, ble_uart::SERVICE_UUID,
            PRIMARY_SERVICE,
            7,
            &_service_handle);
    if (ret != BLE_STATUS_SUCCESS) {
        printf("%s: Adding GATT service failed: %02X\n", __func__, ret);
        return false;
    }

    ret = aci_gatt_add_char(_service_handle,
            UUID_TYPE_128, ble_uart::RX_CHAR_UUID,          /* 128-bit UUID */
            20,                                             /* Length of the characteristic */
            CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RESP, /* Can be written to (w/o response) */
            ATTR_PERMISSION_NONE,                           /* No permissions needed */
            GATT_NOTIFY_ATTRIBUTE_WRITE,                    /* Notify self when this is written to */
            16,                                             /* Min encryption key size */
            1,                                              /* Length is variable */
            &_rx_handle);
    if (ret != BLE_STATUS_SUCCESS) {
        printf("%s: Adding RX characteristic failed: %02X\n", __func__, ret);
        return false;
    }

    ret = aci_gatt_add_char(_service_handle,
            UUID_TYPE_128, ble_uart::TX_CHAR_UUID,          /* 128-bit UUID */
            20,                                             /* Length of the characteristic */
            CHAR_PROP_NOTIFY,                               /* Remote can get notifications */
            ATTR_PERMISSION_NONE,                           /* No permissions needed */
            0,                                              /* Don't notify self */
            16,                                             /* Min encryption key size */
            1,                                              /* Length is variable */
            &_tx_handle);
    if (ret != BLE_STATUS_SUCCESS) {
        printf("%s: Adding TX characteristic failed: %02X\n", __func__, ret);
        return false;
    }

    return true;
}

bool ble_uart::advertise(const char *name) {
    ble::advertising::stop();
    ble::advertising::clear();

    if (!ble::advertising::add_name(name)) {
        return false;
    }

    if (!ble::advertising::add_uuid128(ble_uart::SERVICE_UUID)) {
        return false;
    }

    if (!ble::advertising::start()) {
        return false;
    }

    return true;
}

/**
 * TODO
 */
bool ble_uart::scan() {
    return false;
}

std::size_t ble_uart::available() {
    return _queue.size();
}

char ble_uart::read() {
    if (_queue.empty()) {
        return 0;
    }

    char t;
    while (!_queue.pop(t)) {
        /* Retry */
    }
    return t;
}

std::size_t ble_uart::read(char *dest, std::size_t amount) {
    if (amount > available()) {
        amount = available();
    }

    for (std::size_t i = 0; i < amount; ++i) {
        dest[i] = read();
    }

    return amount;
}

// TODO
void ble_uart::write(char c) {
    (void) c;
}

void ble_uart::write(const char *str) {
    write(str, std::strlen(str));
}

void ble_uart::write(const char *src, std::size_t amount) {
    for (std::size_t i = 0; i < amount; ++i) {
        write(src[i]);
    }
}
