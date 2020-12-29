/*
 * ble.h
 *
 * Module for interacting with BlueNRG module using the STM CubeMX interface.
 *
 * Copyright (c) 2020 Cameron Kluza
 * Distributed under the MIT license (see LICENSE or https://opensource.org/licenses/MIT)
 */

#pragma once

#include <cstdint>

namespace ble {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////////////////////////

enum class Role {
	SERVER,
	CLIENT
};

struct UUID_16 {
    std::uint16_t uuid;
};

struct UUID_128 {
    std::uint8_t uuid[16];
};

struct UUID {
    enum class Type {
        UUID_16,
        UUID_128,
    } type;

    union {
        UUID_16 uuid16;
        UUID_128 uuid128;
    };
};

/**
 * Callback for when events occur. The parameter type is actually
 * "hci_uart_pckt *" but STM uses "void *" for some reason...
 */
using event_callback = void (*)(void *data);

/* Number of event callbacks that can be registered. */
constexpr inline std::uint32_t CALLBACK_COUNT { 10 };

/* Size in bytes of a BLE MAC address */
constexpr inline auto BDADDR_SIZE { 6 };

////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Initialize the BLE hardware.
 *
 * @return true if successful, false otherwise (TODO: actual error codes?)
 */
bool init(Role role);

namespace advertising {
    /**
     * Start advertising with the given information.
     *
     * @return true if successful, else false.
     */
    bool start();

    /** Stops advertising. */
    void stop();

    /** Clears the advertising data. */
    void clear();

    /**
     * Adds a local name to the advertising data.
     *
     * @param[in] name: C-string containing device name to advertise.
     * @return true if successful, else false.
     */
    bool add_name(const char *name);

    /**
     * Adds a 16-bit UUID to the advertising data. UUIDs will not be advertised
     * as the complete list.
     *
     * @param[in] uuid the 16-bit UUID to advertise.
     * @return true if successful, else false.
     */
    bool add_uuid16(std::uint16_t uuid);

    /**
     * Adds a 128-bit UUID to the advertising data. UUIDs will not be advertised
     * as the complete list.
     *
     * @param[in] uuid array of 16 bytes in a 128-bit UUID to advertise.
     * @return true if successful, else false.
     */
    bool add_uuid128(const std::uint8_t *uuid);
} /* namespace advertising */

/** Scan for a device to connect to.
 * TODO */
void scan();

/**
 * Register a callback to be called when BLE events come in
 */
void register_callback(event_callback callback);

/**
 * Processes BLE events. Should be called from a main event loop repeatedly.
 */
void process_events();

/**
 * OS thread to process BLE events.
 * TODO
 */
void thread(void *arg);

} /* namespace ble */
