#!/bin/bash

export BASE_PATH=$CMSSW_BASE/src/UserCode/Stop4Body/
export JSON_PATH=$BASE_PATH/Macros/JSON/

export NTUPLE_DIR=/lstore/cms/cbeiraod/Stop4Body/nTuples_v2017-06-05
#export NTUPLE_DIR=/lstore/cms/cbeiraod/Stop4Body/nTuples_v2017-04-03fixed
export SYNC_DIR=/lstore/cms/cbeiraod/Stop4Body/nTuples_sync
export PU_TEST_DIR=/lstore/cms/cbeiraod/Stop4Body/nTuples_PU_test

export CURRENT_BDT=bdt30

export TRAIN_DIR=${NTUPLE_DIR}_train
export TEST_DIR=${NTUPLE_DIR}_test
export BDTTEST_DIR=${TEST_DIR}_${CURRENT_BDT}
export BDTTRAIN_DIR=${TRAIN_DIR}_${CURRENT_BDT}
export BDT_DIR=${TEST_DIR}_${CURRENT_BDT}

export SWAP_DIR=${NTUPLE_DIR}_swap
export SWAPBDT_DIR=${SWAP_DIR}_${CURRENT_BDT}
export LNT_DIR=${NTUPLE_DIR}_looseNotTight
export LNTBDT_DIR=${LNT_DIR}_${CURRENT_BDT}


#export PROCESS_SAMPLES_CMD="processSamples --jetPtThreshold 20"
export PROCESS_SAMPLES_CMD="processSamples --preemptiveDropEvents --doPromptTagging --doLooseLeptons"
export PROCESS_SAMPLES_CMD_LNT="${PROCESS_SAMPLES_CMD} --looseNotTight"
export PROCESS_SAMPLES_CMD_LOOSE="processSamples --preemptiveDropEvents --doLooseLeptons"
