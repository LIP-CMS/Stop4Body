#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <list>
#include <iterator>
#include <algorithm>
#include <iomanip>
#include <math.h>
#include <memory>
#include <cmath>
#include <cstdlib>

#include "TROOT.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TChain.h"
#include "TObject.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TLegend.h"
#include "TGraph.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TTree.h"
#include "TF1.h"
#include "TCutG.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"
#include "TMultiGraph.h"
#include "TPaveText.h"
#include "THStack.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TLorentzVector.h"
#include "TVectorD.h"

#include "UserCode/Stop4Body/interface/json.hpp"
#include "UserCode/Stop4Body/interface/SampleReader.h"
#include "UserCode/Stop4Body/interface/commonFunctions.h"
#include "UserCode/Stop4Body/interface/doubleWithUncertainty.h"

#define GENPART_LIMIT  40
#define JETCOLL_LIMIT  40
#define LEPCOLL_LIMIT  40
#define TAUCOLL_LIMIT  40
#define TRACOLL_LIMIT 200

#define ISR_JET_PT 90
#define SECOND_LEPTON_PT 20
#define MIN_MET 100

using json = nlohmann::json;

struct FileInfo
{
  std::string path;
  double crossSection;
  double branchingRatio;
  std::string tag;
};

void printHelp();

const double ECALGap_MinEta =  1.4442; // ECAL gap parameters
const double ECALGap_MaxEta =  1.5660;
const double CSV_Loose = 0.5426;
const double CSV_Medium = 0.800; // TODO: Update
const double CSV_Tight = 0.935;

extern TH2D* centralElectronSFHist;
extern TH2D* centralMuonSFHist;
extern TH1F* hephyElectronIDSFHistBarrel;
extern TH1F* hephyElectronIDSFHistEndcap;
extern TH1F* hephyMuonIDSFHist;
extern TH1F* hephyElectronISOSFHistBarrel;
extern TH1F* hephyElectronISOSFHistEndcap;
extern TH1F* hephyMuonISOSFHist;
extern TH1F* electronTightToLooseLowEta;
extern TH1F* electronTightToLooseHighEta;
extern TH1F* muonTightToLooseLowEta;
extern TH1F* muonTightToLooseHighEta;

int main(int argc, char** argv)
{
  std::string jsonFileName = "";
  std::string outputDirectory = "./OUT/";
  bool doSync = false;
  bool noSkim = false;
  std::string suffix = "";
  size_t max_sync_count = 0;
  double jetPtThreshold = 30;
  bool overrideXSec = false;
  bool swap = false;
  bool looseNotTight = false;
  bool preemptiveDropEvents = false;
  bool doLooseLeptons = false;
  bool doPromptTagging = false;
  int part = -1;

  if(argc < 2)
  {
    std::cout << "You did not pass enough parameters" << std::endl;
    printHelp();
    return 0;
  }

  for(int i = 1; i < argc; ++i)
  {
    std::string argument = argv[i];

    if(argument == "--help")
    {
      printHelp();
      return 0;
    }

    if(argument == "--json")
      jsonFileName = argv[++i];

    if(argument == "--outDir")
      outputDirectory = argv[++i];

    if(argument == "--doSync")
    {
      std::stringstream converter;
      doSync = true;
      converter << argv[++i];
      converter >> max_sync_count;
    }

    if(argument == "--suffix")
      suffix = argv[++i];

    if(argument == "--noSkim")
      noSkim = true;

    if(argument == "--jetPtThreshold")
    {
      std::stringstream converter;
      converter << argv[++i];
      converter >> jetPtThreshold;
    }

    if(argument == "--overrideXSec")
      overrideXSec = true;

    if(argument == "--swap")
      swap = true;

    if(argument == "--looseNotTight")
      looseNotTight = true;

    if(argument == "--preemptiveDropEvents")
      preemptiveDropEvents = true;

    if(argument == "--doLooseLeptons")
      doLooseLeptons = true;

    if(argument == "--doPromptTagging")
      doPromptTagging = true;

    if(argument == "--part")
    {
      std::stringstream converter;
      converter << argv[++i];
      converter >> part;
    }
  }

  if(jsonFileName == "")
  {
    std::cout << "You must define a json file" << std::endl;
    return 1;
  }

  if(!fileExists(outputDirectory+"/puWeights.root"))
  {
    std::cout << "You must first obtain the PU weights before trying to process the samples" << std::endl;
    return 1;
  }

  TDirectory* cwd = gDirectory;
  TFile puWeightFile((outputDirectory + "/puWeights.root").c_str(), "READ");
  TFile centralElectronSFFile("../data/scaleFactors.root", "READ");
  centralElectronSFHist = static_cast<TH2D*>(centralElectronSFFile.Get("GsfElectronToCutBasedSpring15V"));
  TFile centralMuonSFFile("../data/TnP_NUM_LooseID_DENOM_generalTracks_VAR_map_pt_eta.root", "READ");
  centralMuonSFHist = static_cast<TH2D*>(centralMuonSFFile.Get("SF"));
  TFile hephySFFile("../data/hephy_scale_factors_10p0binning.root", "READ");
  hephyElectronIDSFHistBarrel = static_cast<TH1F*>(hephySFFile.Get("ele_SF_IdSpec_barrel"));
  hephyElectronIDSFHistEndcap = static_cast<TH1F*>(hephySFFile.Get("ele_SF_IdSpec_endcap"));
  hephyMuonIDSFHist = static_cast<TH1F*>(hephySFFile.Get("muon_SF_Id_all"));
  hephyElectronISOSFHistBarrel = static_cast<TH1F*>(hephySFFile.Get("ele_SF_IpIso_barrel"));
  hephyElectronISOSFHistEndcap = static_cast<TH1F*>(hephySFFile.Get("ele_SF_IpIso_endcap"));
  hephyMuonISOSFHist = static_cast<TH1F*>(hephySFFile.Get("muon_SF_IpIsoSpec_all"));
  TFile tightToLooseRatios("../data/tightToLooseRatios.root", "READ");
  electronTightToLooseLowEta = static_cast<TH1F*>(tightToLooseRatios.Get("electronEfficiencyLowEta"));
  electronTightToLooseHighEta = static_cast<TH1F*>(tightToLooseRatios.Get("electronEfficiencyHighEta"));
  muonTightToLooseLowEta = static_cast<TH1F*>(tightToLooseRatios.Get("muonEfficiencyLowEta"));
  muonTightToLooseHighEta = static_cast<TH1F*>(tightToLooseRatios.Get("muonEfficiencyHighEta"));
  cwd->cd();

  std::cout << "Reading JSON file" << std::endl;
  SampleReader samples(jsonFileName);

  for(auto &process : samples)
  {
    std::cout << "Processing process: " << process.tag() << std::endl;

    TH1D* puWeightDistrib = static_cast<TH1D*>(puWeightFile.Get( ("process_"+process.tag()+"_puWeight").c_str())->Clone("puWeightDistrib"));

    for(auto &sample : process)
    {
      std::cout << "\tProcessing sample: " << sample.tag() << std::endl;
      std::string outputFile = outputDirectory + "/" + sample.tag();
      if(suffix != "")
        outputFile += "_" + suffix;
      if(part >= 0)
      {
        outputFile += "_Part";
        std::stringstream converter;
        converter << part;
        outputFile += converter.str();
      }
      outputFile += ".root";
      std::cout << "\t  Putting output in: " << outputFile << std::endl;

      std::ofstream SyFile;
      if(doSync)
      {
        SyFile.open(outputDirectory + "/" + sample.tag() + "_synch.txt", std::ofstream::out | std::ofstream::trunc);
        SyFile.setf(std::ios::fixed);
        SyFile.precision(3);
      }

      size_t sync_count = 0;

      TFile foutput(outputFile.c_str(), "RECREATE");
      TTree *bdttree= new TTree("bdttree","bdttree");

      // New branch in bdt tree
      UInt_t Run;  bdttree->Branch("Run",&Run);
      ULong64_t Event;  bdttree->Branch("Event",&Event);
      UInt_t LumiSec;  bdttree->Branch("LumiSec",&LumiSec);
      Float_t Nevt;  bdttree->Branch("Nevt",&Nevt,"Nevt/F");
      Float_t XS; bdttree->Branch("XS",&XS,"XS/F");
      Float_t nVert; bdttree->Branch("nVert", &nVert, "nVert/F");

      Float_t genGravitinoM; bdttree->Branch("genGravitinoM", &genGravitinoM, "genGravitinoM/F");
      Float_t genStopM; bdttree->Branch("genStopM", &genStopM, "genStopM/F");
      Float_t genSbottomM; bdttree->Branch("genSbottomM", &genSbottomM, "genSbottomM/F");
      Float_t genNeutralinoM; bdttree->Branch("genNeutralinoM", &genNeutralinoM, "genNeutralinoM/F");

      Float_t genWeight=1; bdttree->Branch("genWeight", &genWeight, "genWeight/F");
      Float_t sumGenWeight=1; bdttree->Branch("sumGenWeight", &sumGenWeight, "sumGenWeight/F");
      Float_t filterEfficiency=1; bdttree->Branch("filterEfficiency", &filterEfficiency, "filterEfficiency/F");
      Float_t splitFactor=1; bdttree->Branch("splitFactor", &splitFactor, "splitFactor/F");
      Float_t triggerEfficiency=1; bdttree->Branch("triggerEfficiency", &triggerEfficiency, "triggerEfficiency/F");
      Float_t EWKISRweight=1; bdttree->Branch("EWKISRweight", &EWKISRweight, "EWKISRweight/F");
      Float_t ISRweight=1; bdttree->Branch("ISRweight", &ISRweight, "ISRweight/F");
      Float_t puWeight=1; bdttree->Branch("puWeight", &puWeight, "puWeight/F");
      Float_t leptonIDSF=1; bdttree->Branch("leptonIDSF", &leptonIDSF, "leptonIDSF/F");
      Float_t leptonISOSF=1; bdttree->Branch("leptonISOSF", &leptonISOSF, "leptonISOSF/F");
      Float_t weight=1; bdttree->Branch("weight", &weight, "weight/F");
      bool isLooseNotTight=false; bdttree->Branch("isLooseNotTight", &isLooseNotTight);
      Float_t looseNotTightWeight=1; bdttree->Branch("looseNotTightWeight", &looseNotTightWeight, "looseNotTightWeight/F");

      bool isTight;      bdttree->Branch("isTight",   &isTight);
      bool isLoose;      bdttree->Branch("isLoose",   &isLoose);
      Float_t LepID;     bdttree->Branch("LepID",     &LepID,     "LepID/F");
      Float_t LepChg;    bdttree->Branch("LepChg",    &LepChg,    "LepChg/F");
      Float_t LepPt;     bdttree->Branch("LepPt",     &LepPt,     "LepPt/F");
      Float_t LepEta;    bdttree->Branch("LepEta",    &LepEta,    "LepEta/F");
      Float_t LepDxy;    bdttree->Branch("LepDxy",    &LepDxy,    "LepDxy/F");
      Float_t LepDz;     bdttree->Branch("LepDz",     &LepDz,     "LepDz/F");
      Float_t LepIso03;  bdttree->Branch("LepIso03",  &LepIso03,  "LepIso03/F");
      Float_t LepHybIso03; bdttree->Branch("LepHybIso03",  &LepHybIso03,  "LepHybIso03/F");
      Float_t isPrompt;  bdttree->Branch("isPrompt",  &isPrompt,  "isPrompt/F");
      Float_t Lep2ID;    bdttree->Branch("Lep2ID",    &Lep2ID,    "Lep2ID/F");
      Float_t Lep2Chg;   bdttree->Branch("Lep2Chg",   &Lep2Chg,   "Lep2Chg/F");
      Float_t Lep2Pt;    bdttree->Branch("Lep2Pt",    &Lep2Pt,    "Lep2Pt/F");
      Float_t Lep2Eta;   bdttree->Branch("Lep2Eta",   &Lep2Eta,   "Lep2Eta/F");
      Float_t Lep2Dxy;   bdttree->Branch("Lep2Dxy",   &Lep2Dxy,   "Lep2Dxy/F");
      Float_t Lep2Dz;    bdttree->Branch("Lep2Dz",    &Lep2Dz,    "Lep2Dz/F");
      Float_t Lep2Iso03; bdttree->Branch("Lep2Iso03", &Lep2Iso03, "Lep2Iso03/F");
      Float_t nGoodMu;   bdttree->Branch("nGoodMu",&nGoodMu,"nGoodMu/F");
      Float_t nGoodEl;   bdttree->Branch("nGoodEl",&nGoodEl,"nGoodEl/F");
      Float_t nGoodMu_loose;
      Float_t nGoodEl_loose;
      Float_t Met; bdttree->Branch("Met",&Met,"Met/F");
      Float_t mt; bdttree->Branch("mt",&mt,"mt/F");
      Float_t mt_old; bdttree->Branch("mt_old",&mt_old,"mt_old/F");
      Float_t Q80; bdttree-> Branch("Q80",&Q80,"Q80/F");
      Float_t CosDeltaPhi; bdttree->Branch("CosDeltaPhi",&CosDeltaPhi,"CosDeltaPhi/F");

      Float_t nIsr_out; bdttree->Branch("nIsr", &nIsr_out, "nIsr/F");
      Float_t Njet;  bdttree->Branch("Njet",&Njet,"Njet/F");
      Float_t Njet30;  bdttree->Branch("Njet30",&Njet30,"Njet30/F");
      Float_t Njet40;  bdttree->Branch("Njet40",&Njet40,"Njet40/F");
      Float_t Njet50;  bdttree->Branch("Njet50",&Njet50,"Njet50/F");
      Float_t Njet60;  bdttree->Branch("Njet60",&Njet60,"Njet60/F");
      Float_t Njet70;  bdttree->Branch("Njet70",&Njet70,"Njet70/F");
      Float_t Njet80;  bdttree->Branch("Njet80",&Njet80,"Njet80/F");
      Float_t Njet90;  bdttree->Branch("Njet90",&Njet90,"Njet90/F");
      Float_t Njet100;  bdttree->Branch("Njet100",&Njet100,"Njet100/F");
      Float_t Jet1Pt;  bdttree->Branch("Jet1Pt",&Jet1Pt,"Jet1Pt/F");
      Float_t Jet1Eta;  bdttree->Branch("Jet1Eta",&Jet1Eta,"Jet1Eta/F");
      Float_t Jet1CSV;  bdttree->Branch("Jet1CSV",&Jet1CSV,"Jet1CSV/F");
      Float_t Jet2Pt;  bdttree->Branch("Jet2Pt",&Jet2Pt,"Jet2Pt/F");
      Float_t Jet2Eta;  bdttree->Branch("Jet2Eta",&Jet2Eta,"Jet2Eta/F");
      Float_t Jet2CSV;  bdttree->Branch("Jet2CSV",&Jet2CSV,"Jet2CSV/F");
      Float_t Jet3Pt;  bdttree->Branch("Jet3Pt",&Jet3Pt,"Jet3Pt/F");
      Float_t Jet3Eta;  bdttree->Branch("Jet3Eta",&Jet3Eta,"Jet3Eta/F");
      Float_t Jet3CSV;  bdttree->Branch("Jet3CSV",&Jet3CSV,"Jet3CSV/F");
      Float_t DPhiJet1Jet2;  bdttree->Branch("DPhiJet1Jet2",&DPhiJet1Jet2,"DPhiJet1Jet2/F");
      Float_t HT;  bdttree->Branch("HT",&HT,"HT/F");

      Float_t NbLoose; bdttree->Branch("NbLoose",&NbLoose,"NbLoose/F");
      Float_t NbTight;  bdttree->Branch("NbTight",&NbTight,"NbTight/F");
      Float_t NbLooseTo50; bdttree->Branch("NbLooseTo50",&NbLooseTo50,"NbLooseTo50/F");
      Float_t NbLoose50;     bdttree->Branch("NbLoose50",&NbLoose50,"NbLoose50/F");
      Float_t NbMediumTo50; bdttree->Branch("NbMediumTo50",&NbMediumTo50,"NbMediumTo50/F");
      Float_t NbMedium50;     bdttree->Branch("NbMedium50",&NbMedium50,"NbMedium50/F");
      Float_t NbTightTo50; bdttree->Branch("NbTightTo50",&NbTightTo50,"NbTightTo50/F");
      Float_t NbTight50;     bdttree->Branch("NbTight50",&NbTight50,"NbTight50/F");
      Float_t JetHBpt;  bdttree->Branch("JetHBpt",&JetHBpt,"JetHBpt/F");
      Float_t JetHBeta;  bdttree->Branch("JetHBeta",&JetHBeta,"JetHBeta/F");
      Float_t JetHBindex; bdttree->Branch("JetHBindex", &JetHBindex, "JetHBindex/F");
      Float_t JetHBCSV; bdttree->Branch("JetHBCSV", &JetHBCSV, "JetHBCSV/F");
      Float_t JetB2pt;  bdttree->Branch("JetB2pt",&JetB2pt,"JetB2pt/F");
      Float_t JetB2eta;  bdttree->Branch("JetB2eta",&JetB2eta,"JetB2eta/F");
      Float_t JetB2index; bdttree->Branch("JetB2index", &JetB2index, "JetB2index/F");
      Float_t JetB2CSV; bdttree->Branch("JetB2CSV", &JetB2CSV, "JetB2CSV/F");

      Float_t DrJet1Lep;  bdttree->Branch("DrJet1Lep",&DrJet1Lep,"DrJet1Lep/F");
      Float_t DrJet2Lep;  bdttree->Branch("DrJet2Lep",&DrJet2Lep,"DrJet2Lep/F");
      Float_t DrJetHBLep;  bdttree->Branch("DrJetHBLep",&DrJetHBLep,"DrJetHBLep/F");
      Float_t DrJet1Jet2;  bdttree->Branch("DrJet1Jet2",&DrJet1Jet2,"DrJet1Jet2/F");
      Float_t JetLepMass;  bdttree->Branch("JetLepMass",&JetLepMass,"JetLepMass/F");
      Float_t J3Mass;  bdttree->Branch("J3Mass",&J3Mass,"J3Mass/F");

      Float_t HLT_PFMET90_PFMHT90;   bdttree->Branch("HLT_PFMET90_PFMHT90",   &HLT_PFMET90_PFMHT90,   "HLT_PFMET90_PFMHT90/F"  );
      Float_t HLT_PFMET100_PFMHT100; bdttree->Branch("HLT_PFMET100_PFMHT100", &HLT_PFMET100_PFMHT100, "HLT_PFMET100_PFMHT100/F");
      Float_t HLT_PFMET110_PFMHT110; bdttree->Branch("HLT_PFMET110_PFMHT110", &HLT_PFMET110_PFMHT110, "HLT_PFMET110_PFMHT110/F");
      Float_t HLT_PFMET120_PFMHT120; bdttree->Branch("HLT_PFMET120_PFMHT120", &HLT_PFMET120_PFMHT120, "HLT_PFMET120_PFMHT120/F");
      Float_t HLT_Ele;               bdttree->Branch("HLT_Ele",               &HLT_Ele,               "HLT_Ele/F"              );
      Float_t HLT_Mu;                bdttree->Branch("HLT_Mu",                &HLT_Mu,                "HLT_Mu/F"               );

      Float_t METFilters; bdttree->Branch("METFilters", &METFilters, "METFilters/F");
      Float_t HBHENoiseFilter; bdttree->Branch("HBHENoiseFilter", &HBHENoiseFilter,"HBHENoiseFilter/F");
      Float_t HBHENoiseIsoFilter; bdttree->Branch("HBHENoiseIsoFilter", &HBHENoiseIsoFilter,"HBHENoiseIsoFilter/F");
      Float_t eeBadScFilter; bdttree->Branch("eeBadScFilter", &eeBadScFilter,"eeBadScFilter/F");
      Float_t EcalDeadCellTriggerPrimitiveFilter; bdttree->Branch("EcalDeadCellTriggerPrimitiveFilter", &EcalDeadCellTriggerPrimitiveFilter,"EcalDeadCellTriggerPrimitiveFilter/F");
      Float_t goodVertices; bdttree->Branch("goodVertices", &goodVertices,"goodVertices/F");
      Float_t globalTightHalo2016Filter; bdttree->Branch("globalTightHalo2016Filter", &globalTightHalo2016Filter, "globalTightHalo2016Filter/F");
      Float_t badMuonMoriond2017; bdttree->Branch("badMuonMoriond2017", &badMuonMoriond2017, "badMuonMoriond2017/F");
      Float_t badCloneMuonMoriond2017; bdttree->Branch("badCloneMuonMoriond2017", &badCloneMuonMoriond2017, "badCloneMuonMoriond2017/F");
      Float_t badMuonFilter; bdttree->Branch("badMuonFilter", &badMuonFilter, "badMuonFilter/F");
      Float_t badChargedHadronFilter; bdttree->Branch("badChargedHadronFilter", &badChargedHadronFilter, "badChargedHadronFilter/F");


      Float_t Ncut0;  //bdttree->Branch("Ncut0",&Ncut0,"Ncut0/F");
      Float_t Ncut1;
      Float_t Ncut2;
      Float_t Ncut3;
      Float_t Ncut4;
      Float_t Ncut5;
      Float_t Ncut6;


      TH2F* filterEfficiencyH = nullptr;
      if(sample.filterEfficiencyFile() != "")
      {
        TDirectory* cwd = gDirectory;
        TFile filterEfficiencyFile(sample.filterEfficiencyFile().c_str(), "READ");
        cwd->cd();
        filterEfficiencyH = static_cast<TH2F*>(filterEfficiencyFile.Get("filterEfficiency")->Clone("filterEfficiencyH"));
      }

      // Get total number of entries and other important values
      Nevt = 0;
      Ncut0 = 0;
      Ncut1 = 0;
      Ncut2 = 0;
      Ncut3 = 0;
      Ncut4 = 0;
      Ncut5 = 0;
      Ncut6 = 0;
      double ISRCParam = 1;
      double EWKISRCParam = 1;
      if(!process.isdata()) // Only perform the below for MC
      {
        std::cout << "\t  Getting Initial number of events, nvtx distribution and sum of gen weights: " << std::flush;

        TVectorD* sampleNevt = static_cast<TVectorD*>(puWeightFile.Get(("sample_"+sample.tag()+"_Nevt").c_str())->Clone("sampleNevt"));
        Nevt = (*sampleNevt)[0];
        TVectorD* sampleSumGenWeight = static_cast<TVectorD*>(puWeightFile.Get(("sample_"+sample.tag()+"_sumGenWeight").c_str())->Clone("sampleSumGenWeight"));
        sumGenWeight = (*sampleSumGenWeight)[0];
        TVectorD* processISRWeights = static_cast<TVectorD*>(puWeightFile.Get(("process_"+process.tag()+"_ISRCParams").c_str())->Clone("processISRWeights"));
        ISRCParam =    (*processISRWeights)[0];
        EWKISRCParam = (*processISRWeights)[1];

        delete sampleNevt;
        delete sampleSumGenWeight;
        delete processISRWeights;

        std::cout << Nevt << "; " << sumGenWeight << std::endl;
        std::cout << "\t  C value for ISR reweighting: " << ISRCParam << std::endl;
        std::cout << "\t  C value for EWK ISR reweighting: " << EWKISRCParam << std::endl;
      }

      for(auto &file : sample)
      {
        if(doSync && max_sync_count > 0 && sync_count >= max_sync_count)
          break;
        std::cout << "\t  Processing file: " << file << std::endl;
        TFile finput(file.c_str(), "READ");
        foutput.cd();
        TTree *inputtree;
        if(process.selection() != "")
          inputtree = static_cast<TTree*>(finput.Get("tree"))->CopyTree(process.selection().c_str());
        else
          inputtree = static_cast<TTree*>(finput.Get("tree"));

        // Read Branches you are interested in from the input tree
        Float_t mtw1;        inputtree->SetBranchAddress("event_mtw1"       , &mtw1);
        Float_t mtw2;        inputtree->SetBranchAddress("event_mtw2"       , &mtw2);
        Float_t mtw;         inputtree->SetBranchAddress("event_mtw"       , &mtw);
        Float_t met_pt;      inputtree->SetBranchAddress("met_pt"    , &met_pt);
        Float_t met_phi;     inputtree->SetBranchAddress("met_phi",   &met_phi);
        Int_t nLepGood;      inputtree->SetBranchAddress("nLepGood"   , &nLepGood);
        Int_t LepGood_pdgId[LEPCOLL_LIMIT];  inputtree->SetBranchAddress("LepGood_pdgId", &LepGood_pdgId);
        Int_t LepGood_mediumMuonId[LEPCOLL_LIMIT]; inputtree->SetBranchAddress("LepGood_mediumMuonId",&LepGood_mediumMuonId);
        Float_t LepGood_pt[LEPCOLL_LIMIT];  inputtree->SetBranchAddress("LepGood_pt", &LepGood_pt);
        Float_t LepGood_eta[LEPCOLL_LIMIT];  inputtree->SetBranchAddress("LepGood_eta", &LepGood_eta);
        Float_t LepGood_phi[LEPCOLL_LIMIT];  inputtree->SetBranchAddress("LepGood_phi", &LepGood_phi);
        Float_t LepGood_relIso03[LEPCOLL_LIMIT]; inputtree->SetBranchAddress("LepGood_relIso03",&LepGood_relIso03);
        Float_t LepGood_absIso03[LEPCOLL_LIMIT]; inputtree->SetBranchAddress("LepGood_absIso03",&LepGood_absIso03);
        Float_t LepGood_dxy[LEPCOLL_LIMIT]; inputtree->SetBranchAddress("LepGood_dxy",&LepGood_dxy);
        Float_t LepGood_dz[LEPCOLL_LIMIT]; inputtree->SetBranchAddress("LepGood_dz",&LepGood_dz);
        Float_t LepGood_mass[LEPCOLL_LIMIT]; inputtree->SetBranchAddress("LepGood_mass",&LepGood_mass);
        Float_t LepGood_mt[LEPCOLL_LIMIT]; inputtree->SetBranchAddress("LepGood_mt", &LepGood_mt);
        Float_t LepGood_Q80[LEPCOLL_LIMIT]; inputtree->SetBranchAddress("LepGood_Q80", &LepGood_Q80);
        Float_t LepGood_cosPhiLepMet[LEPCOLL_LIMIT]; inputtree->SetBranchAddress("LepGood_cosPhiLepMet", &LepGood_cosPhiLepMet);
        Int_t LepGood_charge[LEPCOLL_LIMIT]; inputtree->SetBranchAddress("LepGood_charge",&LepGood_charge);
        Int_t nJetIn;  inputtree->SetBranchAddress("nJet", &nJetIn);
        Float_t Jet_pt[JETCOLL_LIMIT];  inputtree->SetBranchAddress("Jet_pt", &Jet_pt);
        Int_t Jet_id[JETCOLL_LIMIT];  inputtree->SetBranchAddress("Jet_id", &Jet_id);
        Float_t Jet_eta[JETCOLL_LIMIT];  inputtree->SetBranchAddress("Jet_eta", &Jet_eta);
        Float_t Jet_phi[JETCOLL_LIMIT];  inputtree->SetBranchAddress("Jet_phi", &Jet_phi);
        Float_t Jet_btagCSV[JETCOLL_LIMIT];  inputtree->SetBranchAddress("Jet_btagCSV", &Jet_btagCSV);
        Float_t Jet_mass[JETCOLL_LIMIT];  inputtree->SetBranchAddress("Jet_mass", &Jet_mass);
        Float_t Jet_rawPt[JETCOLL_LIMIT]; inputtree->SetBranchAddress("Jet_rawPt", &Jet_rawPt);
        UInt_t run;  inputtree->SetBranchAddress("run", &run);
        ULong64_t evt;  inputtree->SetBranchAddress("evt", &evt);
        UInt_t lumi;  inputtree->SetBranchAddress("lumi", &lumi);
        Int_t nVert_i; inputtree->SetBranchAddress("nVert", &nVert_i);

        Float_t nIsr; inputtree->SetBranchAddress("nIsr", &nIsr);

        Int_t   LepGood_mcMatchId[LEPCOLL_LIMIT];
        Int_t nGenPart = 0;
        Int_t GenPart_motherId[GENPART_LIMIT];
        Int_t GenPart_grandmotherId[GENPART_LIMIT];
        Int_t GenPart_sourceId[GENPART_LIMIT];
        Int_t GenPart_status[GENPART_LIMIT];
        Int_t GenPart_pdgId[GENPART_LIMIT];
        Float_t GenPart_pt[GENPART_LIMIT];
        Float_t GenPart_eta[GENPART_LIMIT];
        Float_t GenPart_phi[GENPART_LIMIT];
        Float_t xsec = 1;
        Float_t nTrueInt = 1;
        if(!process.isdata())
        {
          inputtree->SetBranchAddress("genWeight", &genWeight);
          inputtree->SetBranchAddress("xsec", &xsec);
          inputtree->SetBranchAddress("nTrueInt", &nTrueInt);
          inputtree->SetBranchAddress("LepGood_mcMatchId", &LepGood_mcMatchId);

          inputtree->SetBranchAddress("nGenPart", &nGenPart);
          inputtree->SetBranchAddress("GenPart_motherId", &GenPart_motherId);
          inputtree->SetBranchAddress("GenPart_grandmotherId", &GenPart_grandmotherId);
          inputtree->SetBranchAddress("GenPart_sourceId", &GenPart_sourceId);
          inputtree->SetBranchAddress("GenPart_status", &GenPart_status);
          inputtree->SetBranchAddress("GenPart_pdgId", &GenPart_pdgId);
          inputtree->SetBranchAddress("GenPart_pt", &GenPart_pt);
          inputtree->SetBranchAddress("GenPart_eta", &GenPart_eta);
          inputtree->SetBranchAddress("GenPart_phi", &GenPart_phi);
        }

        // 2015 HLT
        /*Int_t HLT_PFMET170_JetIdCleaned;   inputtree->SetBranchAddress("HLT_PFMET170_JetIdCleaned", &HLT_PFMET170_JetIdCleaned);
        Int_t HLT_PFMET90_PFMHT90;   inputtree->SetBranchAddress("HLT_PFMET90_PFMHT90", &HLT_PFMET90_PFMHT90);
        Int_t HLT_PFMETNoMu90_PFMHTNoMu90;   inputtree->SetBranchAddress("HLT_PFMETNoMu90_PFMHTNoMu90", &HLT_PFMETNoMu90_PFMHTNoMu90);// */

        // 2016 HLT
        Int_t   HLT_PFMET90_PFMHT90_IDTight; inputtree->SetBranchAddress("HLT_PFMET90_PFMHT90_IDTight"  , &HLT_PFMET90_PFMHT90_IDTight  );
        Int_t HLT_PFMET100_PFMHT100_IDTight; inputtree->SetBranchAddress("HLT_PFMET100_PFMHT100_IDTight", &HLT_PFMET100_PFMHT100_IDTight);
        Int_t HLT_PFMET110_PFMHT110_IDTight; inputtree->SetBranchAddress("HLT_PFMET110_PFMHT110_IDTight", &HLT_PFMET110_PFMHT110_IDTight);
        Int_t HLT_PFMET120_PFMHT120_IDTight; inputtree->SetBranchAddress("HLT_PFMET120_PFMHT120_IDTight", &HLT_PFMET120_PFMHT120_IDTight);
        Int_t HLT_Ele24_eta2p1_WPLoose_Gsf;  inputtree->SetBranchAddress("HLT_Ele24_eta2p1_WPLoose_Gsf" , &HLT_Ele24_eta2p1_WPLoose_Gsf );
        Int_t HLT_IsoMu24;                   inputtree->SetBranchAddress("HLT_IsoMu24"                  , &HLT_IsoMu24                  );
        Int_t HLT_Ele25_eta2p1_WPLoose_Gsf;  inputtree->SetBranchAddress("HLT_Ele25_eta2p1_WPLoose_Gsf" , &HLT_Ele25_eta2p1_WPLoose_Gsf );
        Int_t HLT_IsoMu27;                   inputtree->SetBranchAddress("HLT_IsoMu27"                  , &HLT_IsoMu27                  );

        Int_t Flag_METFilters; inputtree->SetBranchAddress("Flag_METFilters", &Flag_METFilters);
        Int_t Flag_HBHENoiseFilter; inputtree->SetBranchAddress("Flag_HBHENoiseFilter", &Flag_HBHENoiseFilter);
        Int_t Flag_HBHENoiseIsoFilter; inputtree->SetBranchAddress("Flag_HBHENoiseIsoFilter", &Flag_HBHENoiseIsoFilter);
        Int_t Flag_eeBadScFilter; inputtree->SetBranchAddress("Flag_eeBadScFilter", &Flag_eeBadScFilter);
        Int_t Flag_EcalDeadCellTriggerPrimitiveFilter; inputtree->SetBranchAddress("Flag_EcalDeadCellTriggerPrimitiveFilter", &Flag_EcalDeadCellTriggerPrimitiveFilter);
        Int_t Flag_goodVertices; inputtree->SetBranchAddress("Flag_goodVertices", &Flag_goodVertices);
        Int_t Flag_globalTightHalo2016Filter; inputtree->SetBranchAddress("Flag_globalTightHalo2016Filter", &Flag_globalTightHalo2016Filter);
        Int_t Flag_badMuonMoriond2017; inputtree->SetBranchAddress("Flag_badMuonMoriond2017", &Flag_badMuonMoriond2017);
        Int_t Flag_badCloneMuonMoriond2017; inputtree->SetBranchAddress("Flag_badCloneMuonMoriond2017", &Flag_badCloneMuonMoriond2017);
        Float_t Flag_badMuonFilter; inputtree->SetBranchAddress("Flag_badMuonFilter", &Flag_badMuonFilter);
        Float_t Flag_badChargedHadronFilter; inputtree->SetBranchAddress("Flag_badChargedHadronFilter", &Flag_badChargedHadronFilter);

        Int_t GenSusyMGravitino=0;
        Int_t GenSusyMStop=0;
        Int_t GenSusyMSbottom=0;
        Int_t GenSusyMNeutralino=0;
        if(!process.isdata())
        {
          inputtree->SetBranchAddress("GenSusyMGravitino", &GenSusyMGravitino);
          inputtree->SetBranchAddress("GenSusyMStop", &GenSusyMStop);
          inputtree->SetBranchAddress("GenSusyMSbottom", &GenSusyMSbottom);
          inputtree->SetBranchAddress("GenSusyMNeutralino", &GenSusyMNeutralino);
        }

        // Read the number of entries in the inputtree
        size_t nentries = static_cast<size_t>(inputtree->GetEntries());
        std::cout << "\t    The file has " << nentries << " events." << std::endl;
        std::cout << "\t    Progress Bar: " << std::flush;
        int statusPrint = 0;
        if(nentries < 20)
          statusPrint = 1;
        else
          statusPrint = nentries/20;
        for(size_t i = 0; i < nentries; i++)
        {
          // Uncomment this if you suspect the loop is not going through the events
          /*if(i%2 == 0)
            std::cout << "$\b" << std::flush;
          else
            std::cout << " \b" << std::flush;// */
          if((statusPrint == 1) || (i%statusPrint == 0 && i != 0))
            std::cout << "*" << std::flush;

          if(doSync && max_sync_count > 0 && sync_count >= max_sync_count)
            break;

          inputtree->GetEntry(i);
          nVert = nVert_i;
          nIsr_out = nIsr;
          if(!process.isdata())
            puWeight = puWeightDistrib->GetBinContent(puWeightDistrib->FindBin(nTrueInt));

          // Object ID
          std::vector<int> validJets;
          std::vector<int> validLeptons;
          std::vector<int> looseLeptons;
          std::vector<int> bjetList; // Same as validJets, but sorted by CSV value

          validJets.clear();
          validLeptons.clear();
          looseLeptons.clear();
          bjetList.clear();

          for(Int_t i = 0; i < nJetIn; ++i)
          {
            if(std::abs(Jet_eta[i]) < 2.4 && Jet_pt[i] > jetPtThreshold)
            {
              validJets.push_back(i);
              bjetList.push_back(i);
            }
          }
          std::sort(validJets.begin(), validJets.end(), [Jet_pt] (const int &left, const int &right) {
            return Jet_pt[left] > Jet_pt[right];
            });
          std::sort(bjetList.begin(), bjetList.end(), [Jet_btagCSV] (const int &left, const int &right) {
              return Jet_btagCSV[left] > Jet_btagCSV[right];
              });

          if(preemptiveDropEvents && validJets.size() > 0)
            if(Jet_pt[validJets[0]] < ISR_JET_PT) continue;
          if(preemptiveDropEvents && validJets.size() == 0)
            continue;

          nGoodMu = 0;
          nGoodEl = 0;
          nGoodMu_loose = 0;
          nGoodEl_loose = 0;
          for(int i = 0; i < nLepGood; ++i)
          {
            bool lPTETA = false;
            if(std::abs(LepGood_pdgId[i]) == 13) // If a muon
            {
              lPTETA = LepGood_pt[i] > 3.5;
              lPTETA = lPTETA && (std::abs(LepGood_eta[i]) < 2.4);
            }
            else // If an electron
            {
              lPTETA = LepGood_pt[i] > 5.0;
              lPTETA = lPTETA && (std::abs(LepGood_eta[i]) < 2.5);
              // Also veto the gap in the ECAL
              lPTETA = lPTETA && ( (std::abs(LepGood_eta[i]) > ECALGap_MaxEta)
                                || (std::abs(LepGood_eta[i]) < ECALGap_MinEta) );
            }

            bool lID       = (std::abs(LepGood_dxy[i]) < 0.02)
                          && (std::abs(LepGood_dz[i]) < 0.1);
            bool lID_loose = (std::abs(LepGood_dxy[i]) < 0.1)
                          && (std::abs(LepGood_dz[i]) < 0.5);

            bool lIS       = LepGood_relIso03[i] < 0.2 || LepGood_absIso03[i] < 5.0;
            bool lIS_loose = LepGood_relIso03[i] < 0.8 || LepGood_absIso03[i] < 20.0;

            if(lPTETA && lID && lIS)
            {
              validLeptons.push_back(i);
              if(std::abs(LepGood_pdgId[i]) == 13)
                nGoodMu++;
              else
                nGoodEl++;
            }
            if(lPTETA && lID_loose && lIS_loose)
            {
              looseLeptons.push_back(i);
              if(std::abs(LepGood_pdgId[i]) == 13)
                nGoodMu_loose++;
              else
                nGoodEl_loose++;
            }
          }
          std::sort(validLeptons.begin(), validLeptons.end(), [&LepGood_pt] (const int &left, const int &right) {
            return LepGood_pt[left] > LepGood_pt[right];
            });
          std::sort(looseLeptons.begin(), looseLeptons.end(), [&LepGood_pt] (const int &left, const int &right) {
            return LepGood_pt[left] > LepGood_pt[right];
            });

          auto lepSel = [&LepGood_pt] (std::vector<int> &leptons) -> bool
          {
            if(leptons.size() > 0)
            {
              if(leptons.size() > 1)
              {
                if(LepGood_pt[leptons[1]] > SECOND_LEPTON_PT)
                  return false;
              }
              return true;
            }
            return false;
          };

          isTight = lepSel(validLeptons);

          isLoose = lepSel(looseLeptons);

          isLooseNotTight = isLoose && !isTight;
          if(isLooseNotTight)
          {
            validLeptons = looseLeptons;
            nGoodMu = nGoodMu_loose;
            nGoodEl = nGoodEl_loose;
          }

          if(preemptiveDropEvents && looseNotTight && !isLooseNotTight)
            continue;

          if(preemptiveDropEvents && !doLooseLeptons && isLooseNotTight)
            continue;

          if(preemptiveDropEvents && !(isLoose || isTight))
            continue;

          // Setting the values to be saved in the output tree
          mt_old = mtw;
          Met = met_pt;

          float lep_phi, lep_eta;
          if(validLeptons.size() > 0)
          {
            auto leptonIndex = validLeptons[0];
            lep_phi     = LepGood_phi[leptonIndex];
            lep_eta     = LepGood_eta[leptonIndex];
          }
          else
          {
            lep_phi     = -9999;
            lep_eta     = -9999;
          }

          if(validJets.size() > 1)
          {
            int jetIndex = validJets[1];
            Jet2Pt = Jet_pt[jetIndex];
            Jet2Eta = Jet_eta[jetIndex];
            Jet2CSV = Jet_btagCSV[jetIndex];

            double dphijj, detajj;
            dphijj = DeltaPhi(Jet_phi[validJets[0]], Jet_phi[jetIndex]);
            detajj = Jet_eta[validJets[0]] - Jet_eta[jetIndex];
            DPhiJet1Jet2 = dphijj;
            DrJet1Jet2 = std::sqrt( std::pow(dphijj,2) + std::pow(detajj,2) );

            if(validLeptons.size() > 0)
            {
              float dphi = DeltaPhi(Jet_phi[jetIndex], lep_phi);
              float deta = Jet_eta[jetIndex] - lep_eta;
              DrJet2Lep = std::sqrt( std::pow(dphi,2) + std::pow(deta,2) );
            }
            else
            {
              DrJet2Lep = -9999;
            }

            int bJetIndex = bjetList[1];
            JetB2pt = Jet_pt[bJetIndex];
            JetB2eta = Jet_eta[bJetIndex];
            JetB2CSV = Jet_btagCSV[bJetIndex];
            JetB2index = bJetIndex;
          }
          else
          {
            Jet2Pt = -9999;
            Jet2Eta = -9999;
            Jet2CSV = -9999;
            DPhiJet1Jet2 = -9999;
            DrJet1Jet2 = -9999;
            DrJet2Lep = -9999;

            JetB2pt = -9999;
            JetB2eta = -9999;
            JetB2CSV = -9999;
            JetB2index = -9999;
          }
          if(preemptiveDropEvents && !(DPhiJet1Jet2 < 2.5 || Jet2Pt < 60))
            continue;

          if(preemptiveDropEvents)
          {
            if(!swap && Met < MIN_MET)
              continue;
          }

          TLorentzVector VLep;
          if(validLeptons.size() > 0)
          {
            auto leptonIndex = validLeptons[0];
            mt          = LepGood_mt[leptonIndex];
            Q80         = LepGood_Q80[leptonIndex];
            CosDeltaPhi = LepGood_cosPhiLepMet[leptonIndex];
            LepChg      = LepGood_charge[leptonIndex];
            LepID       = LepGood_pdgId[leptonIndex];
            LepPt       = LepGood_pt[leptonIndex];
            LepEta      = LepGood_eta[leptonIndex];
            LepDxy      = LepGood_dxy[leptonIndex];
            LepDz       = LepGood_dz[leptonIndex];
            LepIso03    = LepGood_relIso03[leptonIndex];
            Float_t minPt = 25.;
            LepHybIso03 = LepIso03*std::min(LepPt, minPt);
            VLep.SetPtEtaPhiM(LepPt, LepEta, lep_phi, LepGood_mass[leptonIndex]);

            if(!process.isdata())
            {
              leptonIDSF = static_cast<double>(getLeptonIDSF(LepID, LepPt, LepEta));
              leptonISOSF = static_cast<double>(getLeptonISOSF(LepID, LepPt, LepEta));
            }

            if(!process.isdata() && doPromptTagging)
            {
              auto mcMatchId = LepGood_mcMatchId[leptonIndex];
              bool isPromptFlag = !(mcMatchId == 0 || mcMatchId == 99 || mcMatchId == 100);
              if(!isPromptFlag)
              {
                for(int genPartIndex = 0; genPartIndex < nGenPart; ++genPartIndex)
                {
                  if(std::abs(GenPart_pdgId[genPartIndex]) == 15)
                  {
                    if(std::abs(GenPart_motherId[genPartIndex]) == 24 || std::abs(GenPart_motherId[genPartIndex]) == 23 || (GenPart_motherId[genPartIndex] == -9999 && genPartIndex < 3 ))
                    {
                      double dphi = DeltaPhi(GenPart_phi[genPartIndex], lep_phi);
                      double deta = GenPart_eta[genPartIndex] - lep_eta;
                      double dr = std::sqrt(std::pow(dphi,2) + std::pow(deta,2));

                      if(dr < 0.15)
                      {
                        isPromptFlag = true;
                        break;
                      }
                    }
                  }
                }
              }

              isPrompt    = isPromptFlag?1:0;
            }
            else
            {
              isPrompt = 1;
            }
          }
          else
          {
            mt          = -9999;
            Q80         =  9999; // Q80 has a range of [-Inf, 1], so it does not make sense to give it a negative value when not defined
            CosDeltaPhi = -9999;
            LepChg      = -9999;
            LepID       = -9999;
            LepPt       = -9999;
            LepEta      = -9999;
            LepDxy      = -9999;
            LepDz       = -9999;
            LepIso03    = -9999;
            LepHybIso03 = -9999;
            isPrompt    = -9999;
          }
          if(validLeptons.size() > 1)
          {
            auto leptonIndex = validLeptons[1];
            Lep2Chg      = LepGood_charge[leptonIndex];
            Lep2ID       = LepGood_pdgId[leptonIndex];
            Lep2Pt       = LepGood_pt[leptonIndex];
            Lep2Eta      = LepGood_eta[leptonIndex];
            Lep2Dxy      = LepGood_dxy[leptonIndex];
            Lep2Dz       = LepGood_dz[leptonIndex];
            Lep2Iso03    = LepGood_relIso03[leptonIndex];
          }
          else
          {
            Lep2Chg      = -9999;
            Lep2ID       = -9999;
            Lep2Pt       = -9999;
            Lep2Eta      = -9999;
            Lep2Dxy      = -9999;
            Lep2Dz       = -9999;
            Lep2Iso03    = -9999;
          }

          if(!process.isdata())
          {
            triggerEfficiency = static_cast<double>(triggerEfficiencyFromMET(met_pt));
            // For EWK ISR, assume syst 100%
            if(process.tag() == "WJets")
              EWKISRweight = EWKISRCParam * static_cast<double>(EWKISRweightFromISRpT(LepPt, lep_phi, met_pt, met_phi));
            // For ISR, assume syst 50%
            if(process.tag() == "ttbar" || process.tag() == "ttbar_lep" || process.tag() == "ttbar_lo" || process.issignal())
              ISRweight = ISRCParam * static_cast<double>(ISRweightFromNISRJet(nIsr));
            // TODO: missing tt_pow reweighting https://twiki.cern.ch/twiki/bin/viewauth/CMS/SUSRecommendationsMoriond17
          }

          if(validLeptons.size() > 0 && validJets.size() > 0)
          {
            double smallestDeltaR = 999999999.;
            int closestJet = -1;

            for(auto &jet : validJets)
            {
              double dphi = DeltaPhi(Jet_phi[jet], lep_phi);
              double deta = Jet_eta[jet] - lep_eta;
              double dr = std::pow(dphi,2) + std::pow(deta,2); // The square of the dr is used since it is injective
              if(dr < smallestDeltaR)
              {
                smallestDeltaR = dr;
                closestJet = jet;
              }
            }

            TLorentzVector VJ, JLep;
            VJ.SetPtEtaPhiM(Jet_pt[closestJet], Jet_eta[closestJet], Jet_phi[closestJet], Jet_mass[closestJet]);
            JLep = VJ + VLep;
            JetLepMass = JLep.M();

            TLorentzVector VJ3;
            for(auto &jet : validJets)
            {
              if(jet == closestJet)
                continue;
              TLorentzVector VJi;
              VJi.SetPtEtaPhiM(Jet_pt[jet], Jet_eta[jet], Jet_phi[jet], Jet_mass[jet]);
              VJ3 += VJi;
            }
            J3Mass = VJ3.M();

            double dphi = DeltaPhi(Jet_phi[validJets[0]], lep_phi);
            double deta = Jet_eta[validJets[0]] - lep_eta;
            DrJet1Lep = std::sqrt( std::pow(dphi, 2) + std::pow(deta, 2) );

            double dphib, detab;
            int bJetIndex = bjetList[0];
            dphib = DeltaPhi(Jet_phi[bJetIndex], lep_phi);
            detab = Jet_eta[bJetIndex] - lep_eta;
            DrJetHBLep = std::sqrt( std::pow(dphib, 2) + std::pow(detab, 2) );
          }
          else
          {
            JetLepMass = -9999;
            J3Mass = -9999;
            DrJet1Lep = -9999;
            DrJetHBLep = -9999.;
          }

          if(validJets.size() > 0)
          {
            int jetIndex = validJets[0];
            Jet1Pt = Jet_pt[jetIndex];
            Jet1Eta = Jet_eta[jetIndex];
            Jet1CSV = Jet_btagCSV[jetIndex];

            int bJetIndex = bjetList[0];
            JetHBpt = Jet_pt[bJetIndex];
            JetHBeta = Jet_eta[bJetIndex];
            JetHBCSV = Jet_btagCSV[bJetIndex];
            JetHBindex = bJetIndex;
          }
          else
          {
            Jet1Pt = -9999;
            Jet1Eta = -9999;
            Jet1CSV = -9999;

            JetHBpt = -9999;
            JetHBeta = -9999;
            JetHBCSV = -9999;
            JetHBindex = -9999;
          }

          if(validJets.size() > 2)
          {
            int jetIndex = validJets[2];
            Jet3Pt = Jet_pt[jetIndex];
            Jet3Eta = Jet_eta[jetIndex];
            Jet3CSV = Jet_btagCSV[jetIndex];
          }
          else
          {
            Jet3Pt = -9999;
            Jet3Eta = -9999;
            Jet3CSV = -9999;
          }

          NbLoose = 0;
          NbTight = 0;
          NbLooseTo50 = 0;
          NbLoose50 = 0;
          NbMediumTo50 = 0;
          NbMedium50 = 0;
          NbTightTo50 = 0;
          NbTight50 = 0;
          for(auto &bjet : bjetList)
          {
            const auto &csv = Jet_btagCSV[bjet];
            const auto &pt = Jet_pt[bjet];

            if(csv > CSV_Loose)
              ++NbLoose;
            if(csv > CSV_Tight)
              ++NbTight;

            if(pt > 50)
            {
              if(csv > CSV_Loose)
                ++NbLoose50;
              if(csv > CSV_Medium)
                ++NbMedium50;
              if(csv > CSV_Tight)
                ++NbTight50;
            }
            else
            {
              if(csv > CSV_Loose)
                ++NbLooseTo50;
              if(csv > CSV_Medium)
                ++NbMediumTo50;
              if(csv > CSV_Tight)
                ++NbTightTo50;
            }

            // Since the bjetList is sorted by CSV, as soon as it goes below the loose definition, there are no further bjets
            if(csv < CSV_Loose)
              break;
          }

          HT = 0;
          Njet = validJets.size();
          Njet30 = 0;
          Njet40 = 0;
          Njet50 = 0;
          Njet60 = 0;
          Njet70 = 0;
          Njet80 = 0;
          Njet90 = 0;
          Njet100 = 0;
          for(auto &jet : validJets)
          {
            HT += Jet_pt[jet];

            if(Jet_pt[jet] > 30)
              ++Njet30;
            if(Jet_pt[jet] > 40)
              ++Njet40;
            if(Jet_pt[jet] > 50)
              ++Njet50;
            if(Jet_pt[jet] > 60)
              ++Njet60;
            if(Jet_pt[jet] > 70)
              ++Njet70;
            if(Jet_pt[jet] > 80)
              ++Njet80;
            if(Jet_pt[jet] > 90)
              ++Njet90;
            if(Jet_pt[jet] > 100)
              ++Njet100;
          }

          genGravitinoM  = GenSusyMGravitino;
          genStopM       = GenSusyMStop;
          genSbottomM    = GenSusyMSbottom;
          genNeutralinoM = GenSusyMNeutralino;

          Run = run;
          Event = evt;
          LumiSec = lumi;
          if(overrideXSec)
            XS = sample.crossSection();
          else
            XS = xsec;
          if(process.issignal())
            XS = stopCrossSection(genStopM, genNeutralinoM).value();

          HLT_PFMET90_PFMHT90                 = HLT_PFMET90_PFMHT90_IDTight;
          HLT_PFMET100_PFMHT100               = HLT_PFMET100_PFMHT100_IDTight;
          HLT_PFMET110_PFMHT110               = HLT_PFMET110_PFMHT110_IDTight;
          HLT_PFMET120_PFMHT120               = HLT_PFMET120_PFMHT120_IDTight;
          //HLT_Ele                             = HLT_Ele24_eta2p1_WPLoose_Gsf;
          //HLT_Mu                              = HLT_IsoMu24;
          HLT_Ele                             = HLT_Ele25_eta2p1_WPLoose_Gsf;
          HLT_Mu                              = HLT_IsoMu27;
          METFilters                          = Flag_METFilters;
          HBHENoiseFilter                     = Flag_HBHENoiseFilter;
          HBHENoiseIsoFilter                  = Flag_HBHENoiseIsoFilter;
          eeBadScFilter                       = Flag_eeBadScFilter;
          EcalDeadCellTriggerPrimitiveFilter  = Flag_EcalDeadCellTriggerPrimitiveFilter;
          goodVertices                        = Flag_goodVertices;
          globalTightHalo2016Filter           = Flag_globalTightHalo2016Filter;
          badMuonMoriond2017                  = Flag_badMuonMoriond2017;
          badCloneMuonMoriond2017             = Flag_badCloneMuonMoriond2017;
          badMuonFilter                       = Flag_badMuonFilter;
          badChargedHadronFilter              = Flag_badChargedHadronFilter;

          // Filter Efficiency
          if(filterEfficiencyH != nullptr)
          {
            auto theBin = filterEfficiencyH->FindBin(genStopM, genNeutralinoM);
            filterEfficiency = filterEfficiencyH->GetBinContent(theBin);
          }
          else
            filterEfficiency = 1.0;

          if(swap)
          {
            auto tmp = Met;
            Met = LepPt;
            LepPt = tmp;
            triggerEfficiency = 1;
          }

          if(!process.isdata())
            weight = puWeight*XS*filterEfficiency*(genWeight/sumGenWeight)*triggerEfficiency*EWKISRweight*ISRweight*leptonIDSF*leptonISOSF;
          else
            weight = 1;

          if(isLooseNotTight)
          {
            double efficiency = static_cast<double>(getLeptonTightLooseRatio(LepID, LepPt, LepEta));
            looseNotTightWeight = efficiency/(1-efficiency);
            weight *= looseNotTightWeight;
          }
          else
          {
            looseNotTightWeight = 1;
          }

          //if(doSync)
          if(doSync && HT > 200 && Jet1Pt > 100 && Met > 200)
          {
            if(sync_count < max_sync_count)
            {
              SyFile << "Run:LS:Ev " << run << ":" << lumi << ":" << evt << std::endl;
              SyFile << "   Mstop: " << genStopM << "; Mlsp: " << genNeutralinoM << std::endl;
              SyFile << "   HT: " << HT << "; MET: " << Met << std::endl;
              SyFile << "   Njet: " << Njet << std::endl;
              for(size_t i = 0; i < validJets.size(); ++i)
              {
                auto jet = validJets[i];
                SyFile << "   jet " << i+1 << ":  pT: " << Jet_pt[jet] << "; eta: " << Jet_eta[jet] << "; raw pT: " << Jet_rawPt[jet] << "; ID: " << Jet_id[jet] << "; abs(eta): " << std::abs(Jet_eta[jet]) << std::endl;
              }
              SyFile << "   Nlep: " << validLeptons.size() << " ( e - " << nGoodEl << "; mu - " << nGoodMu << ")" << std::endl;
              SyFile << "   leading lepton:  pT: " << LepPt << "; eta: " << LepEta << "; phi: " << lep_phi << "; PDG ID: " << LepID << "; RelIso: " << LepIso03 << "; dxy: " << LepDxy << "; dz: " << LepDz << std::endl;
              SyFile << "   Delta Phi Jet1 Jet2: " << DPhiJet1Jet2 << std::endl;
              SyFile << "   weight for 10 fb-1: " << weight*10000 << " (without SFs: " << 10000*XS*filterEfficiency*(genWeight/sumGenWeight) << ")" << std::endl;
              SyFile << "   passed: ";
              if(HT > 200 && Met > 200 && Jet1Pt > 90) // TODO: Probably change these cuts to reflect newest selection
              {
                SyFile << "Cut0";
                bool passCut1 = false;
                if(validLeptons.size() == 1)
                  passCut1 = true;
                if(validLeptons.size() == 2)
                {
                  if(LepGood_pt[validLeptons[1]] < SECOND_LEPTON_PT)
                    passCut1 = true;
                }

                if(passCut1 && LepPt < 30)
                {
                  SyFile << ";Cut1";

                  if(Jet1Pt > ISR_JET_PT)
                  {
                    SyFile << ";Cut2";

                    if(DPhiJet1Jet2 < 2.5 || Jet2Pt < 60)
                    {
                      SyFile << ";Cut3";

                      if(Met > 280)
                        SyFile << ";Cut4";
                    }
                  }
                }
              }
              else
                SyFile << "None";

              //Printout detailed info (for a specific event for instance)
              //if(lumi == 91151 && evt == 195485531)
              if(true)
              {
                SyFile << std::endl << "nLepGood: " << nLepGood << std::endl;
                SyFile << "LepGood:" << std::endl;
                for(int i = 0; i < nLepGood; ++i)
                  SyFile << "   lep " << i+1 << ": ID: " << LepGood_pdgId[i] << "; pt: " << LepGood_pt[i] << "; eta:" << LepGood_eta[i] << "; phi: " << LepGood_phi[i] << "; relIso03: " << LepGood_relIso03[i] << "; pdgID: " << LepGood_pdgId[i] << "; dxy: " << LepGood_dxy[i] << "; dz: " << LepGood_dz[i] << std::endl;
                SyFile << "nJetGood: " << nJetIn << std::endl;
                for(int i = 0; i < nJetIn; ++i)
                  SyFile << "   jet " << i+1 << ": pt: " << Jet_pt[i] << "; eta: " << Jet_eta[i] << "; phi: " << Jet_phi[i] << "; mass: " << Jet_mass[i] << "; ID: " << Jet_id[i] << std::endl;
              }

              SyFile << std::endl << std::endl;

              ++sync_count;
            }
          }

          // Compute selection cuts
          bool metRequirement = Met > 300;
          bool htRequirement = HT > 200;
          bool jetRequirement = Jet1Pt > ISR_JET_PT;
          bool antiQCDRequirement = DPhiJet1Jet2 < 2.5 || Jet2Pt < 60;
          bool leptonRequirement = isTight;
          bool deltaMRequirement = LepPt < 30;

          // Fill counters for parallel selection computation
          ++Ncut0;
          if(metRequirement)
          {
            ++Ncut1;
            if(htRequirement)
            {
              ++Ncut2;
              if(jetRequirement)
              {
                ++Ncut3;
                if(antiQCDRequirement)
                {
                  ++Ncut4;
                  if(leptonRequirement)
                  {
                    ++Ncut5;
                    if(deltaMRequirement)
                    {
                      ++Ncut6;
                    }
                  }
                }
              }
            }
          }

          if(!noSkim)
          {
            // If we are doing the loose not tight category, reject events that do not fit the category
            if(looseNotTight && !isLooseNotTight)
              continue;

            // Special check in case we are swapping MET and LepPt
            if(swap && LepPt < 5)
              continue;

            // Only keep events whose leptons pass our requirements
            if(looseNotTight && !isLooseNotTight)
              continue;

            if(!doLooseLeptons && isLooseNotTight)
              continue;

            if(!(isLoose || isTight))
              continue;

            // No need to keep events without jets
            if(validJets.size() == 0)
              continue;

            // So-called ISR jet requirement (even though it's on the pT of the leading jet)
            if(Jet1Pt < ISR_JET_PT)
              continue;

            // Cut to help reduce the QCD background
            if(!(DPhiJet1Jet2 < 2.5 || Jet2Pt < 60))
              continue;

            // Minimal requirement on MET
            if(Met < MIN_MET)
              continue;

            // MET filters
            //if ( METFilters                         != 1 ) continue;
            if ( HBHENoiseFilter                    != 1 ) continue;
            if ( HBHENoiseIsoFilter                 != 1 ) continue;
            if ( EcalDeadCellTriggerPrimitiveFilter != 1 ) continue;
            if ( goodVertices                       != 1 ) continue;
            if ( eeBadScFilter                      != 1 ) continue;
            if ( globalTightHalo2016Filter          != 1 ) continue;
            //if ( badMuonFilter                      != 1 ) continue; // Should probably only use 1 of these two
            if ( badMuonMoriond2017                 != 1 ) continue;
            if ( badCloneMuonMoriond2017            != 1 ) continue;
            if ( badChargedHadronFilter             != 1 ) continue;

            bool passHLT = false;
            if(!swap) // Normally use the MET triggers, but only for data
            {
              if(process.isdata())
              {
                if(HLT_PFMET90_PFMHT90_IDTight != 0)
                  passHLT = true;
                if(HLT_PFMET100_PFMHT100_IDTight != 0)
                  passHLT = true;
                if(HLT_PFMET110_PFMHT110_IDTight != 0)
                  passHLT = true;
                if(HLT_PFMET120_PFMHT120_IDTight != 0)
                  passHLT = true;
              }
              else
                passHLT = true;
            }
            else // If swapping MET and LepPt, apply single lepton triggers to data and MC, for data avoid double counting
            {
              if(HLT_IsoMu27 != 0)
                passHLT = true;
              if(HLT_Ele25_eta2p1_WPLoose_Gsf != 0)
                passHLT = true;

              // Remove double counting by removing from the electron PD the events with the muon HLT
              if(sample.tag().find("SingleEl") != std::string::npos && process.isdata())
              {
                if(HLT_IsoMu27 != 0)
                  passHLT = false;
              }
            }

            if(!passHLT)
              continue;
          }

          bdttree->Fill();
        }
        std::cout << std::endl;
        if(process.selection() != "")
          delete inputtree;
      }
      foutput.cd();
      bdttree->Write("",TObject::kOverwrite);

      TVectorD v(7);
      v[0] = Ncut0;
      v[1] = Ncut1;
      v[2] = Ncut2;
      v[3] = Ncut3;
      v[4] = Ncut4;
      v[5] = Ncut5;
      v[6] = Ncut6;
      v.Write("ParellelSelection");
      //puDistrib.Write();

      if(filterEfficiencyH != nullptr)
        delete filterEfficiencyH;
    }

    delete puWeightDistrib;
  }

  std::cout << "Done!" << std::endl << std::endl;

  return 0;
}

void printHelp()
{
  std::cout << "The program has the following parameters:" << std::endl;
  std::cout << "\t--help\t- Print this message" << std::endl;
  std::cout << "\t--json\t- Used to specify the json file which describes the sample, this option is obligatory. The paraeter immediately after this one should be the desired json file" << std::endl;
  std::cout << "\t--outDir\t- Used to specify the output directory. The parameter immediately after this one should be the desired directory. By default the directory './OUT/' is used" << std::endl;
  std::cout << "\t--doSync\t- Whether a txt file with the event output should be created for synchronization purposes" << std::endl;
  std::cout << "\t--suffix\t- Suffix to add at the end of the name of the output files." << std::endl;

  return;
}
