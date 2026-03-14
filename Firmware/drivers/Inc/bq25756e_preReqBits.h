#pragma once
#include "common.h"
#include "event_groups.h"

typedef enum {
    BQ25756E_PREREQ_LTC_VALID,
    BQ25756E_PREREQ_SUPP_VALID,
    BQ25756E_NUM_PREREQS
} bq25756e_prereqs_t;

#define BQ25756E_ALL_PREREQ_BITS    ((1UL << BQ25756E_NUM_PREREQS) - 1UL)
#define BQ25756E_PREREQ(prereq)   (1UL << (prereq))

extern EventGroupHandle_t BQ25756E_preReqBits;
extern StaticEventGroup_t BQ25756E_preReqBitsBuffer;

/**
 * @brief Initializes the BQ25756E pre-requisite event group.
 *
 * Creates a static FreeRTOS event group to track pre-requisite flags
 * required by the BQ25756E driver. Must be called before using
 * set/wait functions.
 *
 * @return uint8_t Returns pdPASS if the event group was successfully created,
 *                 pdFAIL if creation failed.
 */
uint8_t bq25756e_preReqBits_init(void);

/**
 * @brief Sets a pre-requisite bit in the BQ25756E event group.
 *
 * Marks a specific precondition as satisfied by setting the corresponding
 * bit in the event group. Calling this may unblock tasks waiting
 * for the bit.
 *
 * @param bit Pre-requisite bit to set. Type is bq25756e_prereqs_t.
 *            Must be less than BQ25756E_NUM_PREREQS.
 *
 * @note If an invalid bit is provided (>= BQ25756E_NUM_PREREQS), the function
 *       does nothing.
 * @note A call to taskYIELD() is made to allow higher-priority tasks
 *       waiting on this bit to execute immediately.
 */
void bq25756e_set_preReqBit(bq25756e_prereqs_t bit);

/**
 * @brief Waits for a pre-requisite bit to be set.
 *
 * Blocks the calling task until one or more specified pre-requisite bits
 * are set or until the timeout expires. Uses FreeRTOS event group APIs.
 *
 * @param bit Pre-requisite bit to wait for (bq25756e_prereqs_t).  
 *            If `bit == BQ25756E_NUM_PREREQS`, waits for any bit in
 *            the event group (all defined pre-reqs).
 * @param xTicksToWait Maximum time to wait for the bit(s) to be set,
 *                     in FreeRTOS ticks.
 *
 * @return EventBits_t Returns the bit pattern of currently set bits
 *                     at the time of return. If no bits are set before
 *                     the timeout, returns 0.
 *
 * @note The pre-requisite bits are **not cleared** automatically after waiting.
 *       Use `bq25756e_set_preReqBit()` to set bits again if needed.
 */
EventBits_t bq25756e_preReqBit_wait(bq25756e_prereqs_t bit, TickType_t xTicksToWait);