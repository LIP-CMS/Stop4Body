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
#include <TList.h>
#include <TCollection.h>
#include <TObject.h>

#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include <fstream>
#include <stdexcept>

#include "UserCode/Stop4Body/interface/json.hpp"
#include "UserCode/Stop4Body/interface/SampleReader.h"
#include "UserCode/Stop4Body/interface/VariableReader.h"

using json = nlohmann::json;

void printHelp();

class CutInfo
{
public:
  CutInfo(std::string name, std::string cut, std::string latex): name_(name), cut_(cut), latex_(latex) {}

  std::string name()  const {return name_;}
  std::string cut()   const {return cut_;}
  std::string latex() const {return latex_;}

private:
protected:
  std::string name_;
  std::string cut_;
  std::string latex_;
};

int main(int argc, char** argv)
{
  std::string jsonFileName = "";
  std::string inputDirectory = "";
  std::string outputDirectory = "./plots/";
  std::string suffix = "";
  std::string variablesJson = "";
  std::string cutsJson = "";
  double luminosity = -1.0;
  bool noPUweight = false;
  bool puTest = false;
  bool verbose = false;
  bool cumulativeCuts = false;
  bool rawEvents = false;
  bool noSF = false;
  bool unblind = false;
  bool ddfake = false;

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

    if(argument == "--variables")
      variablesJson = argv[++i];

    if(argument == "--cuts")
      cutsJson = argv[++i];

    if(argument == "--lumi")
    {
      std::stringstream convert;
      convert << argv[++i];
      convert >> luminosity;
    }

    if(argument == "--noPUweight")
    {
      noPUweight = true;
    }

    if(argument == "--puTest") // This option normalizes data and MC so we compare shapes
    {
      puTest = true;
    }

    if(argument == "--verbose")
    {
      verbose = true;
    }

    if(argument == "--cumulative")
    {
      cumulativeCuts = true;
    }

    if(argument == "--rawEvents") // Change the MC weight to 1 to get the raw number of events (does not affect if puTest is set)
    {
      rawEvents = true;
    }

    if(argument == "--noSF")
    {
      noSF = true;
    }

    if(argument == "--unblind")
    {
      unblind = true;
    }

    if(argument == "--doDDFake")
    {
      unblind = true;
      ddfake = true;
    }
  }

  if(jsonFileName == "")
  {
    std::cout << "You must define a json file" << std::endl;
    return 1;
  }

  if(variablesJson == "")
  {
    std::cout << "You must define a json file with the variables to plot" << std::endl;
    return 1;
  }

  if(inputDirectory == "")
  {
    std::cout << "You must define an input directory" << std::endl;
    return 1;
  }

  gStyle->SetOptStat(000000);
  gStyle->SetOptTitle(0);

  std::cout << "Reading json files" << std::endl;
  VariableJsonLoader variables(variablesJson);
  TwoDVariableJsonLoader twoDvariables(variablesJson);
  SampleReader samples(jsonFileName, inputDirectory, suffix);

  auto MC = samples.getMCBkg();
  auto Sig = samples.getMCSig();
  auto Data = samples.getData();

  if(luminosity <= 0)
    luminosity = Data.getLumi();

  std::string mcWeight;
  {
    std::stringstream converter;
    if(!noSF)
    {
      converter << "splitFactor*weight"; // Full
      //converter << "weight/(triggerEfficiency*WISRSF*ISRweight)"; // Incrementally adding new tests
      //converter << "weight/puWeight"; // Full no PU
      //converter << "XS*filterEfficiency*puWeight*genWeight/sumGenWeight";
      if(noPUweight)
        converter << "/puWeight";
    }
    else
    {
      converter << "XS*filterEfficiency*(genWeight/sumGenWeight)";
    }
    converter << "*" << luminosity;
    converter >> mcWeight;
  }
  if(rawEvents)
    mcWeight = "1.0";
  std::cout << "Using mcWeight: " << mcWeight << std::endl;

  std::vector<CutInfo> cutFlow;
  if(cutsJson == "")
  {
    cutFlow.push_back(CutInfo("Preselection", "HT > 200", "$H_T > 200$"));
    //cutFlow.push_back(CutInfo("JetPt110", "Jet1Pt > 110", "$p_T\\left(j_1\\right) > 110$"));
    //cutFlow.push_back(CutInfo("MET300", "Met > 300", "$MET > 300$"));
    //cutFlow.push_back(CutInfo("LepPt30", "LepPt < 30", "$p_T\\left(l\\right) < 30$"));
    cutFlow.push_back(CutInfo("Selection", "(HT > 200) && (Met > 280) && (Jet1Pt > 110) && (nGoodEl+nGoodMu <= 2)", "Selection"));
    cutFlow.push_back(CutInfo("Test", "(HT > 200) && (Met > 280) && (Jet1Pt > 110) && (nGoodEl+nGoodMu <= 2) && (Njet60 < 3)", "$N(jet_{60}) < 3$"));
  }
  else
  {
    json jsonFile;
    std::ifstream inputFile(cutsJson);
    inputFile >> jsonFile;

    if(jsonFile.count("cuts") == 0)
      throw MissingJSONParam("The JSON file does not contain the 'cuts' entry. It is not a valid file.");

    for(auto& cut : jsonFile["cuts"])
    {
      try
      {
        std::string cutName, cutString, cutLatex;

        if(cut.count("name") == 0)
          throw MissingJSONParam("The cut does not have a name.");
        cutName = cut["name"];

        if(cut.count("expression") == 0)
          throw MissingJSONParam("The cut does not have an expression.");
        cutString = cut["expression"];

        if(cut.count("latex") == 0)
        {
          cutLatex = cutName;
        }
        else
        {
          cutLatex = cut["latex"];
        }

        cutFlow.push_back(CutInfo(cutName, cutString, cutLatex));
      }
      catch(MissingJSONParam& exception)
      {
        std::cout << "Incomplete cut found, skipping it." << std::endl;
        std::cout << "The message was: " << exception.what() << std::endl;
      }
    }
  }

  std::ofstream cutFlowTable(outputDirectory+"/cutFlow.tex");

  cutFlowTable << "\\begin{tabular}{";
  cutFlowTable << "c|";
  for(auto& process : MC)
  {
    cutFlowTable << "c";
    process.label();
  }
  cutFlowTable << "|c";
  for(auto& process : Data)
  {
    cutFlowTable << "c";
    process.label();
  }
  cutFlowTable << "|";
  for(auto& process : Sig)
  {
    cutFlowTable << "c";
    process.label();
  }
  cutFlowTable << "}\n";

  //cutFlowTable << " & ";
  for(auto& process : MC)
  {
    cutFlowTable << " & " << process.label();
  }
  cutFlowTable << " & Sum";
  for(auto& process : Data)
  {
    cutFlowTable << " & " << process.label();
  }
  for(auto& process : Sig)
  {
    cutFlowTable << " & " << process.label();
  }
  cutFlowTable << "\\\\\n";

  std::string selection = "";
  std::string blindSel = "";
  if(!unblind && Data.hasBDT())
    blindSel = "(BDT < 0.3) && ";
  for(auto& cut : cutFlow)
  {
    if(cut.cut() != "")
    {
      if(cumulativeCuts)
      {
        if(selection == "")
          selection  = "(" + cut.cut() + ")";
        else
          selection += " && (" + cut.cut() + ")";
      }
      else
      {
        selection = cut.cut();
      }
    }

    std::cout << "Getting variables and yields with selection (" << selection << ") and weight (" << mcWeight << ")" << std::endl;

    for(auto & variable : variables)
    {
      std::string dataSel;
      if(variable.expression() == "BDT")
        dataSel = blindSel+selection;
      else
        dataSel = selection;
      if(ddfake)
        dataSel = "weight * (" + dataSel + ")";
      //auto dataH = Data.getHist(cut.name()+"_"+variable.name()+"_Data",   variable.expression(), variable.label()+";Evt.", dataSel    , variable.bins(), variable.min(), variable.max());
      auto dataH = Data.process(0).getHist(variable.expression(), variable.label()+";Evt.", dataSel    , variable.bins(), variable.min(), variable.max());
      auto mcH   =   MC.getHist(cut.name()+"_"+variable.name()+"_MC",     variable.expression(), variable.label()+";Evt.", (rawEvents)?(selection):(mcWeight+"*("+selection+")"), variable.bins(), variable.min(), variable.max());
      //auto sigH  =  Sig.getHist(cut.name()+"_"+variable.name()+"_Signal", variable.expression(), variable.label()+";Evt.", (rawEvents)?(selection):(mcWeight+"*("+selection+")"), variable.bins(), variable.min(), variable.max());
      TH1D* sigH = nullptr;
      if(Sig.nProcesses() > 0)
        sigH  =  Sig.process(0).getHist(variable.expression(), variable.label()+";Evt.", (rawEvents)?(selection):(mcWeight+"*("+selection+")"), variable.bins(), variable.min(), variable.max());
      else
        sigH = Sig.getHist(cut.name()+"_"+variable.name()+"_MC",     variable.expression(), variable.label()+";Evt.", (rawEvents)?(selection):(mcWeight+"*("+selection+")"), variable.bins(), variable.min(), variable.max());

      auto mcS   =   MC.getStack(variable.expression(), variable.label()+";Evt.", (rawEvents)?(selection):(mcWeight+"*("+selection+")"), variable.bins(), variable.min(), variable.max());

      auto ratio = static_cast<TH1D*>(dataH->Clone((cut.name()+"_"+variable.name()+"_Ratio").c_str()));
      ratio->SetTitle((";" + variable.label() + ";Data/#Sigma MC").c_str());
      ratio->Divide(mcH);

      TCanvas c1((cut.name()+"_"+variable.name()).c_str(), "", 1200, 1350); // 800x900 in original, then scaled by 1.5
      gStyle->SetOptStat(0);

      double ratioPadFraction = 0.20;
      double legPadFraction = 0.2;

      TPad* t1 = new TPad("t1","t1", 0.0, ratioPadFraction, 1.0, 1.0);
      TPad* t2 = new TPad("t2","t2", 0.0, 0.0, 1.0, ratioPadFraction);

      t1->Draw();
      t1->cd();
      t1->SetLogy(1);
      mcS->Draw("hist");
      if(variable.legTop())
      {
        double maxVal = mcS->GetYaxis()->GetXmax();
        double minVal = mcS->GetYaxis()->GetXmin();

        maxVal = std::max(mcS->GetMaximum(), dataH->GetMaximum());
        minVal = std::min(mcS->GetMinimum(), dataH->GetMinimum());
        minVal = 0.2;

        if(t1->GetLogy() == 1)
        {
          if(minVal == 0)
            minVal = 0.5;

          maxVal = std::pow(maxVal/std::pow(minVal, legPadFraction), 1/(1 - legPadFraction));
        }
        else
        {
          maxVal = maxVal + legPadFraction*(maxVal - minVal) / (1 - legPadFraction);
        }

        mcS->SetMinimum(minVal);
        mcS->SetMaximum(maxVal);
        //mcS->Draw("hist");
      }
      if(!puTest)
      {
        dataH->Draw("same");
        sigH->Draw("hist same");
      }
      else
      {
        TH1D* data2 = static_cast<TH1D*>(dataH->Clone("dataToDraw"));
        data2->Scale(mcH->Integral()/data2->Integral());
        data2->Draw("same");
      }

      TLegend *legA;
      std::cout << "  The legTop value is: " << variable.legTop() << " (" << (variable.legTop()?"true":"false") << ")" << std::endl;
      std::cout << "  The legLeft value is: " << variable.legLeft() << " (" << (variable.legLeft()?"true":"false") << ")" << std::endl;
      if(variable.legTop())
      {
        //legA = gPad->BuildLegend(0.155, 1, 0.845, 1-legPadFraction, "NDC"); // The current version does not allow options... what?
        legA = gPad->BuildLegend(0.155, 0.95, 0.845, 0.95*(1-legPadFraction), "");
        legA->SetNColumns(3);
      }
      else
      {
        if(variable.legLeft())
          //legA = gPad->BuildLegend(0.155,0.39,0.35,0.89,"", "NDC"); // The current version does not allow options... what?
          legA = gPad->BuildLegend(0.155,0.39,0.35,0.89,"");
        else
          //legA = gPad->BuildLegend(0.845,0.39,0.65,0.89,"", "NDC"); // The current version does not allow options... what?
          legA = gPad->BuildLegend(0.845,0.39,0.65,0.89,"");
      }

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
      t2->SetPad(0,0.0,1.0,ratioPadFraction);
      t2->SetTopMargin(0);
      t2->SetBottomMargin(0.25);

      TH1D *bgUncH = static_cast<TH1D*>(mcH->Clone((cut.name()+"_"+variable.name()+"_bgUncH").c_str()));
      for(int xbin=1; xbin <= bgUncH->GetXaxis()->GetNbins(); xbin++)
      {
        if(bgUncH->GetBinContent(xbin)==0)
          continue;

        double unc = bgUncH->GetBinError(xbin) / bgUncH->GetBinContent(xbin);
        double binContent = bgUncH->GetBinContent(xbin);

        // Add systematic uncertainties
        unc = unc*unc;
        unc += 0.025*0.025; // Luminosity uncertainty
        unc += 0.01*0.01; // Efficiency
        unc += 0.01*0.01; // Pile Up
        unc += 0.04*0.04; // JES
        unc += 0.04*0.04; // Lep ID

        TList* hists = mcS->GetHists();
        TIter next(hists);
        TObject* obj = nullptr;
        while ((obj = next()))
        {
          TH1* thisHist = static_cast<TH1*>(obj);
          std::string histName = thisHist->GetName();
          double thisUnc = 0;
          double thisBinContent=thisHist->GetBinContent(xbin);
          if(histName.find("WJets") != std::string::npos || histName.find("ttbar") != std::string::npos) // ttbar and WJets
          {
            thisUnc = thisBinContent*0.2;
          }
          else
          {
            thisUnc = thisBinContent*0.2;
          }

          thisUnc = thisUnc/binContent;
          unc += thisUnc*thisUnc;
        }

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
      double minErr = -0.1;
      double maxErr = 2.1;
      double yscale = (1.0-0.2)/(0.18-0);
      bgUncH->GetYaxis()->SetTitle("Data/#Sigma MC");
      bgUncH->SetMinimum(minErr);
      bgUncH->SetMaximum(maxErr);
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

      c1.SaveAs((outputDirectory+"/"+cut.name()+"_"+variable.name()+".png").c_str());
      c1.SaveAs((outputDirectory+"/"+cut.name()+"_"+variable.name()+".C").c_str());

      delete dataH;
      delete mcH;
      delete sigH;
      // Delete individual hists in the stack
      TList* histList = mcS->GetHists();
      if(histList != nullptr)
        histList->Delete();
      delete mcS;
      delete ratio;
      delete legA;
      delete T;
      delete bgUncH;
      delete bgUnc;
    }

    for(auto & twoDvariable : twoDvariables)
    {
      std::string dataSel;
      if(twoDvariable.X().expression() == "BDT" || twoDvariable.Y().expression() == "BDT")
        dataSel = blindSel+selection;
      else
        dataSel = selection;

      int nPlots = 0;
      nPlots += Data.nProcesses();
      nPlots += MC.nProcesses();
      nPlots += Sig.nProcesses();

      int plotX = 400;
      int plotY = 400;
      int canvasX = 0;
      int canvasY = 0;
      int nPadsX = 0;
      int nPadsY = 0;
      switch(nPlots)
      {
        case 1:
          canvasY = plotY;
          canvasX = plotX;
          nPadsX = 1;
          nPadsY = 1;
          break;
        case 2:
          canvasY = plotY;
          canvasX = 2*plotX;
          nPadsX = 2;
          nPadsY = 1;
          break;
        case 3:
          canvasY = plotY;
          canvasX = 3*plotX;
          nPadsX = 3;
          nPadsY = 1;
          break;
        case 4:
          canvasY = 2*plotY;
          canvasX = 2*plotX;
          nPadsX = 2;
          nPadsY = 2;
          break;
        case 5:
        case 6:
          canvasY = 2*plotY;
          canvasX = 3*plotX;
          nPadsX = 3;
          nPadsY = 2;
          break;
        case 7:
        case 8:
          canvasY = 2*plotY;
          canvasX = 4*plotX;
          nPadsX = 4;
          nPadsY = 2;
          break;
        case 9:
          canvasY = 3*plotY;
          canvasX = 3*plotX;
          nPadsX = 3;
          nPadsY = 3;
          break;
        case 10:
        case 11:
        case 12:
          canvasY = 3*plotY;
          canvasX = 4*plotX;
          nPadsX = 4;
          nPadsY = 3;
          break;
        default:
          throw std::out_of_range("Calm down speedy, check the number of processes");
      }

      std::vector<TObject*> ObjectToDelete;
      TCanvas c1((cut.name()+"_"+twoDvariable.name()).c_str(), "", canvasX, canvasY);
      c1.SetLogz();
      c1.Divide(nPadsX,nPadsY,0,0);

      int pad = 0;
      for(auto & process : Data)
      {
        pad++;
        TVirtualPad* thisPad = c1.cd(pad);

        thisPad->SetLogz(true);
        thisPad->SetTopMargin(0.10);
        thisPad->SetBottomMargin(0.10);
        thisPad->SetRightMargin(0.20);

        auto hist = process.get2DHist(twoDvariable.X().expression(),
                                      twoDvariable.Y().expression(),
                                      twoDvariable.X().label()+";"+twoDvariable.Y().label()+";Evt.",
                                      dataSel,
                                      twoDvariable.X().bins(),
                                      twoDvariable.X().min(),
                                      twoDvariable.X().max(),
                                      twoDvariable.Y().bins(),
                                      twoDvariable.Y().min(),
                                      twoDvariable.Y().max());
        ObjectToDelete.push_back(hist);
        hist->SetTitle("");
        hist->SetStats(kFALSE);
        hist->Draw("COLZ");

        TPaveText* leg = new TPaveText(0.10,0.995,0.90,0.90, "NDC");
        leg->SetFillColor(0);
        leg->SetFillStyle(0);
        leg->SetLineColor(0);
        leg->SetTextAlign(12);
        leg->AddText(process.label().c_str());
        leg->Draw("same");
        ObjectToDelete.push_back(leg);
      }
      for(auto & process : MC)
      {
        pad++;
        TVirtualPad* thisPad = c1.cd(pad);

        thisPad->SetLogz(true);
        thisPad->SetTopMargin(0.10);
        thisPad->SetBottomMargin(0.10);
        thisPad->SetRightMargin(0.20);

        auto hist = process.get2DHist(twoDvariable.X().expression(),
                                      twoDvariable.Y().expression(),
                                      twoDvariable.X().label()+";"+twoDvariable.Y().label()+";Evt.",
                                      (rawEvents)?(selection):(mcWeight+"*("+selection+")"),
                                      twoDvariable.X().bins(),
                                      twoDvariable.X().min(),
                                      twoDvariable.X().max(),
                                      twoDvariable.Y().bins(),
                                      twoDvariable.Y().min(),
                                      twoDvariable.Y().max());
        ObjectToDelete.push_back(hist);
        hist->SetTitle("");
        hist->SetStats(kFALSE);
        hist->Draw("COLZ");

        TPaveText* leg = new TPaveText(0.10,0.995,0.90,0.90, "NDC");
        leg->SetFillColor(0);
        leg->SetFillStyle(0);
        leg->SetLineColor(0);
        leg->SetTextAlign(12);
        leg->AddText(process.label().c_str());
        leg->Draw("same");
        ObjectToDelete.push_back(leg);
      }
      for(auto & process : Sig)
      {
        pad++;
        TVirtualPad* thisPad = c1.cd(pad);

        thisPad->SetLogz(true);
        thisPad->SetTopMargin(0.10);
        thisPad->SetBottomMargin(0.10);
        thisPad->SetRightMargin(0.20);

        auto hist = process.get2DHist(twoDvariable.X().expression(),
                                      twoDvariable.Y().expression(),
                                      twoDvariable.X().label()+";"+twoDvariable.Y().label()+";Evt.",
                                      (rawEvents)?(selection):(mcWeight+"*("+selection+")"),
                                      twoDvariable.X().bins(),
                                      twoDvariable.X().min(),
                                      twoDvariable.X().max(),
                                      twoDvariable.Y().bins(),
                                      twoDvariable.Y().min(),
                                      twoDvariable.Y().max());
        ObjectToDelete.push_back(hist);
        hist->SetTitle("");
        hist->SetStats(kFALSE);
        hist->Draw("COLZ");

        TPaveText* leg = new TPaveText(0.10,0.995,0.90,0.90, "NDC");
        leg->SetFillColor(0);
        leg->SetFillStyle(0);
        leg->SetLineColor(0);
        leg->SetTextAlign(12);
        leg->AddText(process.label().c_str());
        leg->Draw("same");
        ObjectToDelete.push_back(leg);
      }
      c1.cd(0);

      c1.SaveAs((outputDirectory+"/"+cut.name()+"_"+twoDvariable.name()+".png").c_str());
      c1.SaveAs((outputDirectory+"/"+cut.name()+"_"+twoDvariable.name()+".C").c_str());

      for(unsigned int d = 0; d < ObjectToDelete.size(); d++)
        delete ObjectToDelete[d];
      ObjectToDelete.clear();
    }

    cutFlowTable << cut.name();
    for(auto& process : MC)
    {
      auto yield = process.getYield(selection, mcWeight);
      cutFlowTable << " & " << yield;
      if(verbose)
      {
        std::cout << process.label() << ": " << yield << std::endl;
        for(auto& sample: process)
          std::cout << sample.tag() << ": " << sample.getYield(selection, mcWeight) << std::endl;
      }
    }
    cutFlowTable << " & " << MC.getYield(selection, mcWeight);
    for(auto& process : Data)
    {
      std::string weight = "1";
      if(ddfake)
        weight = "weight";
      auto yield = process.getYield(blindSel+selection, weight);
      cutFlowTable << " & " << yield;
      if(blindSel != "")
        cutFlowTable << " (SR blinded)";
      if(verbose)
        std::cout << process.label() << ": " << yield << std::endl;
    }
    for(auto& process : Sig)
    {
      auto yield = process.getYield(selection, mcWeight);
      cutFlowTable << " & " << yield;
      if(verbose)
        std::cout << process.label() << ": " << yield << std::endl;
    }
    cutFlowTable << "\\\\\n";
  }

  cutFlowTable << "\\end{tabular}";

  return 0;
}

void printHelp()
{
  std::cout << "The program has the following parameters:" << std::endl;
  std::cout << "\t--help\t- Print this message" << std::endl;

  return;
}
