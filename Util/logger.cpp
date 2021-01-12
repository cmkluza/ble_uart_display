/*
 * logger.cpp
 *
 * A minimal deferred logging utility. Hard-coded to work with STM32L5 HAL and FreeRTOS.
 *
 *  Created on: Jan 5, 2021
 *      Author: cmklu
 */

#include "logger.h"

#include <etl/queue_mpmc_mutex.h>
#include <mpaland/printf.h>

#include <cctype>
#include <cstdlib>
#include <cstring>

namespace logger {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Private data
////////////////////////////////////////////////////////////////////////////////////////////////////

/* Multi-producer multi-consumer queue. */
etl::queue_mpmc_mutex<LogData, QUEUE_SIZE> g_log_queue;

/* UART instance to write logs over. */
static UART_HandleTypeDef *g_uart;

/* Whether or not the log thread has started; if it hasn't, it doesn't make sense to
 * defer since the queue uses a FreeRTOS mutex which won't do anything. */
static bool g_thread_started { false };

////////////////////////////////////////////////////////////////////////////////////////////////////
// Private prototypes
////////////////////////////////////////////////////////////////////////////////////////////////////
static void out_buffer(char character, void* arg);
static void log_now(const LogData &log_data);

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public implementations
////////////////////////////////////////////////////////////////////////////////////////////////////

void init(UART_HandleTypeDef *uart) {
    g_uart = uart;
}

void log(const LogData &data) {
    if (!g_thread_started) {
        log_now(data);
    } else {
        while (!g_log_queue.push(data)) {
            /* Retry */
        }
    }
}

void process_logs() {
    auto size = g_log_queue.size();

    if (size == 0) {
        return;
    }

    for (typeof(size) i = 0; i < size; ++i) {
        LogData log_data;
        while (!g_log_queue.pop(log_data)) {
            /* Retry */
        }
        log_now(log_data);
    }
}

// TODO - sleep when no logs are ready and awake when log is called?
// should be run on low priority anyways
void thread(void *arg) {
    (void) arg;
    g_thread_started = true;

    while (true) {
        process_logs();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Private implementations
////////////////////////////////////////////////////////////////////////////////////////////////////
/* Logging is done in one thread, so using these statics here is safe. */
static char g_buffer[FORMAT_BUFFER_SIZE] {};
static size_t g_buffer_i {};

static void out_buffer(char character, void* arg) {
    (void) arg;

    g_buffer[g_buffer_i] = character;
    ++g_buffer_i;
}

static void log_now(const LogData &log_data) {
    /* Format the log into the global buffer. Passing all the args should be fine even if they're
     * not all used; the rest will just take up some stack space for a second. */
    fctprintf(out_buffer, nullptr, log_data.fmt,
            log_data.arguments[0], log_data.arguments[1], log_data.arguments[2],
            log_data.arguments[3], log_data.arguments[4], log_data.arguments[5]);

    /* Send the formatted string out over UART */
    HAL_UART_Transmit(g_uart, (uint8_t *) g_buffer, (uint16_t) g_buffer_i, HAL_MAX_DELAY);
    g_buffer_i = 0;
}

}  // namespace logger
