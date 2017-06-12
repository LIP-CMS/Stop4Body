import os,subprocess,ROOT
import pickle
import re
import time

if __name__ == "__main__":
  import argparse

  parser = argparse.ArgumentParser(description='Process the command line options')
  parser.add_argument('-o', '--inDirectory', required=True, help='Name of the input directory')
  parser.add_argument(      '--overloadTestOut', default="", help='Use to define an alternative output directory for the test events')
  parser.add_argument(      '--overloadTrainOut', default="", help='Use to define an alternative output directory for the train events')
  parser.add_argument(      '--previousTestEvents', help='Use to define where the test nTuples from a previous split are located, to be used for this splitting as well')
  parser.add_argument(      '--previousTrainEvents', help='Use to define where the train nTuples from a previous split are located, to be used for this splitting as well')
  parser.add_argument('-s', '--doSwap', action='store_true', help='Whether to process with the swapping of MET and LepPt variables')
  parser.add_argument('-d', '--dryRun', action='store_true', help='Do a dry run (i.e. do not actually run the potentially dangerous commands but print them to the screen)')

  args = parser.parse_args()

  if len([x for x in (args.previousTestEvents,args.previousTrainEvents) if x is not None]) == 1:
    parser.error('--previousTestEvents and --previousTrainEvents must be given together')

  if not args.dryRun:
    print "You did not enable dry run. You are on your own!"

  jsonFiles = ["MC2Process.json",
               "stop250.json",
               "stop275.json",
               "stop300.json",
               "stop325.json",
               "stop350.json",
               "stop375.json",
               "stop400.json",
               "stop425.json",
               "stop450.json",
               "stop475.json",
               "stop500.json",
               "stop525.json",
               "stop550.json",
               "stop575.json",
               "stop600.json",
               "stop625.json",
               "stop650.json",
               "stop675.json",
               "stop700.json",
               "stop725.json",
               "stop750.json",
               "stop775.json",
               "stop800.json"]
  #if args.doSwap:
  #  jsonFiles.append("DataSingleLepton.json")
  #else:
  #  jsonFiles.append("Data.json")


  testOut = args.inDirectory + "_test"
  if (args.overloadTestOut is not None) and (len(args.overloadTestOut) != 0):
    testOut = args.overloadTestOut
  trainOut = args.inDirectory + "_train"
  if (args.overloadTrainOut is not None) and (len(args.overloadTrainOut) != 0):
    trainOut = args.overloadTrainOut

  print "Creating output directories"
  cmd = "mkdir -p " + testOut
  print "  ", cmd
  if not args.dryRun:
    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()
  cmd = "mkdir -p " + trainOut
  print "  ", cmd
  if not args.dryRun:
    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()


  for json in jsonFiles:
    print "Splitting the samples"
    cmd = "splitTrainingTest"
    cmd += " --json JSON/" + json
    cmd += " --inDir " + args.inDirectory
    cmd += " --testOutDir " + testOut
    cmd += " --trainOutDir " + trainOut
    if len([x for x in (args.previousTestEvents,args.previousTrainEvents) if x is not None]) == 2:
      cmd += " --trainTreeDir " + args.previousTrainEvents
      cmd += " --testTreeDir " + args.previousTestEvents
    print "  ", cmd
    if not args.dryRun:
      p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
      out, err = p.communicate()


















