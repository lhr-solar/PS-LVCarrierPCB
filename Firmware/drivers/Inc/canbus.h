#pragma once

#include "CAN_FD.h"

/**
 * @brief Initializes  FDCAN
 *
 * This function initializes the FDCAN peripheral, sets up send/receive queues,
 * configures the HAL FDCAN driver, applies the filter configuration, and 
 * enables FDCAN interrupts.
 *
 * @param none
 *
 * @return can_status_t Returns CAN_OK on success, CAN_ERR on failure.
 */
can_status_t canbus_init();

/**
 * @brief Sends a FDCAN message.
 *
 * Places a FDCAN message into the transmit mailbox if available, otherwise
 * queues it in the send queue for later transmission.
 *
 * @param id           CAN identifier of the message to receive.
 * @param dlc          Length of the CAN message
 * @param data         Array containing the data to send.
 * @param delay_ticks  Maximum delay to wait if queue is full (FreeRTOS ticks).
 *
 * @return can_status_t Returns CAN_OK if message was successfully sent or queued,
 *                      CAN_ERR on failure.
 */
can_status_t canbus_send(uint32_t id, uint8_t dlc, uint8_t data[], TickType_t delay_ticks);

/**
 * @brief Sends a FDCAN message.
 *
 * Places a FDCAN message into the transmit mailbox if available, otherwise
 * queues it in the send queue for later transmission.
 *
 * @param id           CAN identifier of the message to receive.
 * @param dlc          Length of the CAN message
 * @param data         Array containing the data to recieve.
 * @param delay_ticks  Maximum delay to wait for the queue (FreeRTOS ticks).
 *
 * @return can_status_t Returns CAN_OK if message was successfully sent or queued,
 *                      CAN_ERR on failure.
 */
can_status_t canbus_recv(uint32_t id, uint8_t data[], TickType_t delay_ticks);