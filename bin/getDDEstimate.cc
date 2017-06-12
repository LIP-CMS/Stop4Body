#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include <fstream>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TStyle.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TPad.h>
#include <TLegend.h>
#include <TPaveText.h>
#include <TGraphErrors.h>

#include "UserCode/Stop4Body/interface/json.hpp"
#include "UserCode/Stop4Body/interface/SampleReader.h"
#include "UserCode/Stop4Body/interface/doubleWithUncertainty.h"

using json = nlohmann::json;

void printHelp();
doubleUnc naiveDD(std::ofstream &, ProcessInfo &, SampleReader &, SampleReader &, std::string, std::string, std::string);
doubleUnc promptDD(std::ofstream &, ProcessInfo &, SampleReader &, SampleReader &, std::string, std::string, std::string);
doubleUnc injectDD(std::ofstream &, ProcessInfo &, ProcessInfo &, SampleReader &, SampleReader &, std::string, std::string, std::string, std::string);
doubleUnc fakeDD(std::ofstream &, SampleReader &, SampleReader &, std::string, std::string);
doubleUnc fullDD(std::ofstream &, ProcessInfo &, SampleReader &, SampleReader &, SampleReader &, SampleReader &, std::string, std::string, std::string);

int main(int argc, char** argv)
{
  std::string jsonFileName = "";
  std::string inputDirectory = "";
  std::string outputDirectory = "./OUT/";
  std::string suffix = "";
  double luminosity = -1.0;
  bool isPseudoData = false;
  bool verbose = false;
  std::string looseNotTightDirectory = "";

  //std::string baseSelection = "(HT > 200) && (Jet1Pt > 110) && (Met > 280) && (LepPt < 30.) && (HLT_Mu == 1)";
  std::string baseSelection = "(HT > 200) && (Jet1Pt > 110) && (Met > 280) && (LepPt < 30.)";
  //std::string baseSelection = "(HT > 200) && (Jet1Pt > 110) && (Met > 280) && (LepPt < 280.) && (HLT_Mu == 1)";
  //std::string baseSelection = "(HT > 200) && (Jet1Pt > 110) && (Met > 280)";
  std::string wjetsControlRegion = "(BDT < 0.2) && (NbLoose == 0)";
  std::string ttbarControlRegion = "(BDT < 0.2) && (NbTight > 0)";
  std::string signalRegion       = "(BDT > 0.44)";
  std::string wjetsSignalRegionClosure = "(BDT > 0.44) && (NbLoose == 0)";
  std::string ttbarSignalRegionClosure = "(BDT > 0.44) && (NbTight > 0)";

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

    if(argument == "--suffix")
      suffix = argv[++i];

    if(argument == "--lumi")
    {
      std::stringstream convert;
      convert << argv[++i];
      convert >> luminosity;
    }

    if(argument == "--isPseudoData")
    {
      isPseudoData = true;
    }

    if(argument == "--verbose")
    {
      verbose = true;
    }

    if(argument == "--looseNotTight")
    {
      looseNotTightDirectory = argv[++i];
    }
  }

  gStyle->SetOptStat(000000);
  gStyle->SetOptTitle(0);

  std::cout << "Reading json files" << std::endl;
  SampleReader samples(jsonFileName, inputDirectory, suffix);

  if(verbose)
    std::cout << "Splitting samples according to type" << std::endl;
  auto MC = samples.getMCBkg();
  auto Sig = samples.getMCSig();
  auto Data = samples.getData();

  if(verbose)
    std::cout << "Extracting ttbar and wjets from other backgrounds" << std::endl;
  size_t ttbarIndex = 0, wjetsIndex = 0;
  bool foundTTbar = false, foundWJets = false;
  for(size_t i = 0; i < MC.nProcesses(); ++i)
  {
    if(MC.process(i).tag() == "WJets")
    {
      wjetsIndex = i;
      foundWJets = true;
    }

    if(MC.process(i).tag().find("ttbar") != std::string::npos)
    {
      ttbarIndex = i;
      foundTTbar = true;
    }
  }
  if(!foundTTbar)
  {
    std::cout << "There isn't a ttbar sample in the JSON file" << std::endl;
    return 1;
  }
  if(!foundWJets)
  {
    std::cout << "There isn't a wjets sample in the JSON file" << std::endl;
    return 1;
  }
  auto wjets = MC.process(wjetsIndex);
  auto ttbar = MC.process(ttbarIndex);

  if(verbose)
    std::cout << "Retrieving luminosity" << std::endl;
  if(luminosity <= 0)
    luminosity = Data.getLumi();

  if(verbose)
    std::cout << "Computing correct MC weight" << std::endl;
  std::string mcWeight;
  {
    std::stringstream converter;
    if(isPseudoData)
      converter << "splitFactor*XS*filterEfficiency*(genWeight/sumGenWeight)"; // The scale factors are not considered here
    else
      converter << "splitFactor*weight";
    converter << "*" << luminosity;
    converter >> mcWeight;
  }
  std::cout << "Using mcWeight: " << mcWeight << std::endl;


  // Make a plot of the BDToutput, just for control reasons
  if(verbose)
    std::cout << "Building control plot of BDT output" << std::endl;
  {
    auto dataH = Data.process(0).getHist("BDT", "BDT;Evt.",               baseSelection,     20, -1.0, 1.0);
    auto mcH   =        MC.getHist("MC", "BDT", "BDT;Evt.", mcWeight+"*("+baseSelection+")", 20, -1.0, 1.0);
    auto sigH  =  Sig.process(0).getHist("BDT", "BDT;Evt.", mcWeight+"*("+baseSelection+")", 20, -1.0, 1.0);
    auto mcS   =             MC.getStack("BDT", "BDT;Evt.", mcWeight+"*("+baseSelection+")", 20, -1.0, 1.0);

    auto ratio = static_cast<TH1D*>(dataH->Clone("Ratio"));
    ratio->SetTitle(";BDT;Data/#Sigma MC");
    ratio->Divide(mcH);

    TCanvas c1("BDT_canv", "", 800, 800);
    gStyle->SetOptStat(0);

    TPad* t1 = new TPad("t1","t1", 0.0, 0.20, 1.0, 1.0);
    TPad* t2 = new TPad("t2","t2", 0.0, 0.0, 1.0, 0.2);
    t1->Draw();
    t1->cd();
    t1->SetLogy(true);
    mcS->Draw("hist");
    dataH->Draw("same");
    sigH->Draw("hist same");

    TLegend *legA = gPad->BuildLegend(0.845,0.69,0.65,0.89, "NDC");
    legA->SetFillColor(0);
    legA->SetFillStyle(0);
    legA->SetLineColor(0);
    legA->SetHeader("");
    legA->SetTextFont(42);

    TPaveText* T = new TPaveText(0.1,0.995,0.84,0.95, "NDC");
    T->SetFillColor(0);
    T->SetFillStyle(0);
    T->SetLineColor(0);
    T->SetTextAlign(12);
    char Buffer[1024];
    sprintf(Buffer, "CMS preliminary, #sqrt{s}=%.1f TeV, #scale[0.5]{#int} L=%.1f fb^{-1}", 13.0, luminosity/1000);
    T->AddText(Buffer);
    T->Draw("same");
    T->SetBorderSize(0);

    c1.cd();
    t2->Draw();
    t2->cd();
    t2->SetGridy(true);
    t2->SetPad(0,0.0,1.0,0.2);
    t2->SetTopMargin(0);
    t2->SetBottomMargin(0.5);

    TH1D *bgUncH = static_cast<TH1D*>(mcH->Clone("BDT_bgUncH"));
    for(int xbin=1; xbin <= bgUncH->GetXaxis()->GetNbins(); xbin++)
    {
      if(bgUncH->GetBinContent(xbin)==0)
        continue;

      double unc = bgUncH->GetBinError(xbin) / bgUncH->GetBinContent(xbin);

      // Add systematic uncertainties
      unc = unc*unc;
      unc += 0.026*0.026; // Luminosity uncertainty
      unc = std::sqrt(unc);

      bgUncH->SetBinContent(xbin,1);
      bgUncH->SetBinError(xbin,unc);
    }

    TGraphErrors *bgUnc = new TGraphErrors(bgUncH);
    bgUnc->SetLineColor(1);
    bgUnc->SetFillStyle(3001);
    bgUnc->SetFillColor(kGray);
    bgUnc->SetMarkerColor(1);
    bgUnc->SetMarkerStyle(1);
    bgUncH->Reset("ICE");
    bgUncH->Draw();
    bgUnc->Draw("3");
    double yscale = (1.0-0.2)/(0.18-0);
    bgUncH->GetYaxis()->SetTitle("Data/#Sigma MC");
    bgUncH->SetMinimum(0.4);
    bgUncH->SetMaximum(1.6);
    bgUncH->GetXaxis()->SetTitle("");
    bgUncH->GetXaxis()->SetTitleOffset(1.3);
    bgUncH->GetXaxis()->SetLabelSize(0.033*yscale);
    bgUncH->GetXaxis()->SetTitleSize(0.036*yscale);
    bgUncH->GetXaxis()->SetTickLength(0.03*yscale);
    bgUncH->GetYaxis()->SetTitleOffset(0.3);
    bgUncH->GetYaxis()->SetNdivisions(5);
    bgUncH->GetYaxis()->SetLabelSize(0.033*yscale);
    bgUncH->GetYaxis()->SetTitleSize(0.036*yscale);
    ratio->Draw("same");

    c1.SaveAs((outputDirectory+"/BDTout.png").c_str());
    c1.SaveAs((outputDirectory+"/BDTout.C").c_str());

    delete dataH;
    delete mcH;
    delete sigH;
    // Delete individual hists in the stack
    TList* histList = mcS->GetHists();
    histList->Delete();
    delete mcS;
    delete ratio;
    delete legA;
    delete T;
    delete bgUncH;
    delete bgUnc;
  }

  if(verbose)
    std::cout << "Filling table" << std::endl;
  std::ofstream outputTable(outputDirectory+"/table.tex");
  outputTable << "\\begin{tabular}{r|ccccc}\n";
  outputTable << " & SR & CR & Data in CR & other MC in CR & Estimate\\\\\n\\hline\n";

  naiveDD(outputTable, wjets, Data, MC, baseSelection + " && " + signalRegion, baseSelection + " && " + wjetsControlRegion, mcWeight);
  naiveDD(outputTable, ttbar, Data, MC, baseSelection + " && " + signalRegion, baseSelection + " && " + ttbarControlRegion, mcWeight);

  outputTable << "\\hline\n";

  injectDD(outputTable, ttbar, wjets, Data, MC, baseSelection + " && " + signalRegion, baseSelection + " && " + ttbarControlRegion, baseSelection + " && " + wjetsControlRegion, mcWeight);

  outputTable << "\\hline\n";

  injectDD(outputTable, wjets, ttbar, Data, MC, baseSelection + " && " + signalRegion, baseSelection + " && " + wjetsControlRegion, baseSelection + " && " + ttbarControlRegion, mcWeight);

  //outputTable << "\\hline\n";

  //promptDD(outputTable, wjets, Data, MC, baseSelection + " && " + signalRegion, baseSelection + " && " + wjetsControlRegion, mcWeight);
  //promptDD(outputTable, ttbar, Data, MC, baseSelection + " && " + signalRegion, baseSelection + " && " + ttbarControlRegion, mcWeight);

  if(looseNotTightDirectory != "")
  {
    SampleReader LNTSamples(jsonFileName, looseNotTightDirectory, suffix);
    auto LNTMC   = LNTSamples.getMCBkg();
    //auto LNTSig  = LNTSamples.getMCSig();
    auto LNTData = LNTSamples.getData();

    outputTable << "\\hline\n";

    fakeDD(outputTable, LNTData, LNTMC, baseSelection + " && " + signalRegion, mcWeight);

    outputTable << "\\hline\n";

    fullDD(outputTable, wjets, Data, MC, LNTData, LNTMC, baseSelection + " && " + signalRegion, baseSelection + " && " + wjetsControlRegion, mcWeight);

    outputTable << "\\hline\n";

    fullDD(outputTable, ttbar, Data, MC, LNTData, LNTMC, baseSelection + " && " + signalRegion, baseSelection + " && " + ttbarControlRegion, mcWeight);
  }

  outputTable << "\\hline\n\\end{tabular}\n";

  if(verbose)
    std::cout << "Filling closure table" << std::endl;
  outputTable << "\n\nClosure for DD\n";

  outputTable << "\\begin{tabular}{r|ccccc}\n";
  outputTable << " & SR & CR & Data in CR & other MC in CR & Estimate\\\\\n\\hline\n";

  naiveDD(outputTable, wjets, Data, MC, baseSelection + " && " + wjetsSignalRegionClosure, baseSelection + " && " + wjetsControlRegion, mcWeight);
  naiveDD(outputTable, ttbar, Data, MC, baseSelection + " && " + wjetsSignalRegionClosure, baseSelection + " && " + ttbarControlRegion, mcWeight);

  outputTable << "\\hline\n";

  naiveDD(outputTable, wjets, Data, MC, baseSelection + " && " + ttbarSignalRegionClosure, baseSelection + " && " + wjetsControlRegion, mcWeight);
  naiveDD(outputTable, ttbar, Data, MC, baseSelection + " && " + ttbarSignalRegionClosure, baseSelection + " && " + ttbarControlRegion, mcWeight);

  outputTable << "\\hline\n\\end{tabular}\n";

  return 0;
}

doubleUnc naiveDD(std::ofstream &outputTable, ProcessInfo &toEstimate, SampleReader &Data, SampleReader &MC, std::string signalRegion, std::string controlRegion, std::string mcWeight)
{
  doubleUnc NinSR = toEstimate.getYield(signalRegion, mcWeight);
  doubleUnc NinCR = toEstimate.getYield(controlRegion, mcWeight);
  doubleUnc DatainCR = Data.getYield(controlRegion, "1.0");
  doubleUnc otherMC (0,0);
  if(static_cast<double>(NinSR) == 0)
    NinSR = doubleUnc(4,2);
  for(auto &process: MC)
  {
    if(process.tag() != toEstimate.tag())
      otherMC += process.getYield(controlRegion, mcWeight);
  }

  doubleUnc estimate = NinSR/NinCR * (DatainCR - otherMC);

  outputTable << "$" << toEstimate.label() << "$ & $";
  outputTable << NinSR << "$ & $";
  outputTable << NinCR << "$ & $";
  outputTable << DatainCR << "$ & $";
  outputTable << otherMC << "$ & $";
  outputTable << estimate << "$\\\\\n";

  return estimate;
}

doubleUnc injectDD(std::ofstream &outputTable, ProcessInfo &toEstimate, ProcessInfo &toInject, SampleReader &Data, SampleReader &MC, std::string signalRegion, std::string controlRegion, std::string injectRegion, std::string mcWeight)
{
  outputTable << "inject ";
  doubleUnc inject = naiveDD(outputTable, toInject, Data, MC, controlRegion, injectRegion, mcWeight);
  doubleUnc NinSR = toEstimate.getYield(signalRegion, mcWeight);
  doubleUnc NinCR = toEstimate.getYield(controlRegion, mcWeight);
  doubleUnc DatainCR = Data.getYield(controlRegion, "1.0");
  doubleUnc otherMC (0,0);
  if(static_cast<double>(NinSR) == 0)
    NinSR = doubleUnc(4,2);
  for(auto &process: MC)
  {
    if(process.tag() != toEstimate.tag() && process.tag() != toInject.tag())
      otherMC += process.getYield(controlRegion, mcWeight);
  }

  doubleUnc estimate = NinSR/NinCR * (DatainCR - inject - otherMC);

  outputTable << "$" << toEstimate.label() << "$, $" << toInject.label() << "$ injected & $";
  outputTable << NinSR << "$ & $";
  outputTable << NinCR << "$ & $";
  outputTable << DatainCR << "$ & $";
  outputTable << otherMC << "$ & $";
  outputTable << estimate << "$\\\\\n";

  return estimate;
}

doubleUnc promptDD(std::ofstream &outputTable, ProcessInfo &toEstimate, SampleReader &Data, SampleReader &MC, std::string signalRegion, std::string controlRegion, std::string mcWeight)
{
  doubleUnc NinSR = toEstimate.getYield(signalRegion + " && isPrompt == 1", mcWeight);
  doubleUnc NinCR = toEstimate.getYield(controlRegion, mcWeight);
  doubleUnc DatainCR = Data.getYield(controlRegion, "1.0");
  doubleUnc otherMC (0,0);
  if(static_cast<double>(NinSR) == 0)
    NinSR = doubleUnc(4,2);
  for(auto &process: MC)
  {
    if(process.tag() != toEstimate.tag())
      otherMC += process.getYield(controlRegion, mcWeight);
  }

  doubleUnc estimate = NinSR/NinCR * (DatainCR - otherMC);

  outputTable << "$" << toEstimate.label() << "$ & $";
  outputTable << NinSR << "$ & $";
  outputTable << NinCR << "$ & $";
  outputTable << DatainCR << "$ & $";
  outputTable << otherMC << "$ & $";
  outputTable << estimate << "$\\\\\n";

  return estimate;
}

doubleUnc fakeDD(std::ofstream &outputTable, SampleReader &LNTData, SampleReader &LNTMC, std::string signalRegion, std::string mcWeight)
{
  doubleUnc LNTinSR = LNTData.getYield(signalRegion, "weight");
  doubleUnc LNTMCinSR = LNTMC.getYield(signalRegion + " && isPrompt == 1", mcWeight);

  doubleUnc NinCR = LNTMC.getYield(signalRegion + " && isPrompt == 1", mcWeight+"/looseNotTightWeight");
  doubleUnc DatainCR = LNTData.getYield(signalRegion, "1.0");

  doubleUnc estimate = LNTinSR - LNTMCinSR;

  outputTable << "Fakes & - & $";
  outputTable << NinCR << "$ ($" << LNTMCinSR << "$) & $";
  outputTable << DatainCR << "$ ($" << LNTinSR << "$) & - & $";
  outputTable << estimate << "$\\\\\n";

  return estimate;
}

doubleUnc fullDD(std::ofstream &outputTable, ProcessInfo &toEstimate, SampleReader &Data, SampleReader &MC, SampleReader &LNTData, SampleReader &LNTMC, std::string signalRegion, std::string controlRegion, std::string mcWeight)
{
  doubleUnc NinSR = toEstimate.getYield(signalRegion + " && isPrompt == 1", mcWeight);
  doubleUnc NinCR = toEstimate.getYield(controlRegion + " && isPrompt == 1", mcWeight);
  doubleUnc DatainCR = Data.getYield(controlRegion, "1.0");
  doubleUnc otherMC (0,0);
  if(static_cast<double>(NinSR) == 0)
    NinSR = doubleUnc(4,2);
  for(auto &process: MC)
  {
    if(process.tag() != toEstimate.tag())
      otherMC += process.getYield(controlRegion + " && isPrompt == 1", mcWeight);
  }
  doubleUnc fakes = fakeDD(outputTable, LNTData, LNTMC, controlRegion, mcWeight);

  doubleUnc estimate = NinSR/NinCR * (DatainCR - otherMC - fakes);

  outputTable << "$" << toEstimate.label() << "$ & $";
  outputTable << NinSR << "$ & $";
  outputTable << NinCR << "$ & $";
  outputTable << DatainCR << "$ & $";
  outputTable << otherMC << " + " << fakes << "$(fake) & $";
  outputTable << estimate << "$\\\\\n";

  return estimate;
}

void printHelp()
{
  std::cout << "The program has the following parameters:" << std::endl;
  std::cout << "\t--help\t- Print this message" << std::endl;

  return;
}
