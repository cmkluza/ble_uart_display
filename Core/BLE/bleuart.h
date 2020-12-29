/*
 * bleuart.h
 *
 * Nordic UART BLE service implementation using STM CubeMX interface.
 *
 * Copyright (c) 2020 Cameron Kluza
 * Distributed under the MIT license (see LICENSE or https://opensource.org/licenses/MIT)
 */

#pragma once

#include <cstdint>

class ble_uart {
public:
	ble_uart();
	virtual ~ble_uart();

private:
	/** Size of the UART buffer for receiving. */
	static constexpr std::uint32_t UART_BUFFER_SIZE { 256 };

};
