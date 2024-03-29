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
using namespace pion;

void Spectra(const char *cutSettings = "", const double roi_nsigma_down = 1.5, const double roi_nsigma_up = 11., const double mismatch_nsigma_down = 8.5, const double mismatch_nsigma_up = 13.5, const bool binCounting = false, const int bkg_shape = 1, const bool sigmoidCorrection = true, const char *histoNameDir = ".", const char *outFileName = "SpectraPion1", const char *outFileOption = "recreate", const char *dataFile = "AnalysisResults", const char *signalFile = "SignalPion", const char *effFile = "EfficiencyPion", const char *primFile = "PrimaryPion", const bool sys=false,const bool useEfficiencyMB = false)
{
  if (kVerbose) std::cout << "cutSettings = " << cutSettings << std::endl;
  gStyle->SetOptFit(0);
  gStyle->SetOptStat(0);
  gStyle->SetTextFont(44);

  if(sys)if (kVerbose) std::cout<<"Using sys = true"<<std::endl;

  TH2F *fNevents;
  TFile *inFileDat = TFile::Open(Form("%s/%s_largeNsigma_cutDCAxyChi2TPC.root", kDataDir, dataFile));
  TTList *fMultList = (TTList *)inFileDat->Get("nuclei_pion_");
  fNevents = (TH2F *)fMultList->Get("fNormalisationHist");
  TFile *inFileRaw = TFile::Open(Form("%s/%s.root", kOutDir, signalFile));
  TFile *inFileEff = TFile::Open(Form("%s/%s.root", kOutDir, effFile));
  TFile *inFileSec = TFile::Open(Form("%s/%s.root", kOutDir, primFile));
  TFile *inFileSys;
  if(sys)inFileSys = TFile::Open(Form("%s/SystematicsAllEPt.root", kOutDir));
  if (!inFileRaw || !inFileEff || !inFileSec)
  {
    if (kVerbose) std::cout << "Input files do not exist!" << std::endl;
    return;
  }
 
  int iNsigmaDown = 0;
  if (roi_nsigma_down > 1.4 && roi_nsigma_down < 1.6) iNsigmaDown = 1;
  else if (roi_nsigma_down > 1.9) iNsigmaDown = 2; 
  int iNsigmaUp = 0;
  if (roi_nsigma_up > 10.9 && roi_nsigma_up < 11.1) iNsigmaUp = 1;
  else if (roi_nsigma_up > 11.9) iNsigmaUp = 2;
  int iNsigmaMismatchDown = 0;
  if (mismatch_nsigma_down > 8.4 && mismatch_nsigma_down < 8.6) iNsigmaMismatchDown = 1;
  else if (mismatch_nsigma_down > 8.9) iNsigmaMismatchDown = 2;
  int iNsigmaMismatchUp = 0;
  if (mismatch_nsigma_up > 13.4 && mismatch_nsigma_up < 13.6) iNsigmaMismatchUp = 1;
  else if (mismatch_nsigma_up > 13.9) iNsigmaMismatchUp = 2;

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
    // if (kVerbose) std::cout << "read: " << Form("%s_%d_%d/fATOFrawYield_%.0f_%.0f", cutSettings, binCounting, bkg_shape, kCentBinsLimitsPion[iCent][0], kCentBinsLimitsPion[iCent][1]) << std::endl;
    fRatio[iCent] = new TH1D(*(TH1D *)inFileRaw->Get(Form("%s_%d_%d_%d_%d_%d_%d/fATOFrawYield_%.0f_%.0f", cutSettings, binCounting, bkg_shape, iNsigmaDown, iNsigmaUp, iNsigmaMismatchDown, iNsigmaMismatchUp, kCentBinsLimitsPion[iCent][0], kCentBinsLimitsPion[iCent][1])));
    fRatio[iCent]->Reset();
    fRatio[iCent]->SetName(Form("fRatio_%.0f_%.0f", kCentBinsLimitsPion[iCent][0], kCentBinsLimitsPion[iCent][1]));
    fRatio[iCent]->SetTitle("");
  }

  TH1D *fSpectra[2];
  for (int iCent = 0; iCent < kNCentClasses; ++iCent)
  {
    TH1D *norm;
    norm = fNevents->ProjectionY("norm", kCentBinsPion[iCent][0], kCentBinsPion[iCent][1]);
    TH1D *h_sys, *h_ratio_from_var;
    if(sys) {
      h_ratio_from_var= (TH1D *)inFileSys->Get(Form("fRatioFromVariationsTot_%.0f_%.0f", kCentBinsLimitsPion[iCent][0], kCentBinsLimitsPion[iCent][1]));
      h_sys= (TH1D *)inFileSys->Get(Form("fSystematicUncertaintyTot_%.0f_%.0f", kCentBinsLimitsPion[iCent][0], kCentBinsLimitsPion[iCent][1]));
    }

    // compute corrected spectra
    for (int iMatt = 0; iMatt < 2; ++iMatt)
    {
      outFile.cd(histoNameDir);
      double cent_bin_lim_min = kCentBinsLimitsPion[iCent][0], cent_bin_lim_max = kCentBinsLimitsPion[iCent][1];
      if (useEfficiencyMB){
        cent_bin_lim_min = 0.;
        cent_bin_lim_max = 90.;
      }
      TH1D *eff = (TH1D *)inFileEff->Get(Form("%s_/f%sEff_TOF_%.0f_%.0f", cutSettings, kAntimatterMatter[iMatt], cent_bin_lim_min, cent_bin_lim_max));
      TF1 *sec_f = (TF1 *)inFileSec->Get(Form("f%sFunctionFit_%.0f_%.0f", kAntimatterMatter[iMatt], kCentBinsLimitsPion[iCent][0], kCentBinsLimitsPion[iCent][1]));
      TH1D *sec = (TH1D *)inFileSec->Get(Form("f%sPrimFrac_%.0f_%.0f", kAntimatterMatter[iMatt], kCentBinsLimitsPion[iCent][0], kCentBinsLimitsPion[iCent][1]));
      TH1D *raw = (TH1D *)inFileRaw->Get(Form("%s_%d_%d_%d_%d_%d_%d/f%sTOFrawYield_%.0f_%.0f", cutSettings, binCounting, bkg_shape, iNsigmaDown, iNsigmaUp, iNsigmaMismatchDown, iNsigmaMismatchUp, kAntimatterMatter[iMatt], kCentBinsLimitsPion[iCent][0], kCentBinsLimitsPion[iCent][1]));

      fSpectra[iMatt] = new TH1D(*raw);
      int pTbinMax = 27;
      if (kVerbose) std::cout<<"entering pt loop..."<<std::endl;
      for (int iPtBin = 7; iPtBin < pTbinMax + 1; ++iPtBin)
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
        if (rawYield<1.e-7 || efficiency<1.e-2 || efficiency >1. || primary>0.999 || primary < 0.7) {
          fSpectra[iMatt]->SetBinContent(iPtBin, 0.);
          fSpectra[iMatt]->SetBinError(iPtBin, 0);
        }
        else {
          fSpectra[iMatt]->SetBinContent(iPtBin, primary* rawYield/ efficiency );
          //fSpectra[iMatt]->SetBinError(iPtBin, primary*rawYieldError/efficiency);
          fSpectra[iMatt]->SetBinError(iPtBin, primary* rawYield / efficiency * sqrt(efficiencyError*efficiencyError/efficiency/efficiency + primaryError*primaryError/primary/primary));
        }
        if (kVerbose) std::cout<<"eff="<<efficiency<<"; raw="<<rawYield<<"; rawError="<<rawYieldError<<"; primary="<<primary<<std::endl;
      }
      fSpectra[iMatt]->SetName(Form("f%sSpectra_%.0f_%.0f", kAntimatterMatter[iMatt], kCentBinsLimitsPion[iCent][0], kCentBinsLimitsPion[iCent][1]));
      fSpectra[iMatt]->SetTitle(Form("%s, %.0f-%.0f%%", kAntimatterMatterLabel[iMatt], kCentBinsLimitsPion[iCent][0], kCentBinsLimitsPion[iCent][1]));
      fSpectra[iMatt]->GetYaxis()->SetTitle("1/#it{N}_{ev} d^{2}#it{N}/d#it{p}_{T}d#it{y} (GeV/#it{c})^{-1}");
      fSpectra[iMatt]->GetXaxis()->SetTitle(kAxisTitlePt);

      // scale by number of events
      double events = norm->GetBinContent(4);
      fSpectra[iMatt]->Scale(1. / events, "width");

      // write to file
      fSpectra[iMatt]->Write();
    }

    // compute ratios
    TH1D SysError(Form("fSysError_%.0f_%.0f",kCentBinsLimitsPion[iCent][0], kCentBinsLimitsPion[iCent][1]),Form("%.0f-%.0f%%",kCentBinsLimitsPion[iCent][0], kCentBinsLimitsPion[iCent][1]),kNPtBins,kPtBins);
    int pTbinMax = 27;
    for (int iPtBin = 7; iPtBin < pTbinMax + 1; ++iPtBin)
    {
      double antiSpec = fSpectra[0]->GetBinContent(iPtBin);
      double spec = fSpectra[1]->GetBinContent(iPtBin);
      double antiSpecErr = fSpectra[0]->GetBinError(iPtBin);
      double specErr = fSpectra[1]->GetBinError(iPtBin);
      if (spec > 1.e-8 && antiSpec > 1.e-8)
      {
        fRatio[iCent]->SetBinContent(iPtBin, antiSpec / spec);
        fRatio[iCent]->SetBinError(iPtBin, antiSpec / spec * TMath::Sqrt(antiSpecErr * antiSpecErr / antiSpec / antiSpec + specErr * specErr / spec / spec));
        //if (kVerbose) std::cout<<h_sys->GetBinContent(iPtBin)<<std::endl;
        if(sys){
          fRatio[iCent]->SetBinContent(iPtBin,h_ratio_from_var->GetBinContent(iPtBin));
          double sys_err = h_sys->GetBinContent(iPtBin);
          double prim_err = fRatio[iCent]->GetBinError(iPtBin);
          fRatio[iCent]->SetBinError(iPtBin,TMath::Sqrt(sys_err*sys_err+prim_err*prim_err)*fRatio[iCent]->GetBinContent(iPtBin));
        }
      }
      SysError.SetBinContent(iPtBin,fRatio[iCent]->GetBinError(iPtBin));
      SysError.SetBinError(0,fRatio[iCent]->GetBinError(iPtBin));
    }
    fRatio[iCent]->GetXaxis()->SetTitle(kAxisTitlePt);
    fRatio[iCent]->GetYaxis()->SetTitle(Form("Ratio %s/%s", kAntimatterMatterLabel[0], kAntimatterMatterLabel[1]));
    fRatio[iCent]->SetTitle(Form("%.0f-%.0f%%", kCentBinsLimitsPion[iCent][0], kCentBinsLimitsPion[iCent][1]));
    //fRatio[iCent]->Fit("pol0");
    fRatio[iCent]->GetXaxis()->SetRangeUser(0.5,1.25);
    fRatio[iCent]->Write();

    SysError.GetXaxis()->SetRangeUser(0.5,1.25);
    SysError.GetXaxis()->SetTitle("#it{p}_{T} (GeV/#it{c})");
    SysError.GetYaxis()->SetTitle("Systematic Uncertainty");
    SysError.SetMinimum(0.);
    TCanvas cSysError(SysError.GetName(),SysError.GetTitle());
    SysError.Draw("histo");
    cSysError.Print(Form("%s.pdf",SysError.GetName()));
    SysError.Write();
  }
  outFile.Close();
}
