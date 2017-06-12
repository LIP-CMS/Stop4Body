#!/bin/bash

. setupJSONs.sh
. setupPaths.sh

INPUT=~/local-area/Stop4Body/nTuples_v2017-04-03fixed_test
INPUTS=~/local-area/Stop4Body/nTuples_v2017-04-03fixed_swap
#INPUT=~/local-area/Stop4Body/nTuples_v2017-05-27_test
OUTPUT=~/local-area/Stop4Body/ANPlots/
#OUTPUT=./test/

if [[ -d ${INPUT} ]] ; then
  if [[ ! -d ${OUTPUT} ]] ; then
    mkdir -p ${OUTPUT}/DataMC
    mkdir -p ${OUTPUT}/DataMC_swap
  fi

  JSONFILE=variablesAN_DataMC.json
  #makePlots --json ${JSON_PATH}/plot2016_lep.json --outDir ${OUTPUT}/DataMC/ --inDir ${INPUT}/ --variables ${JSONFILE} --cuts ${JSONFILE}

  JSONFILE=variablesAN_DataMC_swap.json
  #makePlots --json ${JSON_PATH}/plot2016swap_lep.json --outDir ${OUTPUT}/DataMC_swap/ --inDir ${INPUTS}/ --variables ${JSONFILE} --cuts ${JSONFILE}

  for deltaM in 10 20 30 40 50 60 70 80; do
  #for deltaM in 50; do
    mkdir -p ${OUTPUT}/BDT/DeltaM${deltaM}/AR
    mkdir -p ${OUTPUT}/BDT/DeltaM${deltaM}/VR1
    mkdir -p ${OUTPUT}/BDT/DeltaM${deltaM}/VR

    JSONFILE=variablesAN_BDT_SR.json
    makePlots --json ${JSON_PATH}/plot2016_DM${deltaM}RP.json --outDir ${OUTPUT}/BDT/DeltaM${deltaM}/AR --inDir ${INPUT}_bdt${deltaM}/ --variables ${JSONFILE} --cuts ${JSONFILE} --suffix bdt
    JSONFILE=variablesAN_BDT_VR1.json
    makePlots --json ${JSON_PATH}/plot2016_DM${deltaM}RP.json --outDir ${OUTPUT}/BDT/DeltaM${deltaM}/VR1 --inDir ${INPUT}_bdt${deltaM}/ --variables ${JSONFILE} --cuts ${JSONFILE} --suffix bdt --unblind

    JSONFILE=variablesAN_BDT_VR.json
    makePlots --json ${JSON_PATH}/plot2016swap_DM${deltaM}RP.json --outDir ${OUTPUT}/BDT/DeltaM${deltaM}/VR --inDir ${INPUTS}_bdt${deltaM}/ --variables ${JSONFILE} --cuts ${JSONFILE} --suffix bdt --unblind
  done
fi
