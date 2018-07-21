/*
 * anomaly-test.c - Main source file for anomaly detection
 * 
 * Author: Aaron Keesing
 * Date: 02/06/2018
 */

#include "contiki.h"
#include "net/rime/mesh.h"
#include "cfs/cfs.h"

#include "hitemp.h"
#include "common.h"
#include "mat.h"

PROCESS(anomaly_process, "Anomaly Detection Process");
AUTOSTART_PROCESSES(&anomaly_process);

#define NUM_SENSORS 10

static Mat korig, kcent;
static Vec d;
static sensorval readings[NUM_READINGS];
static real lradius;
static real rList[NUM_SENSORS];
static real gradius;
static int children;
static unsigned char isChild;

static const process_event_t EVENT_RAD = 1;
static const process_event_t EVENT_COMM = 2;
static struct etimer et;

#ifdef COMMUNICATION
/* Communication structures */
static linkaddr_t addr;
static struct mesh_conn mc;

/* Communication functions */
static void recvChild(struct mesh_conn *c, const linkaddr_t *from, uint8_t hops)
{
    real radius = *(real *)packetbuf_dataptr();
    printf("Got radius from parent %d, hops %d: '%ld'\n",
        from->u8[0], hops, (long)(lradius*1e6));
    gradius = radius;
}

static void recvParent(struct mesh_conn *c, const linkaddr_t *from, uint8_t hops)
{
    real radius = *(real *)packetbuf_dataptr();
    printf("Got radius from child %d, hops %d: '%ld'\n",
        from->u8[0], hops, (long)(lradius*1e6));
    rList[from->u8[0]] = radius;
    children++;
    if (children == NUM_SENSORS)
        process_post(&anomaly_process, EVENT_RAD, NULL);
}

static void sent(struct mesh_conn *c) {}

static void timedout(struct mesh_conn *c)
{
    if (!process_is_running(&anomaly_process))
        return;
    printf("Timedout %d\n", linkaddr_node_addr.u8[0]);
    packetbuf_clear();
    packetbuf_copyfrom(&lradius, sizeof(real));
    packetbuf_set_datalen(sizeof(real));
    mesh_send(&mc, &addr);
}

static struct mesh_callbacks callbacks = {recvChild, sent, timedout};
#endif

/* Main process thread */
PROCESS_THREAD(anomaly_process, ev, data)
{
#ifdef COMMUNICATION
    PROCESS_EXITHANDLER(mesh_close(&mc));
#endif
    PROCESS_BEGIN();

    if (!init())
        PROCESS_EXIT();

#ifdef COMMUNICATION
    if (linkaddr_node_addr.u8[0] == ROOT_NODE) {
        isChild = 1;
        callbacks.recv = recvParent;
    }
    addr.u8[0] = ROOT_NODE;
    addr.u8[1] = 0;
    mesh_open(&mc, 132, &callbacks);
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
    etimer_set(&et, CLOCK_SECOND*5);
    while (1) {
        PROCESS_WAIT_EVENT();
        if (ev == PROCESS_EVENT_TIMER) {
            for (int i = 0; i < NUM_SENSORS; i++)
                rList[i] = 0;
            children = 0;
            etimer_reset(&et);
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
                    korig.arr[idx(i, j, NUM_READINGS)] = korig.arr[idx(j, i, NUM_READINGS)] = rbf(vals1, vals2, VAL_LEN);
                }
            }
            centMat(&korig, &kcent);

            /* Retain only the centred d(xi, xi) values, and sort */
            for (int i = 0; i < NUM_READINGS; i++)
                d.arr[i] = kcent.arr[idx(i, i, NUM_READINGS)];
            sort(d.arr, NUM_READINGS);

        #if defined(NATIVE) && DEBUG
            printf("d: ");
            for (int i = NUM_READINGS - 1; i >= 0; i--)
                printf("%f ", d.arr[i]);
            printf("\n");
        #endif

            int j = (int)floorf(0.1*NUM_READINGS);
            lradius = d.arr[NUM_READINGS - 1 - j];
            char logbuf[50] = {0};
            int length = 0;
            length += sprintf(logbuf + length, "L ");
            for (int i = 0; i < NUM_READINGS; i++)
                if (kcent.arr[idx(i, i, NUM_READINGS)] > lradius) {
                    length += sprintf(logbuf + length, "%d ", readings[i].epoch);
                }
            length += sprintf(logbuf + length, ": ");

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

#ifdef COMMUNICATION
            packetbuf_clear();
            packetbuf_copyfrom(&lradius, sizeof(real));
            packetbuf_set_datalen(sizeof(real));
            mesh_send(&mc, &addr);
#endif

            cfs_write(logfile, logbuf, strlen(logbuf));
            printf(logbuf);
        }
        else if (ev == EVENT_RAD) {
            if (!isChild) {
#ifdef COMMUNICATION
                packetbuf_clear();
                packetbuf_copyfrom(&gradius, sizeof(real));
                packetbuf_set_datalen(sizeof(real));
                linkaddr_t a;
                for (int i = 1; i <= NUM_SENSORS; i++) {
                    if (i == ROOT_NODE)
                        continue;
                    a.u8[0] = i;
                    a.u8[1] = 0;
                    mesh_send(&mc, &a);
                }
#endif
            }
            else {
                char logbuf[50] = {0};
                int length = 0;
                length += sprintf(logbuf + length, "G ");
                for (int i = 0; i < NUM_READINGS; i++)
                    if (kcent.arr[idx(i, i, NUM_READINGS)] > gradius) {
                        length += sprintf(logbuf + length, "%d ", readings[i].epoch);
                    }
                length += sprintf(logbuf + length, "\n");
            }
        }
    }

    printf("Finished\n");

    PROCESS_END();
}
