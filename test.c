#include "contiki.h"

#include <stdio.h>

PROCESS(test_process, "Test Process");
AUTOSTART_PROCESSES(&test_process);
PROCESS_THREAD(test_process, ev, data)
{
    PROCESS_BEGIN();

    static struct etimer et;
    etimer_set(&et, CLOCK_SECOND);

    while (1) {
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
        etimer_reset(&et);
        energest_flush();
        long cpu = energest_type_time(ENERGEST_TYPE_CPU);
        long transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
        long lpm = energest_type_time(ENERGEST_TYPE_LPM);
        long listen = energest_type_time(ENERGEST_TYPE_LISTEN);
        printf("%ld %ld %ld %ld\n", cpu, transmit, lpm, listen);
    }

    PROCESS_END();
}