#include <stdio.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "smart_delay.h"

#ifdef _WIN32
    #include <Windows.h>
    #define sleep(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define sleep(ms) usleep((ms) * 1000)
#endif

uint32_t diff(smart_delay_t *sd, uint32_t now) {
    return sd ? now - sd->start_time : -1;
}

void test_print(int id, smart_delay_t *sd, uint32_t t, uint32_t now) {
    uint32_t d = diff(sd, now);
    printf("test[%d] elapsed step %d (now %ld startted %ld diff %ld error %ld)\n", id, sd->step, now, sd->start_time, d, d - t);
}

void simple_test(int id, smart_delay_t *sd, uint32_t delay, uint32_t now) {
    if (smart_delay_is_elapsed(id, sd, delay, now))
        test_print(id, sd, delay, now);
}

void defined_steps_test(int id, smart_delay_t *sd, uint32_t delay, uint32_t now) {
    smart_delay_status_t status;
    
    delay = 2000;
    status = smart_stepped_delay(id, sd, delay, now, 0);
    if (status == SMART_DELAY_ELAPSED)
        test_print(id, sd, delay, now);
    else if (status == SMART_DELAY_WAITING)
        return;
    
    delay = 1000;
    status = smart_stepped_delay(id, sd, delay, now, 1);
    if (status == SMART_DELAY_ELAPSED)
        test_print(id, sd, delay, now);
    else if (status == SMART_DELAY_WAITING)
        return;
    
    delay = 3000;
    status = smart_stepped_delay(id, sd, delay, now, 2);
    if (status == SMART_DELAY_ELAPSED)
        test_print(id, sd, delay, now);
    else if (status == SMART_DELAY_WAITING)
        return;
    
    delay = 1000;
    status = smart_stepped_delay(id, sd, delay, now, 3);
    if (status == SMART_DELAY_ELAPSED)
        test_print(id, sd, delay, now);
    else if (status == SMART_DELAY_WAITING)
        return;
}

void sequencial_steps_all_test(int id, smart_delay_t *sd, uint32_t delay, uint32_t now) {
    int step = smart_delay_get_current_step(sd);
    if (step < 0)
        return;
    
    for (int s = 0; s < 10; ++s) {
        delay = s % 2 == 0 ? 1000 : 2000;
        smart_delay_status_t status = smart_stepped_delay(id, sd, delay, now, s);
        if (status == SMART_DELAY_ELAPSED)
            test_print(id, sd, delay, now);
    }
}

void sequencial_steps_unique_test(int id, smart_delay_t *sd, uint32_t delay, uint32_t now) {
    static int step = 0;
    
    delay = step % 2 == 0 ? 1000 : 2000;
    smart_delay_status_t status = smart_stepped_delay(id, sd, delay, now, step);
    if (status != SMART_DELAY_ELAPSED)
        return;
    
    test_print(id, sd, delay, now);
    step = (step + 1) % 10;
}

struct test {
    smart_delay_t sd;
    uint32_t delay;
    void (*fnc)(int id, smart_delay_t *sd, uint32_t delay, uint32_t now);
};

int main(void) {
    struct test tests[] = {
        { .sd = {0}, .delay = 1000, .fnc = simple_test },
        { .sd = {0}, .delay = 2000, .fnc = simple_test },
        { .sd = {0}, .fnc = defined_steps_test },
        { .sd = {0}, .fnc = sequencial_steps_all_test },
        { .sd = {0}, .fnc = sequencial_steps_unique_test },
    };
    int max = sizeof(tests) / sizeof(tests[0]);
    
    
    printf("start %d tests\n", max);
    
    while (1) {
        uint32_t now = clock();
        
        for (int i = 0; i < max; ++i) {
            tests[i].fnc(i, &tests[i].sd, tests[i].delay, now);
        }
        
        sleep(100);
    }
    
    return 0;
}
