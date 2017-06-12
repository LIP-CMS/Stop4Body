#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <utility>

#include "TChain.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"
#include "TPad.h"
#include "TStyle.h"
#include "TGraphErrors.h"
#include "TFitResult.h"
#include "TLegend.h"
#include "TF1.h"
#include "TPaveText.h"

#include "UserCode/Stop4Body/interface/SampleReader.h"

void printHelp();

int main(int argc, char **argv)
{
  std::string jsonFileName = "";
  std::string inputDirectory = "";
  std::string outputDirectory = "./OUT/";
  std::string drawMVA = "BDT";
  std::string suffix = "";
  double luminosity = 0;
  bool isPseudoData = false;
  bool unblind = false;

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
      std::stringstream conv;
      conv << argv[++i];
      conv >> luminosity;
    }

    if(argument == "--draw")
      drawMVA = argv[++i];

    if(argument == "--isPseudoData")
    {
      isPseudoData = true;
      unblind = true;
    }

    if(argument == "--unblind")
      unblind = true;
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

  std::cout << "Reading JSON file" << std::endl;
  SampleReader samples(jsonFileName, inputDirectory, suffix);
  if(luminosity <= 0)
    luminosity = samples.getData().getLumi();

  std::map<std::string, std::string> regions;
  //regions["test1"] = "(LepPt < 30)";
  //regions["test2"] = "(LepPt < 30) && (Met > 300)";
  //regions["test3"] = "(LepPt < 30) && (Met > 300) && (Jet1Pt > 110)";
  regions["SR"]       = ("(LepPt < 30) && (Met > 280) && (Jet1Pt > 110) && (HT > 200)");
  regions["CR_ttbar"] = ("(LepPt < 30) && (Met > 280) && (Jet1Pt > 110) && (HT > 200) && (NbTight > 0)");
  regions["CR_wjets"] = ("(LepPt < 30) && (Met > 280) && (Jet1Pt > 110) && (HT > 200) && (NbLoose == 0)");
  regions["VR1_ttbar"] = ("(LepPt < 30) && (Met > 200) && (Met < 280) && (Jet1Pt > 110) && (HT > 200) && (NbTight > 1)");
  regions["VR1_wjets"] = ("(LepPt < 30) && (Met > 200) && (Met < 280) && (Jet1Pt > 110) && (HT > 200) && (NbLoose == 0)");// */
  /*regions["SR"]       = ("(Met > 280) && (Jet1Pt > 110) && (HT > 200)");
  regions["CR_ttbar"] = ("(Met > 280) && (Jet1Pt > 110) && (HT > 200) && (NbTight > 0)");
  regions["CR_wjets"] = ("(Met > 280) && (Jet1Pt > 110) && (HT > 200) && (NbLoose == 0)");
  regions["VR1_ttbar"] = ("(Met > 200) && (Met < 280) && (Jet1Pt > 110) && (HT > 200) && (NbTight > 1)");
  regions["VR1_wjets"] = ("(Met > 200) && (Met < 280) && (Jet1Pt > 110) && (HT > 200) && (NbLoose == 0)");// */
  /*regions["SR"]       = ("(LepPt < 280) && (Met > 280) && (Jet1Pt > 110) && (HT > 200)");
  regions["CR_ttbar"] = ("(LepPt < 280) && (Met > 280) && (Jet1Pt > 110) && (HT > 200) && (NbTight > 0)");
  regions["CR_wjets"] = ("(LepPt < 280) && (Met > 280) && (Jet1Pt > 110) && (HT > 200) && (NbLoose == 0)");
  regions["VR1_ttbar"] = ("(LepPt < 280) && (Met > 200) && (Met < 280) && (Jet1Pt > 110) && (HT > 200) && (NbTight > 1)");
  regions["VR1_wjets"] = ("(LepPt < 280) && (Met > 200) && (Met < 280) && (Jet1Pt > 110) && (HT > 200) && (NbLoose == 0)");// */

  std::stringstream converter;
  std::string mcWeight;
  if(isPseudoData)
    converter << "splitFactor*XS*filterEfficiency*(genWeight/sumGenWeight)*"; // The scale factors are not considered here
  else
    converter << "splitFactor*weight*";
  converter << luminosity;
  converter >> mcWeight;

  for(auto& region : regions)
  {
    //TFile testfile("/home/t3cms/cbeiraod/local-area/Stop4Body/nTuples_v2016-07-12_bdt/PseudoData_bdt.root", "READ");
    //TTree* tree = static_cast<TTree*>(testfile.Get("bdttree"));
    //TH1D testH("BDT_test", ";BDT;Evt.", 20, -1, 1);
    //testH.Sumw2();
    //tree->Draw("BDT>>BDT_test", region.second.c_str(), "goff");

    auto dataH = samples.getData().getHist("Data", "BDT", "BDT;Evt.", region.second, 26, -0.7, 0.6);
    auto mcH = samples.getMCBkg().getHist("MC", "BDT", "BDT;Evt.", mcWeight+"*("+region.second+")", 26, -0.7, 0.6);
    auto sigH = samples.getMCSig().getHist("Signal", "BDT", "BDT;Evt.", mcWeight+"*("+region.second+")", 26, -0.7, 0.6);
    auto mcS = samples.getMCBkg().getStack("BDT", "BDT;Evt.", mcWeight+"*("+region.second+")", 26, -0.7, 0.6);

    if(!unblind)
    {
      auto theBin = dataH->FindBin(0.3);
      for(int i = theBin; i < 28; ++i)
      {
        dataH->SetBinContent(i, 0.01);
        dataH->SetBinError(i, 0.01);
      }
    }

    TCanvas c1((region.first+"_BDTOut").c_str(), "", 800, 1200);
    gStyle->SetOptStat(0);
    gStyle->SetOptTitle(0);
    c1.Divide(1,2);

    c1.cd(1);
    gPad->SetLogy();
    mcS->Draw("hist");
    dataH->Draw("same");
    sigH->Draw("hist same");
    //TLegend * legend =
    gPad->BuildLegend(0.895,0.69,0.65,0.89,"Title");
    //testH.Draw();

    c1.cd(2);
    gPad->SetGridx(true);
    gPad->SetGridy(true);
    auto ratio = static_cast<TH1D*>(dataH->Clone("ratio"));
    ratio->SetTitle(";BDT;Data/#Sigma MC");
    ratio->Divide(mcH);
    ratio->Draw();
    //ratio->SetRangeUser(0.5, 1.5);
    ratio->SetMaximum(1.3);
    ratio->SetMinimum(0.7);

    c1.SaveAs((outputDirectory+"/"+region.first+".png").c_str());
    c1.SaveAs((outputDirectory+"/"+region.first+".C").c_str());

    TCanvas c4((region.first+"_BDTCompRatio").c_str(), "", 800, 800);
    gStyle->SetOptStat(0);
    TPad* t1 = new TPad("t1","t1", 0.0, 0.20, 1.0, 1.0);
    //TPad* t2 = nullptr;
    TPad* t2 = new TPad("t2","t2", 0.0, 0.0, 1.0, 0.2);
    t1->Draw();
    t1->cd();
    t1->SetLogy(true);
    mcS->Draw("hist");
    dataH->Draw("same");
    sigH->Draw("hist same");
    //TLegend *legA = gPad->BuildLegend(0.845,0.69,0.65,0.89, "NDC");
    TLegend *legA = gPad->BuildLegend(0.155,0.69,0.35,0.89, "NDC");
    legA->SetFillColor(0); legA->SetFillStyle(0); legA->SetLineColor(0);
    legA->SetHeader("");
    legA->SetTextFont(42);
    TPaveText* T = new TPaveText(0.1,0.995,0.84,0.95, "NDC");
    T->SetFillColor(0);
    T->SetFillStyle(0); T->SetLineColor(0);
    T->SetTextAlign(12);
    char Buffer[1024];
    sprintf(Buffer, "CMS preliminary, #sqrt{s}=%.1f TeV, #scale[0.5]{#int} L=%.1f fb^{-1}", 13.0, luminosity/1000);
    T->AddText(Buffer);
    T->Draw("same");
    T->SetBorderSize(0);
    c4.cd();
    t2->Draw();
    t2->cd();
    t2->SetGridy(true);
    t2->SetPad(0,0.0,1.0,0.2);
    t2->SetTopMargin(0);
    t2->SetBottomMargin(0.5);
    TH1D *bgUncH = static_cast<TH1D*>(mcH->Clone("bgUncH"));
    for(int xbin=1; xbin<=bgUncH->GetXaxis()->GetNbins(); xbin++)
    {
      if(bgUncH->GetBinContent(xbin)==0)
        continue;

      double unc = bgUncH->GetBinError(xbin) / bgUncH->GetBinContent(xbin);

      // Add systematic uncertainties
      unc = unc*unc;
      unc += 0.026*0.026; // Luminosity uncertainty
//      unc += 0.15*0.15; // Value assumed for total systematic uncertainty
      unc = std::sqrt(unc);

      bgUncH->SetBinContent(xbin,1);
      bgUncH->SetBinError(xbin,unc);
    }
    TGraphErrors *bgUnc=new TGraphErrors(bgUncH);
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

    c4.SaveAs((outputDirectory+"/"+region.first+"_BDTCompRatio.png").c_str());
    c4.SaveAs((outputDirectory+"/"+region.first+"_BDTCompRatio.C").c_str());

    TCanvas c3((region.first+"_SignalContam").c_str(), "", 800, 600);
    auto contam = static_cast<TH1D*>(sigH->Clone("contamination"));
    contam->SetTitle(";BDT;Signal/#Sigma MC");
    contam->Divide(mcH);
    contam->Draw();
    contam->SetMaximum(0.1);
    contam->SetMinimum(0.0);
    c3.SaveAs((outputDirectory+"/"+region.first+"_SignalContam.png").c_str());
    c3.SaveAs((outputDirectory+"/"+region.first+"_SignalContam.C").c_str());


    TCanvas c2((region.first+"_BDTfits").c_str(), "", 800, 600);
    //c2.cd();
    c2.SetGridx(true);
    c2.SetGridy(true);
    //c2.SetPad(0,0.0,1.0,0.2);
    //c2.SetTopMargin(0);
    //c2.SetBottomMargin(0.5);

    std::vector<double> xvals, yvals, xerrs, yerrs;
    for(double x = -0.7; x <= 0.1; x += 0.1)
    {
      ratio->Fit("pol0", "", "", x, 0.2);
      auto fit = ratio->GetFunction("pol0");

      xvals.push_back(x);
      xerrs.push_back(0);
      yvals.push_back(fit->GetParameter(0));
      yerrs.push_back(fit->GetParError(0));

      /*auto fitResult = ratio->Fit("pol0", "", "", x, 0.5);

      xvals.push_back(x);
      xerrs.push_back(0);
      yvals.push_back(fitResult->Parameter(0));
      yerrs.push_back(fitResult->ParError(0));// */
    }

    TGraphErrors graph(xvals.size(), &(xvals[0]), &(yvals[0]), &(xerrs[0]), &(yerrs[0]));
    graph.SetTitle("Fit Results");
    //graph.SetFillColor(4);
    //graph.SetFillStyle(3010);
    //graph.Draw("al3po");
    graph.SetLineColor(1);
    graph.SetFillStyle(3001);
    graph.SetFillColor(kGray);
    graph.SetMarkerColor(1);
    graph.SetMarkerStyle(2);
    graph.Draw("al3po");
    graph.GetYaxis()->SetRangeUser(0.7,1.3);

    c2.SaveAs((outputDirectory+"/"+region.first+"_fits.png").c_str());
    c2.SaveAs((outputDirectory+"/"+region.first+"_fits.C").c_str());// */

    std::ofstream outFile(outputDirectory+"/"+region.first+"_fitsTable.tex", std::ios_base::trunc);
    //outFile << "%\usepackage[binary-units=true,separate-uncertainty=true,multi-part-units=single,forbid-literal-units=true,round-half=even,round-mode=places,round-precision=3]{siunitx}" << std::endl;
    outFile << "%\\usepackage{siunitx}" << std::endl;
    outFile << "%\\sisetup{ round-half=even, round-mode=places, round-precision=3, separate-uncertainty=true, table-figures-integer = 1, table-figures-decimal = 2}" << std::endl;
    outFile << "\\begin{table}" << std::endl;
    outFile << "\\begin{tabular}[!htbp]{cS@{$\\pm$}S[table-number-alignment=left]S@{$\\pm$}S[table-number-alignment=left]S@{$\\pm$}S[table-number-alignment=left]}" << std::endl;
    outFile << "\\hline" << std::endl;
    outFile << "Interval & \\multicolumn{2}{c}{Polinomial 0} & \\multicolumn{4}{c}{Polinomial 1} \\\\" << std::endl;
    outFile << " & \\multicolumn{2}{c}{Fit Results} & \\multicolumn{2}{c}{constant} & \\multicolumn{2}{c}{Slope} \\\\" << std::endl;
    outFile << "\\hline" << std::endl;
    for(unsigned int i = 0; i < xvals.size(); ++i)
    {
      ratio->Fit("pol1", "", "", xvals[i], 0.2);
      auto fit1 = ratio->GetFunction("pol1");

      outFile << "$\\left[" << xvals[i] << ", 0.2\\right]$ & " << yvals[i] << " & " << yerrs[i] << " & ";
      outFile << fit1->GetParameter(0) << " & " << fit1->GetParError(0) << " & ";
      outFile << fit1->GetParameter(1) << " & " << fit1->GetParError(1) << "\\\\" << std::endl;
    }
    outFile << "\\hline" << std::endl;
    outFile << "\\end{tabular}" << std::endl;
    outFile << "\\end{table}" << std::endl;

    outFile << std::endl;

    auto dataSamples = samples.getData();
    auto mcSamples = samples.getMCBkg();
    auto sigSamples = samples.getMCSig();
    std::string cut;

    outFile << "\\begin{table}" << std::endl;
    outFile << "\\begin{tabular}[!htbp]{cccccc}" << std::endl;
    outFile << "\\hline" << std::endl;
    outFile << " & $\\text{BDT}_\\text{Out} < 0.53$ & $\\text{BDT}_\\text{Out} < 0.2$ & $\\text{BDT}_\\text{Out} > 0.53$ & $\\text{BDT}_\\text{Out} > 0.2$ & $0.2 < \\text{BDT}_\\text{Out} < 0.53$ \\\\" << std::endl;
    outFile << "\\hline" << std::endl;
    outFile << "\\verb|" << region.first << "| & ";
    cut = "(BDT < 0.53) && " + region.second;
    outFile << dataSamples.getYield(cut, "1")/mcSamples.getYield(cut, mcWeight);
    outFile << " & ";
    cut = "(BDT < 0.2) && " + region.second;
    outFile << dataSamples.getYield(cut, "1")/mcSamples.getYield(cut, mcWeight);
    outFile << " & ";
    cut = "(BDT > 0.53) && " + region.second;
    outFile << dataSamples.getYield(cut, "1")/mcSamples.getYield(cut, mcWeight);
    outFile << " & ";
    cut = "(BDT > 0.2) && " + region.second;
    outFile << dataSamples.getYield(cut, "1")/mcSamples.getYield(cut, mcWeight);
    outFile << " & ";
    cut = "(BDT < 0.53) && (BDT > 0.2) && " + region.second;
    outFile << dataSamples.getYield(cut, "1")/mcSamples.getYield(cut, mcWeight);
    outFile << " \\\\" << std::endl;
    outFile << "\\hline" << std::endl;
    outFile << "\\end{tabular}" << std::endl;
    outFile << "\\end{table}" << std::endl;

    outFile << std::endl << std::endl;
    outFile << "Data: " << dataSamples.getYield(region.second, "1") << std::endl;
    outFile << "MC: " << mcSamples.getYield(region.second, mcWeight) << std::endl;
    for(auto & process: mcSamples)
      outFile << "\t" << process.label() << ": " << process.getYield(region.second, mcWeight) << std::endl;
    outFile << "Signal: " << sigSamples.getYield(region.second, mcWeight) << std::endl;


    delete ratio;
    delete dataH;
    delete mcH;
    delete sigH;
    delete mcS;
  }

  return 0;
}

void printHelp()
{
  std::cout << "The program has the following parameters:" << std::endl;
  std::cout << "\t--help\t- Print this message" << std::endl;
  std::cout << "\t--json\t- Used to specify the json file which describes the samples, this option is obligatory. The parameter immediately after this one should be the desired json file" << std::endl;
  std::cout << "\t--outDir\t- Used to specify the output directory. The parameter immediately after this one should be the desired directory. By default the directory './OUT/' is used" << std::endl;

  return;
}
