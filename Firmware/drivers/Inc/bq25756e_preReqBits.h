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

uint8_t bq25756e_preReqBits_init(void);

void bq25756e_set_preReqBit(bq25756e_prereqs_t bit);

EventBits_t bq25756e_preReqBit_wait(bq25756e_prereqs_t bit, TickType_t xTicksToWait);