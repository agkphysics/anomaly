/*
 * anomaly-test.c - Main source file for anomaly detection
 * 
 * Author: Aaron Keesing
 * Date: 02/06/2018
 */

#include "contiki.h"
#include "net/rime/mesh.h"
#include "cfs/cfs.h"

#include "hiws.h"
#include "common.h"
#include "mat.h"

static Mat korig, kcent;
static Vec d;
static sensorval readings[NUM_READINGS];
static real lradius;
static real gradius;

static struct etimer et;

/* Communication structures */
static linkaddr_t addr;
static struct mesh_conn mc;

PROCESS(anomaly_process, "Anomaly Detection Process");
AUTOSTART_PROCESSES(&anomaly_process);

/* Communication functions */
static void recvChild(struct mesh_conn *c, const linkaddr_t *from, uint8_t hops)
{
    real lradius = *(real *)packetbuf_dataptr();
    printf("Got lradius from parent %d, hops %d: '%ld'\n",
        from->u8[0], hops, (long)(lradius*1e6));
}

static void recvParent(struct mesh_conn *c, const linkaddr_t *from, uint8_t hops)
{
    real lradius = *(real *)packetbuf_dataptr();
    printf("Got lradius from child %d, hops %d: '%ld'\n",
        from->u8[0], hops, (long)(lradius*1e6));
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

/* Main process thread */
PROCESS_THREAD(anomaly_process, ev, data)
{
    PROCESS_EXITHANDLER(mesh_close(&mc));
    PROCESS_BEGIN();

    if (!init())
        PROCESS_EXIT();
    if (linkaddr_node_addr.u8[0] == 1)
        callbacks.recv = recvParent;

    addr.u8[0] = 1;
    addr.u8[1] = 0;

    energest_init();
    mesh_open(&mc, 132, &callbacks);

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
        char logbuf[40] = {0};
        int length = 0;
        for (int i = 0; i < NUM_READINGS; i++)
            if (kcent.arr[idx(i, i, NUM_READINGS)] > lradius) {
                length += sprintf(logbuf + length, "%d ", readings[i].epoch);
            }
        length += sprintf(logbuf + length, ": ");

        static long lastCPU, lastTransmit, lastLPM, lastListen;
        long cpu, transmit, lpm, listen;

        energest_flush();
        cpu = energest_type_time(ENERGEST_TYPE_CPU) - lastCPU;
        lpm = energest_type_time(ENERGEST_TYPE_LPM) - lastLPM;
        transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT) - lastTransmit;
        listen = energest_type_time(ENERGEST_TYPE_LISTEN) - lastListen;

        lastCPU = energest_type_time(ENERGEST_TYPE_CPU);
        lastLPM = energest_type_time(ENERGEST_TYPE_LPM);
        lastTransmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
        lastListen = energest_type_time(ENERGEST_TYPE_LISTEN);

        length += sprintf(logbuf + length, "%ld %ld %ld %ld\n", cpu, lpm, transmit, listen);

        packetbuf_clear();
        packetbuf_copyfrom(&lradius, sizeof(real));
        packetbuf_set_datalen(sizeof(real));
        mesh_send(&mc, &addr);

        cfs_write(logfile, logbuf, strlen(logbuf));
        printf(logbuf);
    }

    printf("Finished\n");

    PROCESS_END();
}
