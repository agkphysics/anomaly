/*
 * simBanana.js - Simulation script that automates setting up and running the
 * simulation with data files added to each filesystem.
 */

TIMEOUT(1200000, finish());

motes = mote.getSimulation().getMotes();
for (i = 0; i < motes.length; i++) {
    mote = motes[i];
    id = mote.getID();

    fs = mote.getInterfaces().get("Filesystem");
    log.log(id.toString() + " ");
    if (fs.insertFile("/home/aaron/src/anomaly/data/Banana_" + id.toString()))
        log.log("Succeeded\n");
    else
        log.log("Failed\n");
}

finish = function() {
    for (i = 0; i < motes.length; i++) {
        mote = motes[i];
        id = mote.getID();
        fs = mote.getInterfaces().get("Filesystem");
        fs.extractFile("log", "/home/aaron/src/anomaly/logs/Banana/" + id.toString());
        log.log("Extracted " + id.toString() + "\n");
    }
}
