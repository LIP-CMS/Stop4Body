#!/bin/bash

. setupJSONs.sh
. setupPaths.sh

for DIR in "${NTUPLE_DIR}" "${TEST_DIR}" "${TRAIN_DIR}" ;do
  echo "Processing directory: ${DIR}"
  OUTPUT=${DIR}_${CURRENT_BDT}

  if [[ ! -d ${OUTPUT} ]] ; then
    mkdir -p ${OUTPUT}
  fi

  applyMVA --json ${JSON_PATH}/allSamples.json --method BDT --inDir ${DIR}/ --outDir ${OUTPUT}/
done

