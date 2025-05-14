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

#define dbg(fmt, ...) printf("%10lu " fmt, (uint32_t)clock(), ##__VA_ARGS__)

int diff(smart_delay_t *sd, uint32_t now) {
    return sd ? now - sd->start_time : -1;
}

void test_print(int id, smart_delay_t *sd, uint32_t t, uint32_t now) {
    int d = diff(sd, now);
    int e = d - t;
    dbg("test[%d] elapsed step %d (now %ld started %ld diff %d error %ld)\n", id, sd->step, now, sd->start_time, d, e);
}

void test_exec(int id, int step) {
    dbg("test[%d] execute step %d\n", id, step);
}

void test_end(int id) {
    dbg("test[%d] finish\n", id);
}

void simple_test(int id, smart_delay_t *sd, uint32_t delay, uint32_t now) {
    if (smart_delay_is_elapsed(id, sd, delay, now))
        test_print(id, sd, delay, now);
}

void defined_steps_test(int id, smart_delay_t *sd, uint32_t delay, uint32_t now) {
    static int step = -1;
    smart_delay_status_t status;
    
    if (step == -1)
        test_exec(id, ++step);
    delay = 2000;
    status = smart_stepped_delay(id, sd, delay, now, 0);
    if (status == SMART_DELAY_ELAPSED)
        test_print(id, sd, delay, now);
    else if (status == SMART_DELAY_WAITING)
        return;
    
    if (step == 0)
        test_exec(id, ++step);
    delay = 1000;
    status = smart_stepped_delay(id, sd, delay, now, 1);
    if (status == SMART_DELAY_ELAPSED)
        test_print(id, sd, delay, now);
    else if (status == SMART_DELAY_WAITING)
        return;
    
    if (step == 1)
        test_exec(id, ++step);
    delay = 3000;
    status = smart_stepped_delay(id, sd, delay, now, 2);
    if (status == SMART_DELAY_ELAPSED)
        test_print(id, sd, delay, now);
    else if (status == SMART_DELAY_WAITING)
        return;
    
    if (step == 2)
        test_exec(id, ++step);
    delay = 1000;
    status = smart_stepped_delay(id, sd, delay, now, 3);
    if (status == SMART_DELAY_ELAPSED)
        test_print(id, sd, delay, now);
    else if (status == SMART_DELAY_WAITING)
        return;
    
    step = -1;
    test_end(id);
}

void sequencial_steps_all_test(int id, smart_delay_t *sd, uint32_t delay, uint32_t now) {
    static int step = 0;
    static bool executed = false;
    
    for (int s = 0; s < 10; ++s) {
        if (s == step && !executed) {
            executed = true;
            test_exec(id, s);
        }
        delay = s % 2 == 0 ? 1000 : 2000;
        smart_delay_status_t status = smart_stepped_delay(id, sd, delay, now, s);
        if (status == SMART_DELAY_ELAPSED) {
            test_print(id, sd, delay, now);
            executed = false;
            step = (step + 1) % 10;
            if (step == 0)
                test_end(id);
        }
    }
}

void sequencial_steps_unique_test(int id, smart_delay_t *sd, uint32_t delay, uint32_t now) {
    static int step = 0;
    static int next = 0;
    
    if (next == step) {
        test_exec(id, step);
        next = (next + 1) % 10;
    }
    
    delay = step % 2 == 0 ? 1000 : 2000;
    smart_delay_status_t status = smart_stepped_delay(id, sd, delay, now, step);
    if (status != SMART_DELAY_ELAPSED)
        return;
    
    test_print(id, sd, delay, now);
    step = (step + 1) % 10;
    
    if (step == 0)
        test_end(id);
}

void overflow_test(int id, smart_delay_t *sd, uint32_t delay, uint32_t now) {
    static uint32_t last_now = 0;
    static uint32_t fake_now = 0;
    static bool init = true;
    
    if (init) {
        init = false;
        fake_now = 0xFFFFFFFF - ((delay / 2) + now);
    } else {
        fake_now += (uint32_t)(now - last_now);
    }
    
    last_now = now;
    
    if (smart_delay_is_elapsed(id, sd, delay, fake_now)) {
        test_print(id, sd, delay, fake_now);
        init = true;
    }
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
        { .sd = {0}, .delay = 6000, .fnc = overflow_test },
    };
    int max = sizeof(tests) / sizeof(tests[0]);
    
    dbg("start %d tests\n", max);
    
    while (1) {
        uint32_t now = clock();
        
        for (int i = 0; i < max; ++i) {
            tests[i].fnc(i, &tests[i].sd, tests[i].delay, now);
        }
        
        sleep(100);
    }
    
    return 0;
}
