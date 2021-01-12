/*
 * logger.h
 *
 * A minimal deferred logging utility. Hard-coded to work with STM32L5 HAL and FreeRTOS.
 * TODO: just grab some existing format code from somewhere...
 *
 *  Created on: Jan 5, 2021
 *      Author: cmklu
 */

#pragma once

#include "stm32l5xx_hal.h"

#include <cstdint>

namespace logger {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////////////////////////

/** Number of logs that can be queued in the deferred logging queue. */
inline constexpr std::uint32_t QUEUE_SIZE { 16 };

/** Max number of arguments that can be logged. */
inline constexpr std::uint32_t MAX_ARGUMENTS { 6 };

/** Size of the buffer to format into. Logs that end up longer than this are truncated. */
inline constexpr std::uint32_t FORMAT_BUFFER_SIZE { 128 };

////////////////////////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////////////////////////
struct LogData {
    const char *fmt;
    std::uint32_t arguments[MAX_ARGUMENTS];
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////////////////////////

/** Initializes the logger to run over a UART instance. */
void init(UART_HandleTypeDef *uart);

/**
 * Queues the given log data for logging
 */
void log(const LogData &data);

/**
 * Queues data to be logged.
 *
 * @param fmt  The string format to be logged. Must not be a pointer to a stack array. Supports
 *             base character, integer, and string formats (%c/%d/%x/%s), no modifiers.
 * @param args The arguments for the string format. Will be internally stored as a uint32_t.
 */
template <typename ... Args>
void log(const char *fmt, Args ... args) {
    auto constexpr arg_cnt = sizeof...(args);
    static_assert(arg_cnt <= MAX_ARGUMENTS, "Too many arguments to logger!");

    log(LogData{ fmt, ((std::uint32_t) args)... });
}

/** Process queued logs. Call from a relatively low priority thread. */
void process_logs();

/** An RTOS thread for pushing queued logs. */
void thread(void *arg);

}  // namespace logger
