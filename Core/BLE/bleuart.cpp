/*
 * bleuart.cpp
 *
 * Nordic UART BLE service implementation using STM CubeMX BLE API. Borrows some interface ideas
 * from Arduino. For RX, uses a single-producer, single-consumer queue filled by BLE. So, there
 * should only be one owner reading from a given BLE UART instance at any given time.
 *
 * Currently acts as GAP server/GATT peripheral only.
 *
 * Copyright (c) 2020 Cameron Kluza
 * Distributed under the MIT license (see LICENSE or https://opensource.org/licenses/MIT)
 */

#include "bleuart.h"

#include "ble.h"

extern "C" {
#   include <bluenrg_gatt_aci.h>
#   include <hci_const.h>
}

#include <cstdio>
#include <cstring>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public Implementations
////////////////////////////////////////////////////////////////////////////////////////////////////

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

    ble::register_callback(ble_uart::event_callback, this);

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

std::uint8_t ble_uart::read(char *dest, std::uint8_t amount) {
    if (amount > available()) {
        amount = static_cast<std::uint8_t>(available());
    }

    for (std::uint8_t i = 0; i < amount; ++i) {
        dest[i] = read();
    }

    return amount;
}

bool ble_uart::write(char c) {
    return write(&c, 1);
}

bool ble_uart::write(const char *str) {
    return write(str, static_cast<std::uint8_t>(std::strlen(str)));
}

bool ble_uart::write(const char *src, std::uint8_t amount) {
    auto ret = aci_gatt_update_char_value(_service_handle, _tx_handle, 0, amount, src);

    if (ret != BLE_STATUS_SUCCESS) {
        printf("%s: GATT update TX char failed: %02X\n", __func__, ret);
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Private Implementations
////////////////////////////////////////////////////////////////////////////////////////////////////
void ble_uart::event_callback(void *context, hci_uart_pckt *hci_packet) {
    auto *_this = reinterpret_cast<ble_uart *>(context);

    if (hci_packet->type != HCI_EVENT_PKT) {
        return;
    }

    auto *event_packet = reinterpret_cast<hci_event_pckt *>(hci_packet->data);

    switch (event_packet->evt) {

        case EVT_VENDOR: {
            auto *vendor_packet = reinterpret_cast<evt_blue_aci *>(event_packet->data);
            ble_uart::vendor_callback(vendor_packet, _this);
        } break;

    }
}

void ble_uart::vendor_callback(evt_blue_aci *event, ble_uart *_this) {
    switch (event->ecode) {

        case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED: {
             std::uint16_t handle;
             std::uint8_t length;
             std::uint8_t *data;

            if (ble::board() == ble::ExpansionBoard::IDB04A1) {
                auto *evt = reinterpret_cast<evt_gatt_attr_modified_IDB04A1 *>(event->data);
                handle = evt->attr_handle;
                length = evt->data_length;
                data = evt->att_data;
            } else { /* IDB05A1 */
                auto *evt = reinterpret_cast<evt_gatt_attr_modified_IDB05A1 *>(event->data);
                handle = evt->attr_handle;
                length = evt->data_length;
                data = evt->att_data;
            }

            /* TODO: this just assumes descriptor handle is +1 from char handle, is that enough? */
            if (handle != (_this->_rx_handle + 1)) {
                return;
            }

            for (std::uint8_t i = 0; i < length; ++i) {
                while (!_this->_queue.push(data[i])) {
                    /* Retry */
                }
            }
        } break;

    }
}

