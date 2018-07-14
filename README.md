# Anomaly Detection

This repository contains the code for running/simulating anomaly detection
algorithms on low-power wireless sensor networks (WSNs).
[Contiki OS](http://www.contiki-os.org/) must be installed an placed as a
sibling directory of the `anomaly` repo. The MSP430 GCC toolkit must also be
installed.

## Running

The Cooja simulator can be started from the `contiki/tools/cooja` directory with
the commands `ant run` or `ant run_bigmem`. Then one of the `.csc` files from
this repo can be loaded. Each node must have the data corresponding to its
node/RIME ID. The simIBRL.js script can be used for loading the IBRL data onto
the nodes. The HIWS simulation can easily be done manually with the GUI, and
similarly for the HITEMP data.
