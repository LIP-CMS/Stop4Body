#include <string>
#include <iostream>

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"

void ExtractEventNums(std::string inFileName, std::string outFileName)
{
  std::cout << "Copying from tree: " << inFileName << std::endl;
  std::cout << "Writing to: " << outFileName << std::endl;

  TFile inFile(inFileName.c_str(), "READ");
  TTree* inTree = static_cast<TTree*>(inFile.Get("bdttree"));

  inTree->SetBranchStatus("*", 0);
  inTree->SetBranchStatus("Run", 1);
  inTree->SetBranchStatus("Event", 1);
  inTree->SetBranchStatus("LumiSec", 1);

  TFile outFile(outFileName.c_str(), "RECREATE");
  TTree* outTree = static_cast<TTree*>(inTree->CloneTree());

  outTree->Print();
  outTree->Write();
  //outFile.Write();

  return;
}
