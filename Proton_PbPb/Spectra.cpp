// Spectra.cpp
// This macro computes fully corrected spectra

#include <Riostream.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2F.h>
#include <TStyle.h>
#include <TLatex.h>
#include <TCanvas.h>

#include "../utils/Utils.h"
#include "../utils/Config.h"

using utils::TTList;
using namespace proton;

void Spectra(const char *cutSettings = "", const double roi_nsigma = 8., const bool G3G4Prim = false, const bool binCounting = false, const int bkg_shape = 1, const bool sigmoidCorrection = true, const char *histoNameDir = ".", const char *outFileName = "SpectraProton1", const char *outFileOption = "recreate", const char *dataFile = "AnalysisResults", const char *signalFile = "SignalProton", const char *effFile = "EfficiencyProton", const char *primFile = "PrimaryProton", const bool useEfficiencyMB = false)
{
  if (kVerbose) std::cout << "cutSettings = " << cutSettings << std::endl;
  gStyle->SetOptFit(0);
  gStyle->SetOptStat(0);
  gStyle->SetTextFont(44);

  TH2F *fNevents;
  TFile *inFileDat = TFile::Open(Form("%s/%s_kINT7_10_30_50_90.root", kDataDir, dataFile)); //_largeNsigma_cutDCAxyChi2TPC
  TTList *fMultList = (TTList *)inFileDat->Get("nuclei_proton_");
  fNevents = (TH2F *)fMultList->Get("fNormalisationHist");
  TFile *inFileRaw = TFile::Open(Form("%s/%s.root", kOutDir, signalFile));
  TFile *inFileEff = TFile::Open(Form("%s/%s.root", kOutDir, effFile));
  TFile *inFileSec = TFile::Open(Form("%s/%s.root", kOutDir, primFile));
  if (!inFileRaw || !inFileEff || !inFileSec)
  {
    if (kVerbose) std::cout << "Input files do not exist!" << std::endl;
    return;
  }

  int iNsigma = 0;
  if (roi_nsigma > 7.8 && roi_nsigma < 8.1) iNsigma = 1;
  else if (roi_nsigma > 8.1) iNsigma = 2;

  TFile outFile(Form("%s/%s.root", kOutDir, outFileName), outFileOption);
  TString sFileOption(outFileOption);
  if (outFile.GetDirectory(histoNameDir) && !sFileOption.CompareTo("update")){
    std::cout << "Directory already present in file!" << std::endl;
    return;
  }
  outFile.mkdir(histoNameDir);

  TH1D *fRatio[kNCentClasses];
  TH1D *fRatioUncertainty[kNCentClasses];
  for (int iCent = 0; iCent < kNCentClasses; ++iCent)
  {
    fRatio[iCent] = new TH1D(*(TH1D *)inFileRaw->Get(Form("%s_%d_%d_%d/fATOFrawYield_%.0f_%.0f", cutSettings, binCounting, bkg_shape, iNsigma, kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1])));
    fRatio[iCent]->Reset();
    fRatio[iCent]->SetName(Form("fRatio_%.0f_%.0f", kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
    fRatio[iCent]->SetTitle("");
  }

  TH1D *fSpectra[2];
  for (int iCent = 0; iCent < kNCentClasses; ++iCent)
  {
    TH1D *norm;
    norm = fNevents->ProjectionY("norm", kCentBinsProton[iCent][0], kCentBinsProton[iCent][1]);

    // compute corrected spectra
    for (int iMatt = 0; iMatt < 2; ++iMatt)
    {
      outFile.cd(histoNameDir);
      double cent_bin_lim_min = kCentBinsLimitsProton[iCent][0], cent_bin_lim_max = kCentBinsLimitsProton[iCent][1];
      if (useEfficiencyMB){
        cent_bin_lim_min = 0.;
        cent_bin_lim_max = 90.;
      }
      TH1D *eff = (TH1D *)inFileEff->Get(Form("%s_/f%sEff_TOF_%.0f_%.0f", cutSettings, kAntimatterMatter[iMatt], cent_bin_lim_min, cent_bin_lim_max));
      TF1 *sec_f = (TF1 *)inFileSec->Get(Form("f%sFunctionFit_%.0f_%.0f", kAntimatterMatter[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      TH1D *sec = (TH1D *)inFileSec->Get(Form("f%sPrimFrac_%.0f_%.0f", kAntimatterMatter[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      TH1D *raw = (TH1D *)inFileRaw->Get(Form("%s_%d_%d_%d/f%sTOFrawYield_%.0f_%.0f", cutSettings, binCounting, bkg_shape, iNsigma, kAntimatterMatter[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));

      //sec->Fit(&fitFuncSec,"R");
      fSpectra[iMatt] = new TH1D(*raw);
      int pTbinMax = 42;
      if (kVerbose) std::cout<<"entering pt loop..."<<std::endl;
      for (int iPtBin = 5; iPtBin < pTbinMax + 1; ++iPtBin)
      {
        double rawYield = raw->GetBinContent(iPtBin);
        double rawYieldError = raw->GetBinError(iPtBin);
        double efficiency = eff->GetBinContent(eff->FindBin(raw->GetBinCenter(iPtBin)));
        double efficiencyError = eff->GetBinError(eff->FindBin(raw->GetBinCenter(iPtBin)));
        
        double primary = 0.;
        double primaryError = 0.;
        if (!sigmoidCorrection) {
          primary = sec->GetBinContent(iPtBin);
          primaryError = sec->GetBinError(iPtBin);
        }
        else {
          primary = sec_f->Eval(raw->GetXaxis()->GetBinCenter(iPtBin));
        }
        if (rawYield<1.e-7 || efficiency<1.e-2 || efficiency >1. || primary>0.999 || primary<0.5) {
          fSpectra[iMatt]->SetBinContent(iPtBin, 0.);
          fSpectra[iMatt]->SetBinError(iPtBin, 0);
        }
        else {
          fSpectra[iMatt]->SetBinContent(iPtBin, primary*rawYield/efficiency );
          fSpectra[iMatt]->SetBinError(iPtBin, primary*rawYield/efficiency*sqrt(efficiencyError*efficiencyError/efficiency/efficiency+primaryError*primaryError/primary/primary)); //primary*rawYieldError/efficiency);
        }
        if (kVerbose) std::cout<<"eff="<<efficiency<<"; raw="<<rawYield<<"; rawError="<<rawYieldError<<"; primary="<<primary<<std::endl;
      }
      fSpectra[iMatt]->SetName(Form("f%sSpectra_%.0f_%.0f", kAntimatterMatter[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      fSpectra[iMatt]->SetTitle(Form("%s, %.0f-%.0f%%", kAntimatterMatterLabel[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      fSpectra[iMatt]->GetYaxis()->SetTitle("1/#it{N}_{ev} d^{2}#it{N}/d#it{p}_{T}d#it{y} (GeV/#it{c})^{-1}");
      fSpectra[iMatt]->GetXaxis()->SetTitle(kAxisTitlePt);

      // scale by number of events
      double events = norm->GetBinContent(4);
      fSpectra[iMatt]->Scale(1. / events, "width");

      // write to file
      fSpectra[iMatt]->Write();
    }

    // compute ratios
    int pTbinMax = 42;
    for (int iPtBin = 5; iPtBin < pTbinMax + 1; ++iPtBin)
    {
      double antiSpec = fSpectra[0]->GetBinContent(iPtBin);
      double spec = fSpectra[1]->GetBinContent(iPtBin);
      double antiSpecErr = fSpectra[0]->GetBinError(iPtBin);
      double specErr = fSpectra[1]->GetBinError(iPtBin);
      if (spec > 1.e-8 && antiSpec > 1.e-8)
      {
        fRatio[iCent]->SetBinContent(iPtBin, antiSpec / spec);
        fRatio[iCent]->SetBinError(iPtBin, antiSpec / spec * TMath::Sqrt(antiSpecErr * antiSpecErr / antiSpec / antiSpec + specErr * specErr / spec / spec));
      }
    }
    fRatio[iCent]->GetXaxis()->SetTitle(kAxisTitlePt);
    fRatio[iCent]->GetYaxis()->SetTitle(Form("Ratio %s/%s", kAntimatterMatterLabel[0], kAntimatterMatterLabel[1]));
    fRatio[iCent]->SetTitle(Form("%.0f-%.0f%%", kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
    //fRatio[iCent]->Fit("pol0");
    fRatio[iCent]->GetXaxis()->SetRangeUser(1.0,2.0);
    fRatio[iCent]->Write();
  }
  outFile.Close();
}
