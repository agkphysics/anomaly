/*
 * simHITEMP.js - Simulation script that automates setting up and running the
 * simulation with data files added to each filesystem.
 */

TIMEOUT(1200000, finish());
coords = [[764.7, -1376.1], [-313.1, -1692.1], [364.7, 336.4], [-846.4, -203.9], [242.4, 2150.8], [-1402.0, 1467.8], [364.7, -3251.7], [253.6, -1610.6], [-290.9, 30.6], [620.2, 1997.9]]

motes = mote.getSimulation().getMotes();
for (i = 0; i < motes.length; i++) {
    mote = motes[i];
    id = mote.getID();
    mote.getInterfaces().getPosition().setCoordinates(coords[id-1][0], coords[id-1][1], 0);
    log.log("Repositioned " + id.toString() + "\n");

    fs = mote.getInterfaces().get("Filesystem");
    log.log(id.toString() + " ");
    if (fs.insertFile("/home/aaron/src/anomaly/data/HITEMP_" + id.toString()))
        log.log("Succeeded\n");
    else
        log.log("Failed\n");
}

finish = function() {
    for (i = 0; i < motes.length; i++) {
        mote = motes[i];
        id = mote.getID();
        fs = mote.getInterfaces().get("Filesystem");
        fs.extractFile("log", "/home/aaron/src/anomaly/logs/HITEMP/" + id.toString());
        log.log("Extracted " + id.toString() + "\n");
    }
}
