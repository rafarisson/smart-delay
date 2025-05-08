#ifndef __SMART_DELAY_H__
#define __SMART_DELAY_H__

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    SMART_DELAY_INVALID = 0,
    SMART_DELAY_WAITING,
    SMART_DELAY_ELAPSED
}
smart_delay_status_t;

struct smart_delay_info {
	int id;
	int step;
	bool active;
	uint32_t duration;
	uint32_t start_time;
};

typedef struct smart_delay_info smart_delay_t;

smart_delay_status_t smart_stepped_delay(int id, smart_delay_t *sd, uint32_t delay, uint32_t current_time, int current_step);
bool smart_delay_is_elapsed(int id, smart_delay_t *sd, uint32_t delay, uint32_t current_time);
int smart_delay_get_current_step(smart_delay_t *sd);

#endif
