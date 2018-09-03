/*
 * anomaly-test.c - Main source file for anomaly detection
 *
 * Author: Aaron Keesing
 */

#include "contiki.h"
#include "contiki-conf.h"
#include "net/rime/runicast.h"
//#include "net/mac/tsch/tsch.h"
#include "dev/watchdog.h"
#include "cfs/cfs.h"

#include "dataset.h"
#include "common.h"
#include "mat.h"

PROCESS(anomaly_process, "Anomaly Detection Process");
AUTOSTART_PROCESSES(&anomaly_process);

#define RXMITS 20

static Mat k;
static Vec d;
static sensorval readings[NUM_READINGS];
static real lradius, gradius;
static real rList[NUM_SENSORS];
static int children;
static unsigned char isChild, isValid;

static const process_event_t EVENT_RAD = 1;
static struct etimer periodTimer;

#ifdef COMMUNICATION
/* Communication structures */
static linkaddr_t rootAddr;
static struct runicast_conn ruc;

/* Communication callbacks */
static void recv_ruc(struct runicast_conn *ruc, const linkaddr_t *from, uint8_t seqno)
{
    if (isChild && isValid) {
        real radius = *(real *)packetbuf_dataptr();
        // printf("Got gradius from parent: %ld\n", (long)(radius*1e6));
        gradius = radius;
        process_post(&anomaly_process, EVENT_RAD, NULL);
        isValid = 0;
    }
    else if (!isChild) {
        int id = from->u8[0] - 1;
        if (children & (1 << id))
            return;
        real radius = *(real *)packetbuf_dataptr();
        printf("Got radius from child %d: %ld\n", id + 1, (long)(radius*1e6));
        rList[id] = radius;
        children |= 1 << id;
        if (children == ((1 << NUM_SENSORS) - 1)) {
            if (process_post(&anomaly_process, EVENT_RAD, NULL) != PROCESS_ERR_OK)
                printf("Error posting EVENT_RAD\n");
        }
    }
}

static void sent_ruc(struct runicast_conn *ruc, const linkaddr_t *to, uint8_t rxmits)
{
    if (!isChild && children == ((1 << NUM_SENSORS) - 1) && to->u8[0] < NUM_SENSORS) {
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
    //if (!isChild)
        //tsch_set_coordinator(1);
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
    static int bytes;
    static int epoch = 0; // For debug purposes
#ifdef NATIVE
    etimer_set(&periodTimer, CLOCK_SECOND/2);
#else
    etimer_set(&periodTimer, CLOCK_SECOND*20);
#endif
    while (1) {
        PROCESS_WAIT_EVENT();
        if (ev == PROCESS_EVENT_TIMER) {
            for (int i = 0; i < NUM_SENSORS; i++)
                rList[i] = 0;
            children = 0;
            isValid = 1;
            etimer_reset(&periodTimer);
            watchdog_stop();
            for (int i = 0; i < NUM_READINGS && (bytes = cfs_read(fp, buf + length, BUF_LEN - length)) > 0; epoch++, i++) {
                char *p = getNextReading(buf, &readings[i]);
                //printReading(&readings[i]);
                p += strlen(p) + 1; // p now points to beginning of next line
                length = buf + BUF_LEN - p;
                memmove(buf, p, length); // memcpy() has undefined behaviour in this case
            }
            if (bytes <= 0)
                break;

            for (int i = 0; i < NUM_READINGS; i++) {
                float vals1[VAL_LEN];
                getVector(&readings[i], vals1);
                for(int j = i; j < NUM_READINGS; j++) {
                    float vals2[VAL_LEN];
                    getVector(&readings[j], vals2);
                    k.arr[idx(i, j, NUM_READINGS)] = k.arr[idx(j, i, NUM_READINGS)] = rbf(vals1, vals2, VAL_LEN);
                }
            }
            centMat(&k, NUM_READINGS);

            /* Retain only the centred d(xi, xi) values, and sort */
            for (int i = 0; i < NUM_READINGS; i++)
                d.arr[i] = k.arr[idx(i, i, NUM_READINGS)];
            sort(d.arr, NUM_READINGS);

#ifdef NATIVE
            printf("d: ");
            for (int i = NUM_READINGS - 1; i >= 0; i--)
                printf("%f ", d.arr[i]);
            printf("\n");
#endif

            int j = (int)floorf(NU*NUM_READINGS);
            lradius = d.arr[NUM_READINGS - 1 - j];
            if (linkaddr_node_addr.u8[0] == ROOT_NODE) {
                rList[ROOT_NODE-1] = lradius;
                children |= 1 << (ROOT_NODE - 1);
            }
            char logbuf[120] = {0};
            int length = 0;
            length += sprintf(logbuf + length, "L ");
            for (int i = 0; i < NUM_READINGS; i++)
                if (k.arr[idx(i, i, NUM_READINGS)] > lradius) {
                    length += sprintf(logbuf + length, "%d ", readings[i].epoch);
                }
            length += sprintf(logbuf + length, "\n");

            static long lastCPU, lastTransmit, lastLPM, lastListen, lastClockTime;
            lastCPU = lastTransmit = lastLPM = lastListen = lastClockTime = 0;
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

            length += sprintf(logbuf + length, "%ld %ld %ld %ld %ld\n", clockTime, cpu, lpm, transmit, listen);

            cfs_write(logfile, logbuf, strlen(logbuf));
            printf(logbuf);
#ifdef COMMUNICATION
            if (linkaddr_node_addr.u8[0] != ROOT_NODE) {
                packetbuf_clear();
                packetbuf_copyfrom(&lradius, sizeof(real));
                packetbuf_set_datalen(sizeof(real));
                ctimer_set(&ctimer, CLOCK_SECOND*linkaddr_node_addr.u8[0]/2, send, NULL);
                //runicast_send(&ruc, &rootAddr, RXMITS);
            }
#endif
        }
#ifdef COMMUNICATION
        else if (ev == EVENT_RAD) {
            if (!isChild) {
                // real r = 0;
                // for (int i = 0; i < NUM_SENSORS; i++)
                //     r += rList[i];
                // gradius = r / (real)NUM_SENSORS;

                // Aggregate radii via median
                sort(rList, NUM_SENSORS);
                gradius = rList[NUM_SENSORS / 2];
                printf("gradius: %ld\n", (long)(gradius*1e6));
                packetbuf_clear();
                packetbuf_copyfrom(&gradius, sizeof(real));
                packetbuf_set_datalen(sizeof(real));
                linkaddr_t a;
                a.u8[0] = 2;
                a.u8[1] = 0;
                runicast_send(&ruc, &a, RXMITS);
            }
            char logbuf[160] = {0};
            int length = 0;
            length += sprintf(logbuf + length, "G ");
            for (int i = 0; i < NUM_READINGS; i++)
                if (k.arr[idx(i, i, NUM_READINGS)] > gradius) {
                    length += sprintf(logbuf + length, "%d ", readings[i].epoch);
                }
            length += sprintf(logbuf + length, "\n");
            cfs_write(logfile, logbuf, strlen(logbuf));
            printf(logbuf);
        }
#endif
    }

    printf("Finished\n");

    PROCESS_END();
}
