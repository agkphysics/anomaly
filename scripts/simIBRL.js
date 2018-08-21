/*
 * simIBRL.js - Simulation script that automates setting up and running the
 * simulation with data files added to each filesystem.
 */

TIMEOUT(1200000, finish());
coords = [[21.5, 23.0], [24.5, 20.0], [19.5, 19.0], [22.5, 15.0], [24.5, 12.0], [19.5, 12.0], [22.5, 8.0], [24.5, 4.0], [21.5, 2.0], [19.5, 5.0], [16.5, 3.0], [13.5, 1.0], [12.5, 5.0], [8.5, 6.0], [5.5, 3.0], [1.5, 2.0], [1.5, 8.0], [5.5, 10.0], [3.5, 13.0], [0.5, 17.0], [4.5, 18.0], [1.5, 23.0], [6.0, 24.0], [1.5, 30.0], [4.5, 30.0], [7.5, 31.0], [8.5, 26.0], [10.5, 31.0], [12.5, 26.0], [13.5, 31.0], [15.5, 28.0], [17.5, 31.0], [19.5, 26.0], [21.5, 30.0], [24.5, 27.0], [26.5, 31.0], [27.5, 26.0], [30.5, 31.0], [30.5, 26.0], [33.5, 28.0], [36.5, 30.0], [39.5, 30.0], [35.5, 24.0], [40.5, 22.0], [37.5, 19.0], [34.5, 16.0], [39.5, 14.0], [35.5, 10.0], [39.5, 6.0], [38.5, 1.0], [35.5, 4.0], [31.5, 6.0], [28.5, 5.0], [26.5, 2.0]]

motes = mote.getSimulation().getMotes();
for (i = 0; i < motes.length; i++) {
    mote = motes[i];
    id = mote.getID();
    mote.getInterfaces().getPosition().setCoordinates(coords[id-1][0], coords[id-1][1], 0);
    log.log("Repositioned " + id.toString() + "\n");

    if (id != 28 && id != 5) {
        fs = mote.getInterfaces().get("Filesystem");
        log.log(id.toString() + " ");
        if (fs.insertFile("/home/aaron/src/anomaly/data/IBRL_" + id.toString())) {
            fs.updateFS(); // Requires the method to be declared public
            log.log("Succeeded\n");
        } else {
            log.log("Failed\n");
        }
    }
}

finish = function() {
    for (i = 0; i < motes.length; i++) {
        mote = motes[i];
        id = mote.getID();
        fs = mote.getInterfaces().get("Filesystem");
        fs.extractFile("log", "/home/aaron/src/anomaly/logs/IBRL/" + id.toString());
        log.log("Extracted " + id.toString());
    }
}
