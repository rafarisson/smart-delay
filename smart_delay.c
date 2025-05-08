/******************************************************************************
 * File: smart_delay.c
 * Author: Rafael F. P. Risson
 * License: MIT License
 * 
 * Description: 
 *   A C library for non-blocking delays with step-based sequencing support,
 *   ideal for embedded systems using polling or cooperative multitasking.
 *
 * Copyright (c) 2025 Rafael F. P. Risson. All rights reserved.
 * 
 * This software is released under the MIT License. 
 * See LICENSE file for details.
 ******************************************************************************/
 
#include "smart_delay.h"

smart_delay_status_t smart_stepped_delay(int id, smart_delay_t *sd, uint32_t delay, uint32_t current_time, int current_step) {
    if (!sd)
        return SMART_DELAY_INVALID;

    if (!sd->active) {
        sd->id = id;
        sd->step = current_step;
        sd->duration = delay;
        sd->start_time = current_time;
        if (sd->duration == 0)
            return SMART_DELAY_ELAPSED;
        sd->active = true;
        return SMART_DELAY_WAITING;
    }

    if (sd->id != id || sd->step != current_step)
        return SMART_DELAY_INVALID;

    if (current_time - sd->start_time >= sd->duration)
        sd->active = false;

    return sd->active ? SMART_DELAY_WAITING : SMART_DELAY_ELAPSED;
}

bool smart_delay_is_elapsed(int id, smart_delay_t *sd, uint32_t delay, uint32_t current_time) {
    smart_delay_status_t status = smart_stepped_delay(id, sd, delay, current_time, 0);
    return status == SMART_DELAY_ELAPSED;
}

int smart_delay_get_current_step(smart_delay_t *sd) {
    return sd ? sd->step : -1;
}