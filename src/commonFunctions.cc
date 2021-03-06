
#include "UserCode/Stop4Body/interface/commonFunctions.h"
//#define M_PIl          3.141592653589793238462643383279502884L /* pi */

#include <cmath>
#include <fstream>

#include "TMath.h"

TH2D* centralElectronSFHist = nullptr;
TH2D* centralMuonSFHist = nullptr;
TH1F* hephyElectronIDSFHistBarrel = nullptr;
TH1F* hephyElectronIDSFHistEndcap = nullptr;
TH1F* hephyMuonIDSFHist = nullptr;
TH1F* hephyElectronISOSFHistBarrel = nullptr;
TH1F* hephyElectronISOSFHistEndcap = nullptr;
TH1F* hephyMuonISOSFHist = nullptr;
TH1F* electronTightToLooseLowEta = nullptr;
TH1F* electronTightToLooseHighEta = nullptr;
TH1F* muonTightToLooseLowEta = nullptr;
TH1F* muonTightToLooseHighEta = nullptr;


bool fileExists(std::string fileName)
{
  std::ifstream infile(fileName);
  return infile.good();
}

std::string getBaseName(const std::string& s)
{
  char sep = '/';

  size_t i = s.rfind(sep, s.length());
  if (i != std::string::npos)
  {
    return s.substr(i+1, s.length() - i);
  }

  return "";
}

float DeltaPhi(double p1, double p2)
{
  double x = p1 - p2;

  while(x >= M_PIl)
    x -= (2.*M_PIl);

  while(x < -M_PIl)
    x += (2.*M_PIl);

  if(x < 0)
    return -x;
  return x;
  //return std::abs(x);
}

// Taken from Ivan's presentation, here: https://www.dropbox.com/s/nqj5qfpikvws1rv/17-03-internal2-mikulec.pdf?dl=0
doubleUnc triggerEfficiencyFromMET(double met_pt)
{
  double val = 0, unc = 0;

  double par0    = 0.9899;
  double par1    = 109.8;
  double par2    = 94.26;
  double par0err = 0.0006464;
  double par1err = 2.225;
  double par2err = 2.443;

  double recenterMet = (met_pt - par1)/par2;

  val = par0 * 0.5 * (1.0 + TMath::Erf(recenterMet));

  double term1 = par0err * 0.5 * (1.0 + TMath::Erf(recenterMet));
  term1 = term1 * term1;
  double term2 = (par0/par2)*(par0/par2) * std::exp(2*recenterMet*recenterMet) / TMath::Pi();
  term2 *= par1err*par1err + par2err*par2err*recenterMet*recenterMet;
  unc = std::sqrt(term1 + term2);

  doubleUnc retVal(val, unc);
  return retVal;
}

// Taken from Ivan's presentation, here: https://indico.cern.ch/event/613194/
doubleUnc WISRScaleFactorFromLepMet(double lep_pt, double lep_phi, double met_pt, double met_phi)
{
  double lep_x = lep_pt * std::cos(lep_phi);
  double lep_y = lep_pt * std::sin(lep_phi);
  double met_x = met_pt * std::cos(met_phi);
  double met_y = met_pt * std::sin(met_phi);

  double w_pt = std::sqrt((lep_x + met_x)*(lep_x + met_x) + (lep_y + met_y)*(lep_y + met_y));

  double val = 1, unc = 0;
  if(w_pt > 800)
    val = 0.74, unc = 0.13;
  else
  {
    if(w_pt > 650)
      val = 0.78, unc = 0.11;
    else
    {
      if(w_pt > 450)
        val = 0.84, unc = 0.08;
      else
      {
        if(w_pt > 350)
          val = 0.9, unc = 0.05;
        else
        {
          if(w_pt > 250)
            val = 0.96, unc = 0.02;
          else
          {
            if(w_pt > 200)
              val = 0.98, unc = 0.01;
            else
              val = 1, unc = 0.01;
          }
        }
      }
    }
  }

  doubleUnc retVal(val, unc);
  return retVal;
}

// https://twiki.cern.ch/twiki/bin/viewauth/CMS/SUSRecommendationsMoriond17
// Taken from: https://indico.cern.ch/event/592621/contributions/2398559/attachments/1383909/2105089/16-12-05_ana_manuelf_isr.pdf
doubleUnc ISRweightFromNISRJet(int nISRJet)
{
  switch(nISRJet)
  {
    case 0:
      return doubleUnc(1.000, 0);
      break;
    case 1:
      return doubleUnc(0.920, 0);
      break;
    case 2:
      return doubleUnc(0.821, 0);
      break;
    case 3:
      return doubleUnc(0.715, 0);
      break;
    case 4:
      return doubleUnc(0.662, 0);
      break;
    case 5:
      return doubleUnc(0.561, 0);
      break;
    default:
      return doubleUnc(0.511, 0);
      break;
  }
  return doubleUnc(0, 0);
}

// Taken from: https://indico.cern.ch/event/616816/contributions/2489809/attachments/1418579/2174166/17-02-22_ana_isr_ewk.pdf
doubleUnc EWKISRweightFromISRpT(double lep_pt, double lep_phi, double met_pt, double met_phi)
{
  double lep_x = lep_pt * std::cos(lep_phi);
  double lep_y = lep_pt * std::sin(lep_phi);
  double met_x = met_pt * std::cos(met_phi);
  double met_y = met_pt * std::sin(met_phi);

  double w_pt = std::sqrt((lep_x + met_x)*(lep_x + met_x) + (lep_y + met_y)*(lep_y + met_y));

  return EWKISRweightFromISRpT(w_pt);
}
doubleUnc EWKISRweightFromISRpT(double ISRpT)
{
  if(ISRpT >= 0)
  {
    if(ISRpT < 50)
      return doubleUnc(1, 0);
    if(ISRpT < 100)
      return doubleUnc(1.052, 0);
    if(ISRpT < 150)
      return doubleUnc(1.179, 0);
    if(ISRpT < 200)
      return doubleUnc(1.150, 0);
    if(ISRpT < 300)
      return doubleUnc(1.057, 0);
    if(ISRpT < 400)
      return doubleUnc(1.000, 0);
    if(ISRpT < 600)
      return doubleUnc(0.912, 0);

    return doubleUnc(0.783, 0);
  }
  return doubleUnc(0, 0);
}

// Taken from Ivan's presentation, here: https://www.dropbox.com/s/nqj5qfpikvws1rv/17-03-internal2-mikulec.pdf?dl=0
doubleUnc getLeptonIDSF(double LepID, double LepPt, double LepEta)
{
  LepEta = std::abs(LepEta);
  double val = 1, unc = 0;
  if(std::abs(LepID) == 11) // If electron
  {
    if(LepPt > 10)
    {
      if(centralElectronSFHist == nullptr)
        return doubleUnc(1, 0);

      if(LepPt >= 200)
        LepPt = 199.999;
      if(LepEta >= 2.5)
        LepEta = 2.49999;

      auto bin = centralElectronSFHist->FindBin(LepPt, LepEta);
      val = centralElectronSFHist->GetBinContent(bin);
      unc = centralElectronSFHist->GetBinError(bin);
    }
    else
    {
      if(LepEta > 1.48) // Endcap electron
      {
        if(hephyElectronIDSFHistEndcap == nullptr)
          return doubleUnc(1, 0);

        auto bin = hephyElectronIDSFHistEndcap->FindBin(LepPt);
        val = hephyElectronIDSFHistEndcap->GetBinContent(bin);
        unc = hephyElectronIDSFHistEndcap->GetBinError(bin);
      }
      else // Barrel electron
      {
        if(hephyElectronIDSFHistBarrel == nullptr)
          return doubleUnc(1, 0);

        auto bin = hephyElectronIDSFHistBarrel->FindBin(LepPt);
        val = hephyElectronIDSFHistBarrel->GetBinContent(bin);
        unc = hephyElectronIDSFHistBarrel->GetBinError(bin);
      }
    }
  }
  else
  {
    if(LepPt > 10)
    {
      if(centralMuonSFHist == nullptr)
        return doubleUnc(1, 0);

      if(LepPt >= 120)
        LepPt = 119.999;
      if(LepEta >= 2.4)
        LepEta = 2.39999;

      auto bin = centralMuonSFHist->FindBin(LepPt, LepEta);
      val = centralMuonSFHist->GetBinContent(bin);
      unc = centralMuonSFHist->GetBinError(bin);
    }
    else
    {
      if(hephyMuonIDSFHist == nullptr)
        return doubleUnc(1, 0);

      auto bin = hephyMuonIDSFHist->FindBin(LepPt);
      val = hephyMuonIDSFHist->GetBinContent(bin);
      unc = hephyMuonIDSFHist->GetBinError(bin);
    }
  }

  doubleUnc retVal(val, unc);
  return retVal;
}

// Taken from Ivan's presentation, here: https://www.dropbox.com/s/nqj5qfpikvws1rv/17-03-internal2-mikulec.pdf?dl=0
doubleUnc getLeptonISOSF(double LepID, double LepPt, double LepEta)
{
  LepEta = std::abs(LepEta);
  double val = 1, unc = 0;
  if(std::abs(LepID) == 11) // If electron
  {
    if(LepEta > 1.48) // Endcap electron
    {
      if(hephyElectronISOSFHistEndcap == nullptr)
        return doubleUnc(1, 0);

      if(LepPt >= 60)
        LepPt = 59.999;

      auto bin = hephyElectronISOSFHistEndcap->FindBin(LepPt);
      val = hephyElectronISOSFHistEndcap->GetBinContent(bin);
      unc = hephyElectronISOSFHistEndcap->GetBinError(bin);
    }
    else // Barrel electron
    {
      if(hephyElectronISOSFHistBarrel == nullptr)
        return doubleUnc(1, 0);

      if(LepPt >= 60)
        LepPt = 59.999;

      auto bin = hephyElectronISOSFHistBarrel->FindBin(LepPt);
      val = hephyElectronISOSFHistBarrel->GetBinContent(bin);
      unc = hephyElectronISOSFHistBarrel->GetBinError(bin);
    }
  }
  else
  {
    if(hephyMuonISOSFHist == nullptr)
      return doubleUnc(1, 0);

    if(LepPt >= 60)
      LepPt = 59.999;

    auto bin = hephyMuonISOSFHist->FindBin(LepPt);
    val = hephyMuonISOSFHist->GetBinContent(bin);
    unc = hephyMuonISOSFHist->GetBinError(bin);
  }

  doubleUnc retVal(val, unc);
  return retVal;
}

doubleUnc getLeptonTightLooseRatio(double LepID, double LepPt, double LepEta)
{
  double val = 1, unc = 0;
  if(LepPt >= 220)
    LepPt = 219.999;

  if(std::abs(LepID) == 13)
  {
    if(std::abs(LepEta) < 1.5)
    {
      auto bin = muonTightToLooseLowEta->FindBin(LepPt);
      val = muonTightToLooseLowEta->GetBinContent(bin);
      unc = muonTightToLooseLowEta->GetBinError(bin);
    }
    else
    {
      auto bin = muonTightToLooseHighEta->FindBin(LepPt);
      val = muonTightToLooseHighEta->GetBinContent(bin);
      unc = muonTightToLooseHighEta->GetBinError(bin);
    }
  }
  else
  {
    if(std::abs(LepEta) < 1.5)
    {
      auto bin = electronTightToLooseLowEta->FindBin(LepPt);
      val = electronTightToLooseLowEta->GetBinContent(bin);
      unc = electronTightToLooseLowEta->GetBinError(bin);
    }
    else
    {
      auto bin = electronTightToLooseHighEta->FindBin(LepPt);
      val = electronTightToLooseHighEta->GetBinContent(bin);
      unc = electronTightToLooseHighEta->GetBinError(bin);
    }
  }

  doubleUnc retVal(val, unc);
  return retVal;
}



doubleUnc stopCrossSection(double stopM, double lspM)
{
  double val = 0, unc = 0.1;

  int massIndex = (stopM - 100.0)/5.0;

  switch(massIndex)
  {
  case 0:
    val = 1521.11;
    unc = 15.4038;
    break;
  case 1:
    val = 1233.18;
    unc = 15.4059;
    break;
  case 2:
    val = 1013.76;
    unc = 15.4088;
    break;
  case 3:
    val = 832.656;
    unc = 15.1503;
    break;
  case 4:
    val = 689.799;
    unc = 15.044;
    break;
  case 5:
    val = 574.981;
    unc = 14.9895;
    break;
  case 6:
    val = 481.397;
    unc = 14.8906;
    break;
  case 7:
    val = 405.159;
    unc = 14.8952;
    break;
  case 8:
    val = 342.865;
    unc = 14.9119;
    break;
  case 9:
    val = 291.752;
    unc = 14.8022;
    break;
  case 10:
    val = 249.409;
    unc = 14.7477;
    break;
  case 11:
    val = 214.221;
    unc = 14.5928;
    break;
  case 12:
    val = 184.623;
    unc = 14.5821;
    break;
  case 13:
    val = 159.614;
    unc = 14.7859;
    break;
  case 14:
    val = 139.252;
    unc = 14.547;
    break;
  case 15:
    val = 121.416;
    unc = 14.6341;
    break;
  case 16:
    val = 106.194;
    unc = 14.2033;
    break;
  case 17:
    val = 93.3347;
    unc = 14.4893;
    break;
  case 18:
    val = 82.2541;
    unc = 14.4677;
    break;
  case 19:
    val = 72.7397;
    unc = 14.4452;
    break;
  case 20:
    val = 64.5085;
    unc = 14.4098;
    break;
  case 21:
    val = 57.2279;
    unc = 14.4191;
    break;
  case 22:
    val = 50.9226;
    unc = 14.2457;
    break;
  case 23:
    val = 45.3761;
    unc = 14.344;
    break;
  case 24:
    val = 40.5941;
    unc = 14.2634;
    break;
  case 25:
    val = 36.3818;
    unc = 14.2189;
    break;
  case 26:
    val = 32.6679;
    unc = 14.1592;
    break;
  case 27:
    val = 29.3155;
    unc = 14.2233;
    break;
  case 28:
    val = 26.4761;
    unc = 14.1723;
    break;
  case 29:
    val = 23.8853;
    unc = 13.9482;
    break;
  case 30:
    val = 21.5949;
    unc = 14.0595;
    break;
  case 31:
    val = 19.5614;
    unc = 13.8755;
    break;
  case 32:
    val = 17.6836;
    unc = 13.9505;
    break;
  case 33:
    val = 16.112;
    unc = 13.9531;
    break;
  case 34:
    val = 14.6459;
    unc = 13.9278;
    break;
  case 35:
    val = 13.3231;
    unc = 14.2549;
    break;
  case 36:
    val = 12.1575;
    unc = 14.1584;
    break;
  case 37:
    val = 11.0925;
    unc = 14.0904;
    break;
  case 38:
    val = 10.1363;
    unc = 13.8967;
    break;
  case 39:
    val = 9.29002;
    unc = 13.9107;
    break;
  case 40:
    val = 8.51615;
    unc = 13.9223;
    break;
  case 41:
    val = 7.81428;
    unc = 13.8996;
    break;
  case 42:
    val = 7.17876;
    unc = 13.9357;
    break;
  case 43:
    val = 6.60266;
    unc = 13.9256;
    break;
  case 44:
    val = 6.08444;
    unc = 13.7957;
    break;
  case 45:
    val = 5.60471;
    unc = 13.8144;
    break;
  case 46:
    val = 5.17188;
    unc = 13.6954;
    break;
  case 47:
    val = 4.77871;
    unc = 13.7554;
    break;
  case 48:
    val = 4.41629;
    unc = 13.7945;
    break;
  case 49:
    val = 4.08881;
    unc = 13.7075;
    break;
  case 50:
    val = 3.78661;
    unc = 13.6877;
    break;
  case 51:
    val = 3.50911;
    unc = 13.8089;
    break;
  case 52:
    val = 3.25619;
    unc = 13.8002;
    break;
  case 53:
    val = 3.02472;
    unc = 13.7093;
    break;
  case 54:
    val = 2.8077;
    unc = 13.8064;
    break;
  case 55:
    val = 2.61162;
    unc = 13.8477;
    break;
  case 56:
    val = 2.43031;
    unc = 13.6999;
    break;
  case 57:
    val = 2.26365;
    unc = 13.728;
    break;
  case 58:
    val = 2.10786;
    unc = 13.732;
    break;
  case 59:
    val = 1.9665;
    unc = 13.4737;
    break;
  case 60:
    val = 1.83537;
    unc = 13.6985;
    break;
  case 61:
    val = 1.70927;
    unc = 13.7114;
    break;
  case 62:
    val = 1.60378;
    unc = 13.5468;
    break;
  case 63:
    val = 1.49798;
    unc = 13.4453;
    break;
  case 64:
    val = 1.39688;
    unc = 13.6719;
    break;
  case 65:
    val = 1.31169;
    unc = 13.5013;
    break;
  case 66:
    val = 1.22589;
    unc = 13.3237;
    break;
  case 67:
    val = 1.14553;
    unc = 13.5478;
    break;
  case 68:
    val = 1.07484;
    unc = 13.7238;
    break;
  case 69:
    val = 1.01019;
    unc = 13.4187;
    break;
  case 70:
    val = 0.948333;
    unc = 13.4559;
    break;
  case 71:
    val = 0.890847;
    unc = 13.4587;
    break;
  case 72:
    val = 0.836762;
    unc = 13.4468;
    break;
  case 73:
    val = 0.787221;
    unc = 13.4149;
    break;
  case 74:
    val = 0.740549;
    unc = 13.4127;
    break;
  case 75:
    val = 0.697075;
    unc = 13.3926;
    break;
  case 76:
    val = 0.655954;
    unc = 13.4392;
    break;
  case 77:
    val = 0.618562;
    unc = 13.3705;
    break;
  case 78:
    val = 0.582467;
    unc = 13.3914;
    break;
  case 79:
    val = 0.549524;
    unc = 13.3691;
    break;
  case 80:
    val = 0.51848;
    unc = 13.3797;
    break;
  case 81:
    val = 0.489324;
    unc = 13.3608;
    break;
  case 82:
    val = 0.462439;
    unc = 13.3046;
    break;
  case 83:
    val = 0.436832;
    unc = 13.3703;
    break;
  case 84:
    val = 0.412828;
    unc = 13.272;
    break;
  case 85:
    val = 0.390303;
    unc = 13.3443;
    break;
  case 86:
    val = 0.368755;
    unc = 13.3769;
    break;
  case 87:
    val = 0.348705;
    unc = 13.2706;
    break;
  case 88:
    val = 0.330157;
    unc = 13.2981;
    break;
  case 89:
    val = 0.312672;
    unc = 13.277;
    break;
  case 90:
    val = 0.296128;
    unc = 13.2687;
    break;
  case 91:
    val = 0.280734;
    unc = 13.2363;
    break;
  case 92:
    val = 0.266138;
    unc = 13.193;
    break;
  case 93:
    val = 0.251557;
    unc = 13.1731;
    break;
  case 94:
    val = 0.238537;
    unc = 13.3409;
    break;
  case 95:
    val = 0.226118;
    unc = 13.2741;
    break;
  case 96:
    val = 0.214557;
    unc = 13.1697;
    break;
  case 97:
    val = 0.203566;
    unc = 13.3257;
    break;
  case 98:
    val = 0.193079;
    unc = 13.2037;
    break;
  case 99:
    val = 0.183604;
    unc = 13.0973;
    break;
  case 100:
    val = 0.174599;
    unc = 13.2074;
    break;
  case 101:
    val = 0.166131;
    unc = 13.0154;
    break;
  case 102:
    val = 0.158242;
    unc = 13.142;
    break;
  case 103:
    val = 0.150275;
    unc = 13.285;
    break;
  case 104:
    val = 0.142787;
    unc = 13.0642;
    break;
  case 105:
    val = 0.136372;
    unc = 12.7962;
    break;
  case 106:
    val = 0.129886;
    unc = 13.2957;
    break;
  case 107:
    val = 0.123402;
    unc = 13.016;
    break;
  case 108:
    val = 0.11795;
    unc = 12.7132;
    break;
  case 109:
    val = 0.112008;
    unc = 12.808;
    break;
  case 110:
    val = 0.107045;
    unc = 12.9232;
    break;
  case 111:
    val = 0.102081;
    unc = 13.0012;
    break;
  case 112:
    val = 0.09725;
    unc = 12.9038;
    break;
  case 113:
    val = 0.0927515;
    unc = 12.9548;
    break;
  case 114:
    val = 0.0885084;
    unc = 13.0218;
    break;
  case 115:
    val = 0.0844877;
    unc = 13.0703;
    break;
  case 116:
    val = 0.0806192;
    unc = 13.1131;
    break;
  case 117:
    val = 0.0769099;
    unc = 13.1517;
    break;
  case 118:
    val = 0.0734901;
    unc = 13.2344;
    break;
  case 119:
    val = 0.0701805;
    unc = 13.2716;
    break;
  case 120:
    val = 0.0670476;
    unc = 13.3429;
    break;
  case 121:
    val = 0.0641426;
    unc = 13.363;
    break;
  case 122:
    val = 0.0612942;
    unc = 13.3941;
    break;
  case 123:
    val = 0.0585678;
    unc = 13.4663;
    break;
  case 124:
    val = 0.0560753;
    unc = 13.4984;
    break;
  case 125:
    val = 0.0536438;
    unc = 13.5804;
    break;
  case 126:
    val = 0.0513219;
    unc = 13.5682;
    break;
  case 127:
    val = 0.0491001;
    unc = 13.6268;
    break;
  case 128:
    val = 0.0470801;
    unc = 13.6895;
    break;
  case 129:
    val = 0.045061;
    unc = 13.6816;
    break;
  case 130:
    val = 0.0431418;
    unc = 13.7455;
    break;
  case 131:
    val = 0.0413447;
    unc = 13.7833;
    break;
  case 132:
    val = 0.0396264;
    unc = 13.8518;
    break;
  case 133:
    val = 0.0379036;
    unc = 13.8537;
    break;
  case 134:
    val = 0.0363856;
    unc = 13.9334;
    break;
  case 135:
    val = 0.0348796;
    unc = 13.9597;
    break;
  case 136:
    val = 0.0334669;
    unc = 14.0267;
    break;
  case 137:
    val = 0.0320548;
    unc = 14.0406;
    break;
  case 138:
    val = 0.0307373;
    unc = 14.115;
    break;
  case 139:
    val = 0.0295348;
    unc = 14.1397;
    break;
  case 140:
    val = 0.0283338;
    unc = 14.171;
    break;
  case 141:
    val = 0.0272206;
    unc = 14.241;
    break;
  case 142:
    val = 0.0261233;
    unc = 14.2891;
    break;
  case 143:
    val = 0.0251107;
    unc = 14.3632;
    break;
  case 144:
    val = 0.0241099;
    unc = 14.3805;
    break;
  case 145:
    val = 0.0230866;
    unc = 14.4428;
    break;
  case 146:
    val = 0.0221834;
    unc = 14.4791;
    break;
  case 147:
    val = 0.0213766;
    unc = 14.5511;
    break;
  case 148:
    val = 0.0204715;
    unc = 14.6131;
    break;
  case 149:
    val = 0.0197653;
    unc = 14.6602;
    break;
  case 150:
    val = 0.0189612;
    unc = 14.702;
    break;
  case 151:
    val = 0.0182516;
    unc = 14.7648;
    break;
  case 152:
    val = 0.0175509;
    unc = 14.7944;
    break;
  case 153:
    val = 0.0168336;
    unc = 14.8528;
    break;
  case 154:
    val = 0.0162314;
    unc = 14.8772;
    break;
  case 155:
    val = 0.015625;
    unc = 14.9567;
    break;
  case 156:
    val = 0.0150143;
    unc = 15.0389;
    break;
  case 157:
    val = 0.0144112;
    unc = 15.0614;
    break;
  case 158:
    val = 0.0138979;
    unc = 15.1;
    break;
  case 159:
    val = 0.0133962;
    unc = 15.1325;
    break;
  case 160:
    val = 0.0128895;
    unc = 15.2026;
    break;
  case 161:
    val = 0.0123843;
    unc = 15.2968;
    break;
  case 162:
    val = 0.0119837;
    unc = 15.3089;
    break;
  case 163:
    val = 0.0114713;
    unc = 15.3678;
    break;
  case 164:
    val = 0.0110688;
    unc = 15.4082;
    break;
  case 165:
    val = 0.0106631;
    unc = 15.4806;
    break;
  case 166:
    val = 0.0102629;
    unc = 15.5313;
    break;
  case 167:
    val = 0.0098874;
    unc = 15.6066;
    break;
  case 168:
    val = 0.00952142;
    unc = 15.6055;
    break;
  case 169:
    val = 0.00916636;
    unc = 15.6849;
    break;
  case 170:
    val = 0.00883465;
    unc = 15.7177;
    break;
  case 171:
    val = 0.00851073;
    unc = 15.8094;
    break;
  case 172:
    val = 0.00820884;
    unc = 15.844;
    break;
  case 173:
    val = 0.00791403;
    unc = 15.9216;
    break;
  case 174:
    val = 0.00763112;
    unc = 15.9742;
    break;
  case 175:
    val = 0.00735655;
    unc = 16.0548;
    break;
  case 176:
    val = 0.00710317;
    unc = 16.0626;
    break;
  case 177:
    val = 0.00684867;
    unc = 16.144;
    break;
  case 178:
    val = 0.00660695;
    unc = 16.1813;
    break;
  case 179:
    val = 0.00637546;
    unc = 16.2158;
    break;
  case 180:
    val = 0.00615134;
    unc = 16.2953;
    break;
  case 181:
    val = 0.00593765;
    unc = 16.3716;
    break;
  case 182:
    val = 0.00572452;
    unc = 16.3857;
    break;
  case 183:
    val = 0.00553094;
    unc = 16.4628;
    break;
  case 184:
    val = 0.00533968;
    unc = 16.4963;
    break;
  case 185:
    val = 0.00514619;
    unc = 16.5762;
    break;
  case 186:
    val = 0.00497235;
    unc = 16.5838;
    break;
  case 187:
    val = 0.00479906;
    unc = 16.6646;
    break;
  case 188:
    val = 0.00463806;
    unc = 16.6947;
    break;
  case 189:
    val = 0.00447537;
    unc = 16.7071;
    break;
  case 190:
    val = 0.00432261;
    unc = 16.7859;
    break;
  case 191:
    val = 0.00417983;
    unc = 16.8637;
    break;
  case 192:
    val = 0.00403886;
    unc = 16.8981;
    break;
  case 193:
    val = 0.0038962;
    unc = 16.9794;
    break;
  case 194:
    val = 0.00376343;
    unc = 16.9764;
    break;
  case 195:
    val = 0.00364174;
    unc = 17.0634;
    break;
  case 196:
    val = 0.00352093;
    unc = 17.0908;
    break;
  case 197:
    val = 0.00339813;
    unc = 17.1929;
    break;
  case 198:
    val = 0.00328695;
    unc = 17.2274;
    break;
  case 199:
    val = 0.00317628;
    unc = 17.2617;
    break;
  case 200:
    val = 0.00307413;
    unc = 17.3377;
    break;
  case 201:
    val = 0.00297377;
    unc = 17.3822;
    break;
  case 202:
    val = 0.00287148;
    unc = 17.4725;
    break;
  case 203:
    val = 0.00278078;
    unc = 17.5091;
    break;
  case 204:
    val = 0.00268873;
    unc = 17.5883;
    break;
  case 205:
    val = 0.00260821;
    unc = 17.6126;
    break;
  case 206:
    val = 0.00251529;
    unc = 17.6836;
    break;
  case 207:
    val = 0.00243484;
    unc = 17.7128;
    break;
  case 208:
    val = 0.00236295;
    unc = 17.7977;
    break;
  case 209:
    val = 0.00228192;
    unc = 17.8507;
    break;
  case 210:
    val = 0.00221047;
    unc = 17.9259;
    break;
  case 211:
    val = 0.00213907;
    unc = 18.0255;
    break;
  case 212:
    val = 0.00206845;
    unc = 18.0518;
    break;
  case 213:
    val = 0.0020063;
    unc = 18.0954;
    break;
  case 214:
    val = 0.00194569;
    unc = 18.1194;
    break;
  case 215:
    val = 0.0018741;
    unc = 18.2145;
    break;
  case 216:
    val = 0.00182266;
    unc = 18.3074;
    break;
  case 217:
    val = 0.00176211;
    unc = 18.3375;
    break;
  case 218:
    val = 0.00170006;
    unc = 18.4075;
    break;
  case 219:
    val = 0.00164968;
    unc = 18.4438;
    break;
  case 220:
    val = 0.00159844;
    unc = 18.5209;
    break;
  case 221:
    val = 0.0015472;
    unc = 18.5977;
    break;
  case 222:
    val = 0.00149657;
    unc = 18.6485;
    break;
  case 223:
    val = 0.00145544;
    unc = 18.7347;
    break;
  case 224:
    val = 0.00140288;
    unc = 18.8774;
    break;
  case 225:
    val = 0.00136155;
    unc = 18.989;
    break;
  case 226:
    val = 0.00131271;
    unc = 18.8763;
    break;
  case 227:
    val = 0.0012717;
    unc = 18.9588;
    break;
  case 228:
    val = 0.00123066;
    unc = 19.049;
    break;
  case 229:
    val = 0.00119994;
    unc = 19.1442;
    break;
  case 230:
    val = 0.0011583;
    unc = 19.3006;
    break;
  case 231:
    val = 0.00112694;
    unc = 19.4441;
    break;
  case 232:
    val = 0.00108716;
    unc = 19.4141;
    break;
  case 233:
    val = 0.00105517;
    unc = 19.6361;
    break;
  case 234:
    val = 0.00102241;
    unc = 19.6297;
    break;
  case 235:
    val = 0.000991293;
    unc = 19.762;
    break;
  case 236:
    val = 0.000961012;
    unc = 19.7926;
    break;
  case 237:
    val = 0.000932394;
    unc = 19.8682;
    break;
  case 238:
    val = 0.000903404;
    unc = 19.9924;
    break;
  case 239:
    val = 0.000876957;
    unc = 20.0777;
    break;
  case 240:
    val = 0.000850345;
    unc = 20.1604;
    break;
  case 241:
    val = 0.00082443;
    unc = 20.2883;
    break;
  case 242:
    val = 0.00079983;
    unc = 20.373;
    break;
  case 243:
    val = 0.000775222;
    unc = 20.4622;
    break;
  case 244:
    val = 0.000751372;
    unc = 20.5919;
    break;
  case 245:
    val = 0.000728912;
    unc = 20.6884;
    break;
  case 246:
    val = 0.000706867;
    unc = 20.7763;
    break;
  case 247:
    val = 0.000685372;
    unc = 20.8587;
    break;
  case 248:
    val = 0.000664649;
    unc = 20.9879;
    break;
  case 249:
    val = 0.000644804;
    unc = 21.1487;
    break;
  case 250:
    val = 0.000625155;
    unc = 21.2761;
    break;
  case 251:
    val = 0.000606802;
    unc = 21.3529;
    break;
  case 252:
    val = 0.000588512;
    unc = 21.4428;
    break;
  case 253:
    val = 0.000570506;
    unc = 21.6584;
    break;
  case 254:
    val = 0.000553379;
    unc = 21.6036;
    break;
  case 255:
    val = 0.000536646;
    unc = 21.775;
    break;
  case 256:
    val = 0.000521404;
    unc = 21.8383;
    break;
  case 257:
    val = 0.000505008;
    unc = 21.9675;
    break;
  case 258:
    val = 0.000490353;
    unc = 22.1444;
    break;
  case 259:
    val = 0.000476164;
    unc = 22.2016;
    break;
  case 260:
    val = 0.000461944;
    unc = 22.2704;
    break;
  case 261:
    val = 0.000448172;
    unc = 22.4911;
    break;
  case 262:
    val = 0.000435082;
    unc = 22.5606;
    break;
  case 263:
    val = 0.000422967;
    unc = 22.6095;
    break;
  case 264:
    val = 0.000410381;
    unc = 22.797;
    break;
  case 265:
    val = 0.000398106;
    unc = 22.8949;
    break;
  case 266:
    val = 0.000386792;
    unc = 23.1319;
    break;
  case 267:
    val = 0.000375724;
    unc = 23.1724;
    break;
  case 268:
    val = 0.000364616;
    unc = 23.2234;
    break;
  case 269:
    val = 0.000353965;
    unc = 23.4637;
    break;
  case 270:
    val = 0.000343923;
    unc = 23.4948;
    break;
  case 271:
    val = 0.000333885;
    unc = 23.5468;
    break;
  case 272:
    val = 0.000324344;
    unc = 23.771;
    break;
  case 273:
    val = 0.0003153;
    unc = 23.8004;
    break;
  case 274:
    val = 0.00030583;
    unc = 24.0064;
    break;
  case 275:
    val = 0.000296811;
    unc = 24.0314;
    break;
  case 276:
    val = 0.000288149;
    unc = 23.9248;
    break;
  case 277:
    val = 0.000279711;
    unc = 24.1257;
    break;
  case 278:
    val = 0.000271724;
    unc = 24.1274;
    break;
  case 279:
    val = 0.000264275;
    unc = 24.3545;
    break;
  case 280:
    val = 0.000256248;
    unc = 24.372;
    break;
  case 281:
    val = 0.000248853;
    unc = 24.5827;
    break;
  case 282:
    val = 0.000241844;
    unc = 24.6187;
    break;
  case 283:
    val = 0.000234438;
    unc = 24.8442;
    break;
  case 284:
    val = 0.000227374;
    unc = 24.8909;
    break;
  case 285:
    val = 0.000221045;
    unc = 25.0895;
    break;
  case 286:
    val = 0.000214431;
    unc = 24.8728;
    break;
  case 287:
    val = 0.000208092;
    unc = 25.1043;
    break;
  case 288:
    val = 0.000201748;
    unc = 25.3207;
    break;
  case 289:
    val = 0.000196399;
    unc = 25.5641;
    break;
  case 290:
    val = 0.000190474;
    unc = 25.5213;
    break;
  case 291:
    val = 0.000185188;
    unc = 25.7329;
    break;
  case 292:
    val = 0.000179263;
    unc = 25.6931;
    break;
  case 293:
    val = 0.000174021;
    unc = 25.9111;
    break;
  case 294:
    val = 0.000169176;
    unc = 25.8106;
    break;
  case 295:
    val = 0.000163861;
    unc = 26.0597;
    break;
  case 296:
    val = 0.000159583;
    unc = 26.2958;
    break;
  case 297:
    val = 0.000154719;
    unc = 26.195;
    break;
  case 298:
    val = 0.000150506;
    unc = 26.4111;
    break;
  case 299:
    val = 0.000145626;
    unc = 26.3077;
    break;
  case 300:
    val = 0.000141382;
    unc = 26.5291;
    break;
  case 301:
    val = 0.000137131;
    unc = 26.7424;
    break;
  case 302:
    val = 0.000132187;
    unc = 26.668;
    break;
  case 303:
    val = 0.000127929;
    unc = 26.9117;
    break;
  case 304:
    val = 0.000124086;
    unc = 26.7738;
    break;
  case 305:
    val = 0.00011982;
    unc = 27.0483;
    break;
  case 306:
    val = 0.000116042;
    unc = 26.8071;
    break;
  case 307:
    val = 0.000112767;
    unc = 27.127;
    break;
  case 308:
    val = 0.000108936;
    unc = 26.9351;
    break;
  case 309:
    val = 0.000105746;
    unc = 27.1783;
    break;
  case 310:
    val = 0.000102693;
    unc = 27.292;
    break;
  case 311:
    val = 0.000100112;
    unc = 27.4445;
    break;
  case 312:
    val = 9.75763e-05;
    unc = 27.5431;
    break;
  case 313:
    val = 9.52062e-05;
    unc = 27.6946;
    break;
  case 314:
    val = 9.29857e-05;
    unc = 27.7869;
    break;
  case 315:
    val = 9.08285e-05;
    unc = 27.9347;
    break;
  case 316:
    val = 8.87433e-05;
    unc = 28.1539;
    break;
  case 317:
    val = 8.66618e-05;
    unc = 28.3509;
    break;
  case 318:
    val = 8.46535e-05;
    unc = 28.4432;
    break;
  case 319:
    val = 8.27102e-05;
    unc = 28.591;
    break;
  case 320:
    val = 8.07774e-05;
    unc = 28.7497;
    break;
  case 321:
    val = 7.8666e-05;
    unc = 28.8194;
    break;
  case 322:
    val = 7.6572e-05;
    unc = 29.0265;
    break;
  case 323:
    val = 7.45994e-05;
    unc = 29.1193;
    break;
  case 324:
    val = 7.25199e-05;
    unc = 29.3013;
    break;
  case 325:
    val = 7.05189e-05;
    unc = 29.3697;
    break;
  case 326:
    val = 6.85712e-05;
    unc = 29.4972;
    break;
  case 327:
    val = 6.67296e-05;
    unc = 29.6167;
    break;
  case 328:
    val = 6.49184e-05;
    unc = 29.7686;
    break;
  case 329:
    val = 6.30949e-05;
    unc = 29.8524;
    break;
  case 330:
    val = 6.13637e-05;
    unc = 29.9789;
    break;
  case 331:
    val = 5.97301e-05;
    unc = 30.0928;
    break;
  case 332:
    val = 5.80751e-05;
    unc = 30.2585;
    break;
  case 333:
    val = 5.65479e-05;
    unc = 30.366;
    break;
  case 334:
    val = 5.49998e-05;
    unc = 30.5241;
    break;
  case 335:
    val = 5.35686e-05;
    unc = 30.6718;
    break;
  case 336:
    val = 5.20828e-05;
    unc = 30.6799;
    break;
  case 337:
    val = 5.07079e-05;
    unc = 30.9201;
    break;
  case 338:
    val = 4.93948e-05;
    unc = 31.0043;
    break;
  case 339:
    val = 4.80635e-05;
    unc = 31.138;
    break;
  case 340:
    val = 4.67492e-05;
    unc = 31.2291;
    break;
  case 341:
    val = 4.55055e-05;
    unc = 31.4321;
    break;
  case 342:
    val = 4.42835e-05;
    unc = 31.5499;
    break;
  case 343:
    val = 4.30744e-05;
    unc = 31.6302;
    break;
  case 344:
    val = 4.19954e-05;
    unc = 31.7151;
    break;
  case 345:
    val = 4.08527e-05;
    unc = 31.9048;
    break;
  case 346:
    val = 3.97561e-05;
    unc = 31.9718;
    break;
  case 347:
    val = 3.87041e-05;
    unc = 32.2028;
    break;
  case 348:
    val = 3.76008e-05;
    unc = 32.268;
    break;
  case 349:
    val = 3.66914e-05;
    unc = 32.4529;
    break;
  case 350:
    val = 3.56995e-05;
    unc = 32.5039;
    break;
  case 351:
    val = 3.47689e-05;
    unc = 32.6767;
    break;
  case 352:
    val = 3.38528e-05;
    unc = 32.8878;
    break;
  case 353:
    val = 3.29644e-05;
    unc = 32.8975;
    break;
  case 354:
    val = 3.20679e-05;
    unc = 32.9608;
    break;
  case 355:
    val = 3.12583e-05;
    unc = 33.1541;
    break;
  case 356:
    val = 3.04342e-05;
    unc = 33.3117;
    break;
  case 357:
    val = 2.96516e-05;
    unc = 33.2866;
    break;
  case 358:
    val = 2.88952e-05;
    unc = 33.6279;
    break;
  case 359:
    val = 2.81145e-05;
    unc = 33.6845;
    break;
  case 360:
    val = 2.73974e-05;
    unc = 33.8247;
    break;
  case 361:
    val = 2.66796e-05;
    unc = 33.9708;
    break;
  case 362:
    val = 2.59941e-05;
    unc = 33.9526;
    break;
  case 363:
    val = 2.52784e-05;
    unc = 34.1137;
    break;
  case 364:
    val = 2.46598e-05;
    unc = 34.2714;
    break;
  case 365:
    val = 2.39932e-05;
    unc = 34.2328;
    break;
  case 366:
    val = 2.33737e-05;
    unc = 34.394;
    break;
  case 367:
    val = 2.27623e-05;
    unc = 34.5138;
    break;
  case 368:
    val = 2.21454e-05;
    unc = 34.6933;
    break;
  case 369:
    val = 2.15924e-05;
    unc = 35.0815;
    break;
  case 370:
    val = 2.10232e-05;
    unc = 34.9444;
    break;
  case 371:
    val = 2.05211e-05;
    unc = 35.0155;
    break;
  case 372:
    val = 1.98996e-05;
    unc = 35.2135;
    break;
  case 373:
    val = 1.9408e-05;
    unc = 35.3328;
    break;
  case 374:
    val = 1.88974e-05;
    unc = 35.4643;
    break;
  case 375:
    val = 1.84612e-05;
    unc = 35.7904;
    break;
  case 376:
    val = 1.79562e-05;
    unc = 35.8898;
    break;
  case 377:
    val = 1.75673e-05;
    unc = 35.989;
    break;
  case 378:
    val = 1.70612e-05;
    unc = 36.0953;
    break;
  case 379:
    val = 1.66228e-05;
    unc = 36.4709;
    break;
  case 380:
    val = 1.62355e-05;
    unc = 36.5277;
    break;
  default:
    val = 0;
    unc = 20;
  }
  unc = unc/100;

  doubleUnc retVal(val, val*unc);
  return retVal;
}
