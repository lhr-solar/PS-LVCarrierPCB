#pragma once

#include "CAN_FD.h"

can_status_t canbus_init();

can_status_t canbus_send(uint32_t id, uint8_t data[], TickType_t delay_ticks);

can_status_t canbus_recv(uint32_t id, uint8_t data[], TickType_t delay_ticks);