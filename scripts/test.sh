#!/usr/bin/bash

TEST=$1
TEST_NO=$2
DATASETS=$3
CONTIKI_DIR=$HOME/src/contiki

if [ "$TEST" = "" ]
then
    echo "Must specify test."
    exit 1
fi
if [ "$TEST_NO" = "" ]
then
    echo "Must specify test number."
    exit 1
fi
if [ "$DATASETS" = "" ]
then
    echo "Must specify datasets to use, or 'all'."
    exit 1
elif [ "$DATASETS" = "all" ]
then
    DATASETS="HIWS HITEMP Banana Banana2 StBernard"
fi

cd $CONTIKI_DIR/tools/cooja

for DATASET in $DATASETS
do
    ant run_nogui -Dargs=$HOME/src/anomaly/$DATASET.csc
    mkdir -v -p $HOME/src/anomaly/logs/$DATASET/$TEST/Test$TEST_NO
    mv -v $HOME/src/anomaly/logs/$DATASET/[0-9]* $HOME/src/anomaly/logs/$DATASET/$TEST/Test$TEST_NO
done
