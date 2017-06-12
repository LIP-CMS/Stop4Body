#!/bin/bash

die () {
    echo >&2 "$@"
    exit 1
}

[ "$#" -eq 2 ] || die "2 arguments required, $# provided"

inDir=$1
outDir=$2

[ -d "$inDir" ] || die "Directory $inDir does not exist"

if [[ ! -d "$outDir" ]] ; then
  mkdir -p "$outDir"
fi

for filename in $inDir/*.root; do
  [ -e "$filename" ] || continue
  echo "Running on file: $filename"
  name=${filename##*/}
  base=${name%.root}

  CMD="root -b -q ExtractEventNums.C+\(\\\"$filename\\\",\\\"$outDir/$name\\\"\)"
  #echo "  $name"
  #echo "  $base"

  echo " Command: $CMD"
  eval $CMD
done
