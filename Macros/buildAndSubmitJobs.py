import os,subprocess,ROOT
import pickle
import re
import time

def getNJobs():
  cmd = "qstat | wc -l"
  p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  out, err = p.communicate()

  return int(out)

if __name__ == "__main__":
  import argparse

  parser = argparse.ArgumentParser(description='Process the command line options')
  parser.add_argument('-o', '--outDirectory', required=True, help='Name of the output directory')
  parser.add_argument('-j', '--jsonTemplate', default="./Templates/jsonTemplate.json", help='Template for the json file for each job')
  parser.add_argument('-t', '--jobTemplate', default="./Templates/Step1_JobTemplate.sh", help='Template for the script for each job')
  parser.add_argument(      '--filterSubmit', default="*", help='Glob to be used when searching which samples have to be submitted')
  parser.add_argument('-s', '--doSwap', action='store_true', help='Whether to process with the swapping of MET and LepPt variables')
  parser.add_argument('-d', '--dryRun', action='store_true', help='Do a dry run (i.e. do not actually run the potentially dangerous commands but print them to the screen)')

  args = parser.parse_args()

  if not args.dryRun:
    print "You did not enable dry run. You are on your own!"

  cmd = "./setupJSONs.sh"
  p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  out, err = p.communicate()

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
  if args.doSwap:
    jsonFiles.append("DataSingleLepton.json")
  else:
    jsonFiles.append("Data.json")

  cmd = "buildJobs --template " + args.jobTemplate + " --jsonTemplate " + args.jsonTemplate + " --outDir " + args.outDirectory
  if args.doSwap:
    cmd = cmd + " --swap"

  for json in jsonFiles:
    thisCMD = cmd + " --samples JSON/" + json
    print "Running the command:", thisCMD
    p = subprocess.Popen(thisCMD, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()

  cmd = "ls -d " + args.outDirectory + "/" + args.filterSubmit + "/"
  p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  out, err = p.communicate()
  samples = out.split()

  for sample in samples:
    sampleName = os.path.basename(os.path.normpath(sample))
    print "Processing sample:", sampleName
    cmd = "ls " + sample + "/*.sh"
    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()
    jobs = out.split()

    cwd = os.getcwd() # for now, we need to change the directory so that the out and err files are in the right place
    os.chdir(sample)

    jobInfo = {}
    for job in jobs:
      jobName = os.path.basename(job)
      cmd = "qsub " + job + " -e " + job + ".e$JOB_ID -o " + job + ".o$JOB_ID"
      if args.dryRun:
        print "Going to run command:", cmd
      if not args.dryRun:
        p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = p.communicate()
        p = re.compile("Your job (\d+) .+")
        jobNumber = p.search(out).group(1)

        jobInfo[jobName] = jobNumber

    os.chdir(cwd)

    with open(sample + '/jobs.pickle', 'wb') as handle:
      pickle.dump(jobInfo, handle, protocol=pickle.HIGHEST_PROTOCOL)

    if getNJobs() > 1700:
      print "Waiting for some jobs to complete..."
      while getNJobs() > 1000:
        time.sleep(5*60)
      print "Done waiting"















