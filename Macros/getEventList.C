#include "TFile.h"
#include "TTree.h"

#include <iostream>
#include <fstream>


int getEventList()
{
  TFile inFile("Wjets_100to200.root", "READ");
  TTree* inTree = static_cast<TTree*>(inFile.Get("bdttree"));

  UInt_t Run;
  ULong64_t Event;
  UInt_t LumiSec;
  inTree->SetBranchAddress("Run", &Run);
  inTree->SetBranchAddress("Event", &Event);
  inTree->SetBranchAddress("LumiSec", &LumiSec);

  Float_t nGoodEl;   inTree->SetBranchAddress("nGoodEl",   &nGoodEl);
  Float_t nGoodMu;   inTree->SetBranchAddress("nGoodMu",   &nGoodMu);
  Float_t LepID;     inTree->SetBranchAddress("LepID",     &LepID);
  Float_t LepChg;    inTree->SetBranchAddress("LepChg",    &LepChg);
  Float_t LepPt;     inTree->SetBranchAddress("LepPt",     &LepPt);
  Float_t LepEta;    inTree->SetBranchAddress("LepEta",    &LepEta);
  Float_t LepDxy;    inTree->SetBranchAddress("LepDxy",    &LepDxy);
  Float_t LepDz;     inTree->SetBranchAddress("LepDz",     &LepDz);
  Float_t LepIso03;  inTree->SetBranchAddress("LepIso03",  &LepIso03);
  Float_t Lep2ID;    inTree->SetBranchAddress("Lep2ID",    &Lep2ID);
  Float_t Lep2Chg;   inTree->SetBranchAddress("Lep2Chg",   &Lep2Chg);
  Float_t Lep2Pt;    inTree->SetBranchAddress("Lep2Pt",    &Lep2Pt);
  Float_t Lep2Eta;   inTree->SetBranchAddress("Lep2Eta",   &Lep2Eta);
  Float_t Lep2Dxy;   inTree->SetBranchAddress("Lep2Dxy",   &Lep2Dxy);
  Float_t Lep2Dz;    inTree->SetBranchAddress("Lep2Dz",    &Lep2Dz);
  Float_t Lep2Iso03; inTree->SetBranchAddress("Lep2Iso03", &Lep2Iso03);
  Float_t Met; inTree->SetBranchAddress("Met",&Met);
  Float_t mt; inTree->SetBranchAddress("mt",&mt);
  Float_t HT;  inTree->SetBranchAddress("HT",&HT);
  Float_t Njet;  inTree->SetBranchAddress("Njet",&Njet);
  Float_t Jet1Pt;  inTree->SetBranchAddress("Jet1Pt",&Jet1Pt);
  Float_t Jet2Pt;  inTree->SetBranchAddress("Jet2Pt",&Jet2Pt);
  Float_t Jet3Pt;  inTree->SetBranchAddress("Jet3Pt",&Jet3Pt);
  Float_t DPhiJet1Jet2;  inTree->SetBranchAddress("DPhiJet1Jet2",&DPhiJet1Jet2);


  ofstream outFile ("eventlist.txt", std::ios::binary);

  Long64_t nentries = inTree->GetEntries();
  for(Long64_t i = 0; i < nentries; ++i)
  {
    if(i >= 100)
      break;
    inTree->GetEntry(i);

    outFile << "Run: " << Run << "; Event: " << Event << "; Lumisection: " << LumiSec << std::endl;
    outFile << "nGoodEl: " << nGoodEl << "; nGoodMu: " << nGoodMu << std::endl;
    outFile << "LepID: " << LepID;
    outFile << "; LepChg: " << LepChg;
    outFile << "; LepPt: " << LepPt;
    outFile << "; LepEta: " << LepEta;
    outFile << "; LepDxy: " << LepDxy;
    outFile << "; LepDz: " << LepDz;
    outFile << "; LepIso03: " << LepIso03 << std::endl;
    outFile << "Lep2ID: " << Lep2ID;
    outFile << "; Lep2Chg: " << Lep2Chg;
    outFile << "; Lep2Pt: " << Lep2Pt;
    outFile << "; Lep2Eta: " << Lep2Eta;
    outFile << "; Lep2Dxy: " << Lep2Dxy;
    outFile << "; Lep2Dz: " << Lep2Dz;
    outFile << "; Lep2Iso03: " << Lep2Iso03 << std::endl;
    outFile << "Met: " << Met << std::endl;
    outFile << "mt: " << mt << std::endl;
    outFile << "HT: " << HT << std::endl;
    outFile << "Njet: " << Njet << std::endl;
    outFile << "Jet1Pt: " << Jet1Pt << std::endl;
    outFile << "Jet2Pt: " << Jet2Pt << std::endl;
    outFile << "Jet3Pt: " << Jet3Pt << std::endl;
    outFile << "DPhiJet1Jet2: " << DPhiJet1Jet2 << std::endl;
    outFile << std::endl;
  }

  return 0;
}
