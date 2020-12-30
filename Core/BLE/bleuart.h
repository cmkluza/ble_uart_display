/*
 * bleuart.h
 *
 * Nordic UART BLE service implementation using STM CubeMX BLE API. Borrows some interface ideas
 * from Arduino. For RX, uses a single-producer, single-consumer queue filled by BLE. So, there
 * should only be one owner reading from a given BLE UART instance at any given time.
 *
 * Copyright (c) 2020 Cameron Kluza
 * Distributed under the MIT license (see LICENSE or https://opensource.org/licenses/MIT)
 */

#pragma once

#include <etl/queue_spsc_atomic.h>

#include <cstdint>

class ble_uart {
public:
////////////////////////////////////////////////////////////////////////////////////////////////////
// Public Constants
////////////////////////////////////////////////////////////////////////////////////////////////////
    /* 6E400001-B5A3-F393-E0A9-E50E24DCCA9E */
    static constexpr std::uint8_t SERVICE_UUID[] =
        { 0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x01, 0x00, 0x40, 0x6E };
    /* 6E400002-B5A3-F393-E0A9-E50E24DCCA9E */
    static constexpr std::uint8_t RX_CHAR_UUID[] =
        { 0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x02, 0x00, 0x40, 0x6E };
    /* 6E400003-B5A3-F393-E0A9-E50E24DCCA9E */
    static constexpr std::uint8_t TX_CHAR_UUID[] =
        { 0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x03, 0x00, 0x40, 0x6E };

public:
////////////////////////////////////////////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////////////////////////////////////////////

    /** Initialize any needed resources. */
    ble_uart();
    /** Free any owned resources. */
	virtual ~ble_uart();

	/**
	 * Perform BLE initialization such as adding this service.
	 *
	 * @return true if successful, else false.
	 */
	bool init();

	/**
	 * Starts advertising this device with a name and the BLE UART UUID.
	 *
	 * @param[in] name name to advertise this device as.
	 * @return         true if successful, else false.
	 */
	bool advertise(const char *name);

	/**
	 * TODO
	 */
	bool scan();

	/**
	 * Returns number of available characters in the RX queue.
	 *
	 * @return number of characters in the RX queue.
	 */
	std::size_t available();

	/**
	 * Reads a single character from the RX queue.
	 *
	 * @return the next character in the RX queue or 0 if empty.
	 */
	char read();

	/**
	 * Reads several characters from the RX queue into the destination. Returns the actual amount
	 * of characters read (truncated if amount exceeds what's available).
	 *
	 * @param[in] dest   the buffer to place characters into.
	 * @param     amount the number of characters to read.
	 * @return           actual number of characters read.
	 */
	std::size_t read(char *dest, std::size_t amount);

	/**
	 * Writes a single character to a remote device.
	 *
	 * @param c the character to write.
	 */
	void write(char c);

	/**
	 * Writes a C-string to a remote device.
	 *
	 * @param[in] str the C-string to write.
	 */
	void write(const char *str);

	/**
	 * Writes a given amount of characters to a remote device.
	 *
	 * @param[in] src    array of characters to write.
	 * @param     amount the number of characters to write.
	 */
	void write(const char *src, std::size_t amount);

private:
////////////////////////////////////////////////////////////////////////////////////////////////////
// Private Data
////////////////////////////////////////////////////////////////////////////////////////////////////

	/** Size of the UART queue for receiving. */
	static constexpr std::uint32_t UART_QUEUE_SIZE { 256 };
	/** UART RX queue. */
	etl::queue_spsc_atomic<char, UART_QUEUE_SIZE> _queue;
	/** Handles for service and characteristics. */
	std::uint16_t _service_handle;
	std::uint16_t _rx_handle;
	std::uint16_t _tx_handle;
};
