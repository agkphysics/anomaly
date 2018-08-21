/*
 * simHIWS.js - Simulation script that automates setting up and running the
 * simulation with data files added to the filesystem.
 */

TIMEOUT(1200000, finish());

fs = mote.getInterfaces().get("Filesystem");
if (fs.insertFile("/home/aaron/src/anomaly/data/HIWS"))
    log.log("Succeeded\n");
else
    log.log("Failed\n");

finish = function() {
    fs.extractFile("log", "/home/aaron/src/anomaly/logs/HIWS/1");
    log.log("Extracted\n");
}
