#include "contiki.h"

#include <stdio.h>


static struct ctimer ct;

static void send(void *p)
{
    printf("test\n");
    ctimer_reset(&ct);
}

PROCESS(test_process, "Test Process");
AUTOSTART_PROCESSES(&test_process);
PROCESS_THREAD(test_process, ev, data)
{
    PROCESS_BEGIN();

    ctimer_set(&ct, CLOCK_SECOND, send, NULL);

    while (1) {
        PROCESS_WAIT_EVENT();
        energest_flush();
        long cpu = energest_type_time(ENERGEST_TYPE_CPU);
        long transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
        long lpm = energest_type_time(ENERGEST_TYPE_LPM);
        long listen = energest_type_time(ENERGEST_TYPE_LISTEN);
        printf("%ld %ld %ld %ld\n", cpu, transmit, lpm, listen);
    }

    PROCESS_END();
}