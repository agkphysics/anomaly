/*
 * anomaly-periodic.c - Main source file for anomaly detection
 *
 * Author: Aaron Keesing
 */

#include "contiki.h"
#include "contiki-conf.h"
#include "net/rime/runicast.h"
#include "dev/watchdog.h"
#include "cfs/cfs.h"

#include "dataset.h"
#include "common.h"
#include "mat.h"

PROCESS(anomaly_process, "Anomaly Detection Process");
AUTOSTART_PROCESSES(&anomaly_process);

#define RXMITS 20
#define l(x) (long)(1e6*x)

static Mat k;
static Vec d;
static sensorval readings[NUM_READINGS];
static real lradius, gradius;
static real rList[NUM_SENSORS];
static real rListSorted[NUM_SENSORS];
static int children;
static long lastCPU, lastTransmit, lastLPM, lastListen, lastClockTime;
static int period;

static struct etimer periodTimer;

#ifdef COMMUNICATION
static unsigned char isChild;

/* Communication structures */
static linkaddr_t rootAddr;
static struct runicast_conn ruc;

/* Communication callbacks */
static void recv_ruc(struct runicast_conn *ruc, const linkaddr_t *from, uint8_t seqno)
{
    if (isChild) {
        real radius = *(real *)packetbuf_dataptr();
        gradius = radius;
    }
    else {
        int id = from->u8[0] - 1;
        real radius = *(real *)packetbuf_dataptr();
        printf("Got radius from child %d: %ld\n", id + 1, (long)(radius*1e6));
        rList[id] = radius;
        if (period == 0)
            children |= 1 << id;
        if (period > 0 || children == ((1 << NUM_SENSORS) - 1)) {
            // Aggregate radii via median
            for (int i = 0; i < NUM_SENSORS; i++)
                rListSorted[i] = rList[i];
            sort(rListSorted, NUM_SENSORS);
            if (NUM_SENSORS % 2 != 0)
                gradius = rListSorted[NUM_SENSORS/2];
            else
                gradius = (rListSorted[NUM_SENSORS/2] + rListSorted[NUM_SENSORS/2-1])/2;
            printf("gradius: %ld\n", (long)(gradius*1e6));
            packetbuf_clear();
            packetbuf_copyfrom(&gradius, sizeof(real));
            packetbuf_set_datalen(sizeof(real));
            linkaddr_t a;
            a.u8[0] = 2;
            a.u8[1] = 0;
            runicast_send(ruc, &a, RXMITS);
        }
    }
}

static void sent_ruc(struct runicast_conn *ruc, const linkaddr_t *to, uint8_t rxmits)
{
    if (!isChild && to->u8[0] < NUM_SENSORS) {
        linkaddr_t a;
        a.u8[0] = to->u8[0] + 1;
        a.u8[1] = 0;
        packetbuf_clear();
        packetbuf_copyfrom(&gradius, sizeof(real));
        packetbuf_set_datalen(sizeof(real));
        runicast_send(ruc, &a, RXMITS);
    }
}

static void timeout_ruc(struct runicast_conn *ruc, const linkaddr_t *to, uint8_t rtxmits)
{
    printf("Timedout %d to %d\n", linkaddr_node_addr.u8[0], to->u8[0]);
}

static void send(void *p)
{
    packetbuf_clear();
    packetbuf_copyfrom(&lradius, sizeof(real));
    packetbuf_set_datalen(sizeof(real));
    runicast_send(&ruc, &rootAddr, RXMITS);
}

static struct ctimer ctimer;

static struct runicast_callbacks callbacks_ruc = {recv_ruc, sent_ruc, timeout_ruc};
#endif

/* Main process thread */
PROCESS_THREAD(anomaly_process, ev, data)
{
#ifdef COMMUNICATION
    PROCESS_EXITHANDLER(runicast_close(&ruc););
#endif
    PROCESS_BEGIN();

    if (!init())
        PROCESS_EXIT();

#ifdef COMMUNICATION
    isChild = !(linkaddr_node_addr.u8[0] == ROOT_NODE);
    rootAddr.u8[0] = ROOT_NODE;
    rootAddr.u8[1] = 0;
    runicast_open(&ruc, 132, &callbacks_ruc);
#endif
    energest_init();

    static char name[32] = {0};
    getName(name);

    printf("Reading from %s\n", name);
    static int fp;
    fp = cfs_open(name, CFS_READ);
    if (fp == -1) {
        printf("Error: could not open data.\n");
        PROCESS_EXIT();
    }
    static int logfile;
    logfile = cfs_open("log", CFS_WRITE);
    if (logfile == -1) {
        printf("Error: could not open log.\n");
        PROCESS_EXIT();
    }

    static char buf[BUF_LEN + 2] = {0};
    static size_t length = 0;
    static int epoch = 0; // For debug purposes
    static int r;
    period = 0;
    lastCPU = lastTransmit = lastLPM = lastListen = lastClockTime = 0;

    /* Initial data */
    for (r = 0; r < NUM_READINGS; epoch++, r++) {
        cfs_read(fp, buf + length, BUF_LEN - length);
        char *p = getNextReading(buf, &readings[r]);
        p += strlen(p) + 1;
        length = buf + BUF_LEN - p;
        memmove(buf, p, length);
    }

    for (int i = 0; i < NUM_READINGS; i++) {
        real vals1[VAL_LEN];
        getVector(&readings[i], vals1);
        for(int j = i; j < NUM_READINGS; j++) {
            real vals2[VAL_LEN];
            getVector(&readings[j], vals2);
            k.arr[idx(i, j, NUM_READINGS)] = k.arr[idx(j, i, NUM_READINGS)] = rbf(vals1, vals2, VAL_LEN);
        }
    }
    centMat(&k, NUM_READINGS);

    /* Retain only the centred d(xi, xi) values, and sort */
    for (int i = 0; i < NUM_READINGS; i++)
        d.arr[i] = k.arr[idx(i, i, NUM_READINGS)];
    sort(d.arr, NUM_READINGS);

    int j = (int)floorf(NU*NUM_READINGS);
    lradius = d.arr[NUM_READINGS - 1 - j];
    if (linkaddr_node_addr.u8[0] == ROOT_NODE) {
        rList[ROOT_NODE-1] = lradius;
        children |= 1 << (ROOT_NODE - 1);
    }

    long cpu, transmit, lpm, listen, clockTime;

    energest_flush();
    clockTime = clock_time() - lastClockTime;
    cpu = energest_type_time(ENERGEST_TYPE_CPU) - lastCPU;
    lpm = energest_type_time(ENERGEST_TYPE_LPM) - lastLPM;
    transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT) - lastTransmit;
    listen = energest_type_time(ENERGEST_TYPE_LISTEN) - lastListen;

    lastClockTime = clock_time();
    lastCPU = energest_type_time(ENERGEST_TYPE_CPU);
    lastLPM = energest_type_time(ENERGEST_TYPE_LPM);
    lastTransmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
    lastListen = energest_type_time(ENERGEST_TYPE_LISTEN);

    char logbuf[50] = {0};
    int loglength = 0;
    loglength += sprintf(logbuf + loglength, "%ld %ld %ld %ld %ld\n", clockTime, cpu, lpm, transmit, listen);

    cfs_write(logfile, logbuf, strlen(logbuf));
    printf(logbuf);

#ifdef NATIVE
    etimer_set(&periodTimer, CLOCK_SECOND/2);
#else
    etimer_set(&periodTimer, CLOCK_SECOND*5);
#endif

#ifdef COMMUNICATION
    if (linkaddr_node_addr.u8[0] != ROOT_NODE) {
        packetbuf_clear();
        packetbuf_copyfrom(&lradius, sizeof(real));
        packetbuf_set_datalen(sizeof(real));
        ctimer_set(&ctimer, CLOCK_SECOND*linkaddr_node_addr.u8[0]/4, send, NULL);
    }
#else
    gradius = lradius;
#endif

    while (1) {
        PROCESS_WAIT_EVENT();
        if (ev == PROCESS_EVENT_TIMER) {
            period++;
            etimer_reset(&periodTimer);
            watchdog_stop();

            char logbuf[50] = {0};
            int loglength = 0;

            if (period % 50 == 0) {
                long cpu, transmit, lpm, listen, clockTime;

                energest_flush();
                clockTime = clock_time() - lastClockTime;
                cpu = energest_type_time(ENERGEST_TYPE_CPU) - lastCPU;
                lpm = energest_type_time(ENERGEST_TYPE_LPM) - lastLPM;
                transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT) - lastTransmit;
                listen = energest_type_time(ENERGEST_TYPE_LISTEN) - lastListen;

                lastClockTime = clock_time();
                lastCPU = energest_type_time(ENERGEST_TYPE_CPU);
                lastLPM = energest_type_time(ENERGEST_TYPE_LPM);
                lastTransmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
                lastListen = energest_type_time(ENERGEST_TYPE_LISTEN);

                loglength += sprintf(logbuf + loglength, "%ld %ld %ld %ld %ld\n", clockTime, cpu, lpm, transmit, listen);
                cfs_write(logfile, logbuf, strlen(logbuf));
                printf(logbuf);
                loglength = 0;
            }

            if (cfs_read(fp, buf + length, BUF_LEN - length) <= 0)
                break;
            char *p = getNextReading(buf, &readings[r]);
            p += strlen(p) + 1;
            length = buf + BUF_LEN - p;
            memmove(buf, p, length);
            r = (r + 1) % NUM_READINGS;
            real vals[VAL_LEN];
            getVector(&readings[r], vals);
            //real kxx = rbf(vals, vals, VAL_LEN);
            real sum = 0;
            for (int i = 0; i < NUM_READINGS; i++) {
                real vals2[VAL_LEN];
                getVector(&readings[i], vals2);
                sum += 1.0/(real)NUM_READINGS - 2*rbf(vals, vals2, VAL_LEN);
            }
            sum /= (real)NUM_READINGS;
            real dc = sqrtf(1.0 + sum);
            //printf("lradius = %ld, gradius = %ld, dc = %ld\n", l(lradius), l(gradius), l(dc));
            if (dc > lradius && dc > gradius) { // Outlier
                loglength += sprintf(logbuf + loglength, "A %d\n", readings[r].epoch);
                cfs_write(logfile, logbuf, strlen(logbuf));
                printf(logbuf);
            }

            if (period % M != 0) { // Periodic updates only
                continue;
            }

            for (int i = 0; i < NUM_READINGS; i++) {
                real vals1[VAL_LEN];
                getVector(&readings[i], vals1);
                for(int j = i; j < NUM_READINGS; j++) {
                    real vals2[VAL_LEN];
                    getVector(&readings[j], vals2);
                    k.arr[idx(i, j, NUM_READINGS)] = k.arr[idx(j, i, NUM_READINGS)] = rbf(vals1, vals2, VAL_LEN);
                }
            }
            centMat(&k, NUM_READINGS);

            /* Retain only the centred d(xi, xi) values, and sort */
            for (int i = 0; i < NUM_READINGS; i++)
                d.arr[i] = k.arr[idx(i, i, NUM_READINGS)];
            sort(d.arr, NUM_READINGS);

            int j = (int)floorf(NU*NUM_READINGS);
            lradius = d.arr[NUM_READINGS - 1 - j];

#ifdef COMMUNICATION
            if (linkaddr_node_addr.u8[0] == ROOT_NODE) {
                rList[ROOT_NODE-1] = lradius;
                // Aggregate radii via median
                for (int i = 0; i < NUM_SENSORS; i++)
                    rListSorted[i] = rList[i];
                sort(rListSorted, NUM_SENSORS);
                if (NUM_SENSORS % 2 != 0)
                    gradius = rListSorted[NUM_SENSORS/2];
                else
                    gradius = (rListSorted[NUM_SENSORS/2] + rListSorted[NUM_SENSORS/2-1])/2;
                printf("gradius: %ld\n", (long)(gradius*1e6));
                packetbuf_clear();
                packetbuf_copyfrom(&gradius, sizeof(real));
                packetbuf_set_datalen(sizeof(real));
                linkaddr_t a;
                a.u8[0] = 2;
                a.u8[1] = 0;
                runicast_send(&ruc, &a, RXMITS);
            }
            else {
                packetbuf_clear();
                packetbuf_copyfrom(&lradius, sizeof(real));
                packetbuf_set_datalen(sizeof(real));
                ctimer_set(&ctimer, CLOCK_SECOND*linkaddr_node_addr.u8[0]/4, send, NULL);
            }
#else
            gradius = lradius;
#endif
        }
    }

    printf("Finished\n");

    PROCESS_END();
}
