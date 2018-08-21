/*
 * simStBernard.js - Simulation script that automates setting up and running the
 * simulation with data files added to each filesystem.
 */

TIMEOUT(1200000, finish());
coords = [[66.2, 51.6, 16.6], [-48.8, -70.4, -24.4], [-6.8, -128.4, 7.6], [6.2, 8.6, -5.4], [-16.8, 138.6, 5.6]];

motes = mote.getSimulation().getMotes();
for (i = 0; i < motes.length; i++) {
    mote = motes[i];
    id = mote.getID();
    mote.getInterfaces().getPosition().setCoordinates(coords[id-1][0], coords[id-1][1], 0);
    log.log("Repositioned " + id.toString() + "\n");

    fs = mote.getInterfaces().get("Filesystem");
    log.log(id.toString() + " ");
    if (fs.insertFile("/home/aaron/src/anomaly/data/StBernard_" + id.toString()))
        log.log("Succeeded\n");
    else
        log.log("Failed\n");
}

finish = function() {
    for (i = 0; i < motes.length; i++) {
        mote = motes[i];
        id = mote.getID();
        fs = mote.getInterfaces().get("Filesystem");
        fs.extractFile("log", "/home/aaron/src/anomaly/logs/StBernard/" + id.toString());
        log.log("Extracted " + id.toString() + "\n");
    }
}
