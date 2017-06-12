#!/bin/bash

. setupJSONs.sh
. setupPaths.sh

INPUT=${BDT_DIR}/
OUTPUT=~cbeiraod/local-area/Stop4Body/BDT_Fits/
OUTPUT_POW=~cbeiraod/local-area/Stop4Body/BDT_Fits_pow/
OUTPUT_LEP=~cbeiraod/local-area/Stop4Body/BDT_Fits_lep/

INPUTS=${SWAPBDT_DIR}/
OUTPUTS=~cbeiraod/local-area/Stop4Body/BDT_Fits_Swap/
OUTPUTS_POW=~cbeiraod/local-area/Stop4Body/BDT_Fits_Swap_pow/
OUTPUTS_LEP=~cbeiraod/local-area/Stop4Body/BDT_Fits_Swap_lep/

if [[ ! -d ${OUTPUT} ]] ; then
  mkdir -p ${OUTPUT}
fi
if [[ ! -d ${OUTPUT_POW} ]] ; then
  mkdir -p ${OUTPUT_POW}
fi
if [[ ! -d ${OUTPUT_LEP} ]] ; then
  mkdir -p ${OUTPUT_LEP}
fi
if [[ ! -d ${OUTPUTS} ]] ; then
  mkdir -p ${OUTPUTS}
fi
if [[ ! -d ${OUTPUTS_POW} ]] ; then
  mkdir -p ${OUTPUTS_POW}
fi
if [[ ! -d ${OUTPUTS_LEP} ]] ; then
  mkdir -p ${OUTPUTS_LEP}
fi

BDTStudy --json ${JSON_PATH}/plot2016.json     --outDir ${OUTPUT}     --inDir ${INPUT} --suffix bdt
BDTStudy --json ${JSON_PATH}/plot2016_pow.json --outDir ${OUTPUT_POW} --inDir ${INPUT} --suffix bdt
#BDTStudy --json ${JSON_PATH}/plot2016_lep.json --outDir ${OUTPUT_LEP} --inDir ${INPUT} --suffix bdt
BDTStudy --json ${JSON_PATH}/plot2016_DM80RP.json --outDir ${OUTPUT_LEP} --inDir ${INPUT} --suffix bdt

BDTStudy --json ${JSON_PATH}/plot2016swap.json     --outDir ${OUTPUTS}     --inDir ${INPUTS} --suffix bdt --unblind
BDTStudy --json ${JSON_PATH}/plot2016swap_pow.json --outDir ${OUTPUTS_POW} --inDir ${INPUTS} --suffix bdt --unblind
#BDTStudy --json ${JSON_PATH}/plot2016swap_lep.json --outDir ${OUTPUTS_LEP} --inDir ${INPUTS} --suffix bdt --unblind
BDTStudy --json ${JSON_PATH}/plot2016swap_DM80RP.json --outDir ${OUTPUTS_LEP} --inDir ${INPUTS} --suffix bdt --unblind
