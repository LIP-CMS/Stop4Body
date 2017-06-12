#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TChain.h>
#include <TCut.h>
#include <TString.h>
#include <TObject.h>
#include <TRandom3.h>
#include <TDirectory.h>
#include <TStopwatch.h>

#include "TMVA/Factory.h"
#include "TMVA/Tools.h"
#include "TMVA/Reader.h"

#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include <fstream>

#include "UserCode/Stop4Body/interface/json.hpp"
#include "UserCode/Stop4Body/interface/SampleReader.h"

//#include "FWCore/PythonParameterSet/interface/MakeParameterSets.h"
//#include "FWCore/ParameterSet/interface/ParameterSet.h"

using json = nlohmann::json;

struct FileInfo
{
  std::string path;
  double crossSection;
  double branchingRatio;
  std::string tag;
};

void printHelp();
bool fileExists(std::string);
bool replace(std::string&, const std::string&, const std::string&);

int main(int argc, char** argv)
{
  std::string jsonFileName = "";
  std::string inputDirectory = "";
  std::string outputDirectory = "./OUT/";
  std::vector<std::string> methods;
  methods.clear();
  std::string suffix = "";
  std::string MVAsuffix = "";

  // Default MVA methods to be trained + tested
  std::map<std::string,int> Use;
  // --- Boosted Decision Trees
  Use["BDT"]             = 0; // uses Adaptive Boost
  Use["BDTG"]            = 0; // uses Gradient Boost
  Use["BDTB"]            = 0; // uses Bagging
  Use["BDTD"]            = 0; // decorrelation + Adaptive Boost
  Use["BDTF"]            = 0; // allow usage of fisher discriminant for node splitting
  // --- Friedman's RuleFit method, ie, an optimised series of cuts ("rules")
  Use["RuleFit"]         = 0;

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

    if(argument == "--inDir")
      inputDirectory = argv[++i];

    if(argument == "--method")
      methods.push_back(argv[++i]);

    if(argument == "--suffix")
      suffix = argv[++i];

    if(argument == "--mvaSuffix")
      MVAsuffix = argv[++i];
  }

  if(jsonFileName == "")
  {
    std::cout << "You must define a json file" << std::endl;
    return 1;
  }

  if(inputDirectory == "")
  {
    std::cout << "You must define an input directory" << std::endl;
    return 1;
  }

  for(auto & method : methods)
  {
    if(Use.count(method) != 1)
    {
      std::cout << "Unknown method \"" << method << "\"." << std::endl;
      std::cout << "Ignoring it" << std::endl;
      //std::cout << "Trusting the user and adding it" << std::endl;
      //Use[method] = 1;
    }
    else
      Use[method] = 1;
  }

  std::cout << "Reading JSON file" << std::endl;
  SampleReader samples(jsonFileName, inputDirectory, suffix);

  std::vector<std::string> filesToProcess = samples.getAllFiles();

  std::cout << "Processing " << filesToProcess.size() << " files." << std::endl;

  // Load TMVA
  TMVA::Tools::Instance();

  std::cout << std::endl;
  std::cout << "==> Start TMVAClassificationApplication" << std::endl;

  TMVA::Reader *reader = new TMVA::Reader( "!Color:!Silent" );

  Float_t LepPt, LepEta, LepDxy, LepDz, Met, mt, Q80, CosDeltaPhi;
  Float_t Jet1Pt,Jet1Eta,Jet2Pt,Jet2Eta,JetHBpt,DrJet1Lep,DrJet2Lep,DrJetHBLep,DrJet1Jet2,JetLepMass;
  Float_t J3Mass,XS,HT;
  Float_t Njet, NbLoose, NbTight, LepID, LepChg, Nevt, Event;
  Float_t JetHBCSV;
  //Int_t nGoodMu, nGoodEl;
  //Float_t LepSip3, LepIso03, LepIso04, DPhiJet1Jet2,

  // SET0
  reader->AddVariable("Jet1Pt", &Jet1Pt);
  reader->AddVariable("mt", &mt);
  reader->AddVariable("Met", &Met);
  reader->AddVariable("LepChg", &LepChg);
  reader->AddVariable("LepEta", &LepEta);
  reader->AddVariable("LepPt", &LepPt);
  reader->AddVariable("HT", &HT);
  reader->AddVariable("NbLoose", &NbLoose);
  reader->AddVariable("Njet", &Njet);
  reader->AddVariable("JetHBpt", &JetHBpt);
  reader->AddVariable("DrJetHBLep",&DrJetHBLep);
  // SET1
  // SET2
  reader->AddVariable("JetHBCSV",&JetHBCSV);
  // SET3
  //reader->AddVariable("JetLepMass",&JetLepMass);
  //reader->AddVariable("J3Mass",&J3Mass);

  //reader->AddVariable("NbLooseTo50",&NbLooseTo50);
  //reader->AddVariable("NbLoose50",&NbLoose50);
  //reader->AddVariable("NbMediumTo50",&NbMediumTo50);
  //reader->AddVariable("NbMedium50",&NbMedium50);
  //reader->AddVariable("NbTightTo50",&NbTightTo50);
  //reader->AddVariable("NbTight50",&NbTight50);
  //reader->AddVariable("nGoodTrack", &nGoodTrack);
  //reader->AddVariable("Jet1Eta", &Jet1Eta);
  //reader->AddVariable("Jet2Pt", &Jet2Pt);
  //reader->AddVariable("DrJet1Lep", &DrJet1Lep);
  //reader->AddVariable("Q80", &Q80);
  //reader->AddVariable("CosDeltaPhi", &CosDeltaPhi);
  //reader->AddVariable("DrJet1Jet2",&DrJet1Jet2);
  //reader->AddVariable("Jet1CSV", &Jet1CSV);
  //reader->AddVariable("Jet2CSV", &Jet2CSV);
  //reader->AddVariable("Jet3CSV", &Jet3CSV);

  TString dir    = "weights/";
  TString prefix = "TMVAClassification";

  for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++)
  {
    if (it->second)
    {
      TString methodName = TString(it->first) + TString(" method");
      std::cout << "method being used " << methodName << std::endl;
      TString weightfile = dir + prefix + TString("_") + TString(it->first);
      if(MVAsuffix != "")
        weightfile += TString("_"+MVAsuffix);
      weightfile += TString(".weights.xml");
      std::cout << "weight file being used : "    <<  weightfile << std::endl;
      reader->BookMVA( methodName, weightfile );
    }
  }

  for(auto & file : filesToProcess)
  {
    auto cwd = gDirectory;

    std::string outFile = file.substr(0, file.find_last_of("."))+"_bdt.root";
    replace(outFile, inputDirectory, outputDirectory);
    std::cout << "Processing file " << file << " and saving output in " << outFile << std::endl;

    TFile *inputFile = new TFile(file.c_str(), "READ");
    TFile *outputFile = new TFile(outFile.c_str(), "RECREATE");

    TTree* OutputTree = static_cast<TTree*>(inputFile->Get("bdttree"))->CloneTree();

    OutputTree->SetBranchAddress("LepID", &LepID);
    OutputTree->SetBranchAddress("LepChg", &LepChg);
    OutputTree->SetBranchAddress("LepPt", &LepPt);
    OutputTree->SetBranchAddress("LepEta", &LepEta);
    OutputTree->SetBranchAddress("LepDxy", &LepDxy);
    OutputTree->SetBranchAddress("LepDz", &LepDz);
    //OutputTree->SetBranchAddress("LepIso", &LepIso);
    OutputTree->SetBranchAddress("Met", &Met);
    OutputTree->SetBranchAddress("mt",  &mt);
    OutputTree->SetBranchAddress("Q80", &Q80);
    OutputTree->SetBranchAddress("CosDeltaPhi",&CosDeltaPhi);
    OutputTree->SetBranchAddress("NbLoose", &NbLoose);
    OutputTree->SetBranchAddress("NbTight", &NbTight);
    OutputTree->SetBranchAddress("Njet", &Njet);
    OutputTree->SetBranchAddress("Jet1Pt", &Jet1Pt);
    OutputTree->SetBranchAddress("Jet1Eta", &Jet1Eta);
    OutputTree->SetBranchAddress("Jet2Pt", &Jet2Pt);
    OutputTree->SetBranchAddress("Jet2Eta", &Jet2Eta);
    OutputTree->SetBranchAddress("JetHBpt", &JetHBpt);
    OutputTree->SetBranchAddress("JetHBCSV", &JetHBCSV);
    OutputTree->SetBranchAddress("DrJet1Lep", &DrJet1Lep);
    OutputTree->SetBranchAddress("DrJet2Lep", &DrJet2Lep);
    OutputTree->SetBranchAddress("DrJetHBLep",&DrJetHBLep);
    OutputTree->SetBranchAddress("DrJet1Jet2",&DrJet1Jet2);
    OutputTree->SetBranchAddress("JetLepMass",&JetLepMass);
    OutputTree->SetBranchAddress("J3Mass",&J3Mass);
    OutputTree->SetBranchAddress("HT",&HT);
    OutputTree->SetBranchAddress("XS", &XS);
    OutputTree->SetBranchAddress("Event", &Event);
    OutputTree->SetBranchAddress("Nevt", &Nevt);

    std::map<std::string, Float_t> MVAs;
    for(auto & mva : Use)
      if(mva.second)
        MVAs[mva.first] = 0;

    std::map<std::string, TBranch* > MVABranches;
    for(auto & mva : MVAs)
    {
      std::cout << "Creating branch for \"" << mva.first << "\"" << std::endl;
      MVABranches[mva.first] = OutputTree->Branch(mva.first.c_str(), &mva.second);
    }

    std::cout << "--- Processing: " << OutputTree->GetEntries() << " events" << std::endl;
    TStopwatch sw;
    sw.Start();

    for (Long64_t ievt=0; ievt < OutputTree->GetEntries(); ievt++)
    {
      if (ievt%1000 == 0) std::cout << "--- ... Processing event: " << ievt << std::endl;

      OutputTree->GetEntry(ievt);
      for(auto & mva : MVAs)
      {
        //if (ievt%1000 == 0) std::cout << "Evaluating method: \"" << mva.first << "\"" << std::endl;
        mva.second = reader->EvaluateMVA((mva.first + " method").c_str());
        MVABranches[mva.first]->Fill();
      }
    }

    sw.Stop();
    std::cout << "--- End of event loop: ";
    sw.Print();

    OutputTree->Write();
    //outputFile->Write();
    cwd->cd();
    outputFile->Close();
    inputFile->Close();
  }




  /*if(emptyLines.size() != 0)
  {
    std::cout << "The following lines did not have any files associated to them: ";
    for(auto &process : emptyLines)
      std::cout << process << "; ";
    std::cout << std::endl;
  }

  if(emptyPath.size() != 0)
  {
    std::cout << "The following files did not have any path defined, or the defined path was empty: ";
    for(auto &file : emptyPath)
      std::cout << file << "; ";
    std::cout << std::endl;
  }

  if(invalidPath.size() != 0)
  {
    std::cout << "The following files could not be found: " << std::endl;
    for(auto &file : invalidPath)
      std::cout << "\t" << file << std::endl;
    std::cout << std::endl;
  }*/

  return 0;
}

void printHelp()
{
  std::cout << "The program has the following parameters:" << std::endl;
  std::cout << "\t--help\t- Print this message" << std::endl;

  return;
}

bool replace(std::string& str, const std::string& from, const std::string& to)
{
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

bool fileExists(std::string fileName)
{
  std::ifstream infile(fileName);
  return infile.good();
}
