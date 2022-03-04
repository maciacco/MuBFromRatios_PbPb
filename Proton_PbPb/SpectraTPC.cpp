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

double protonCorrectionPt(int iMatt,double pt){
  if (iMatt == 1) 
    return 1;
    //return 0.99876*TMath::Power(pt,0.00036);
  //return 1.03176*TMath::Power(pt,-0.01249);
  return 1;
};

void SpectraTPC(const char *cutSettings = "", const double roi_nsigma = 8., const bool G3G4Prim = false, const bool binCounting = false, const int bkg_shape = 1, const bool sigmoidCorrection = true, const char *histoNameDir = ".", const char *outFileName = "SpectraProton1", const char *outFileOption = "recreate", const char *dataFile = "AnalysisResults", const char *signalFile = "SignalProton", const char *effFile = "EfficiencyProton", const char *primFile = "PrimaryProton", const bool sys=false,const bool useEfficiencyMB = false)
{
  if (kVerbose) std::cout << "cutSettings = " << cutSettings << std::endl;
  gStyle->SetOptFit(0);
  gStyle->SetOptStat(0);
  gStyle->SetTextFont(44);

  TH2F *fNevents;
  TFile *inFileDat = TFile::Open(Form("%s/%s_largeNsigma_cutDCAxyChi2TPC_lowPt.root", kDataDir, dataFile));
  TTList *fMultList = (TTList *)inFileDat->Get("nuclei_proton_");
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
    // if (kVerbose) std::cout << "read: " << Form("%s_%d_%d/fATPCrawYield_%.0f_%.0f", cutSettings, binCounting, bkg_shape, kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]) << std::endl;
    fRatio[iCent] = new TH1D(*(TH1D *)inFileRaw->Get(Form("%s_%d_%d_%d/fATPCrawYield_%.0f_%.0f", cutSettings, binCounting, bkg_shape, iNsigma, kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1])));
    fRatio[iCent]->Reset();
    fRatio[iCent]->SetName(Form("fRatio_%.0f_%.0f", kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
    fRatio[iCent]->SetTitle("");
  }

  TH1D *fSpectra[2];
  for (int iCent = 0; iCent < kNCentClasses; ++iCent)
  {
    TH1D *norm;
    norm = fNevents->ProjectionY("norm", kCentBinsProton[iCent][0], kCentBinsProton[iCent][1]);
    TH1D *h_sys, *h_ratio_from_var;
    if(sys) {
      h_ratio_from_var= (TH1D *)inFileSys->Get(Form("fRatioFromVariationsTot_%.0f_%.0f", kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      h_sys= (TH1D *)inFileSys->Get(Form("fSystematicUncertaintyTot_%.0f_%.0f", kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
    }

    // compute corrected spectra
    for (int iMatt = 0; iMatt < 2; ++iMatt)
    {
      outFile.cd(histoNameDir);
      double cent_bin_lim_min = kCentBinsLimitsProton[iCent][0], cent_bin_lim_max = kCentBinsLimitsProton[iCent][1];
      if (useEfficiencyMB){
        cent_bin_lim_min = 0.;
        cent_bin_lim_max = 90.;
      }
      TH1D *eff = (TH1D *)inFileEff->Get(Form("%s_/f%sEff_TPC_%.0f_%.0f", cutSettings, kAntimatterMatter[iMatt], cent_bin_lim_min, cent_bin_lim_max));
      //TF1 *sec_f = (TF1 *)inFileSec->Get(Form("f%sFunctionFit_%.0f_%.0f", kAntimatterMatter[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      //TH2D *sec_f_cov = (TH2D *)inFileSec->Get(Form("f%sCovMat_%.0f_%.0f", kAntimatterMatter[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      TH1D *sec = (TH1D *)inFileSec->Get(Form("f%sPrimFrac_%.0f_%.0f", kAntimatterMatter[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      TH1D *raw = (TH1D *)inFileRaw->Get(Form("%s_%d_%d_%d/f%sTPCrawYield_%.0f_%.0f", cutSettings, binCounting, bkg_shape, iNsigma, kAntimatterMatter[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));

      //sec->Fit(&fitFuncSec,"R");
      fSpectra[iMatt] = new TH1D(*raw);
      int pTbinMax = 12;
      if (kVerbose) std::cout<<"entering pt loop..."<<std::endl;
      for (int iPtBin = 3; iPtBin < pTbinMax + 1; ++iPtBin)
      {
        double protonCorrection = protonCorrectionPt(iMatt,fSpectra[0]->GetBinCenter(iPtBin));
        double rawYield = raw->GetBinContent(iPtBin);
        double rawYieldError = raw->GetBinError(iPtBin);
        double efficiency = eff->GetBinContent(eff->FindBin(raw->GetBinCenter(iPtBin)));
        double effError = eff->GetBinError(eff->FindBin(raw->GetBinCenter(iPtBin)));
        /* double efficiency = eff->GetFunction("fitEff")->Eval(raw->GetBinCenter(iPtBin));
        double effError = 0.; */

        double primary = 0.;
        double primaryError = 0.;
        if (!sigmoidCorrection) {
          primary = sec->GetBinContent(iPtBin);
          primaryError = sec->GetBinError(iPtBin);
          //if (primary < 0.5) continue;
        }
        else {
          /* primary = sec_f->Eval(raw->GetXaxis()->GetBinCenter(iPtBin));
          double pt_center = raw->GetXaxis()->GetBinCenter(iPtBin);
          double par_0 = sec_f->GetParameter(0);
          double par_1 = sec_f->GetParameter(1);
          double var_par_0 = sec_f_cov->GetBinContent(1,1);
          double var_par_1 = sec_f_cov->GetBinContent(2,2);
          double cov = sec_f_cov->GetBinContent(1,2);
          double exponential = TMath::Exp(par_1*pt_center);
          double denominator = 1+par_0*exponential;
          double first_derivative_par_0 = -exponential/denominator/denominator;
          double first_derivative_par_1 = -par_0*exponential*pt_center/denominator/denominator;

          primaryError = TMath::Sqrt(first_derivative_par_0*first_derivative_par_0*var_par_0+first_derivative_par_1*first_derivative_par_1*var_par_1+2*first_derivative_par_0*first_derivative_par_1*cov);
          if (kVerbose) std::cout<<"error (fit) = "<<primaryError<<"; error (hist) = "<<sec->GetBinError(iPtBin)<<std::endl; */
        }
        //primaryError = sec->GetBinError(iPtBin);
        if (rawYield<1.e-7 || efficiency<1.e-2 || efficiency >1. || primary>0.999 || primary<0.5) {
          fSpectra[iMatt]->SetBinContent(iPtBin, 0.);
          fSpectra[iMatt]->SetBinError(iPtBin, 0);
        }
        else {
          fSpectra[iMatt]->SetBinContent(iPtBin, /* rawYield * primary / */ primary*rawYield/efficiency );
          fSpectra[iMatt]->SetBinError(iPtBin, primary*rawYieldError/efficiency);//rawYield * primary / efficiency * TMath::Sqrt(rawYieldError*rawYieldError/rawYield/rawYield + primaryError*primaryError/primary/primary));//(rawYield * primary / efficiency / protonCorrection) * TMath::Sqrt(primaryError * primaryError / primary / primary + effError * effError / efficiency / efficiency + rawYieldError * rawYieldError / rawYield / rawYield));
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
    TH1D SysError(Form("fSysError_%.0f_%.0f",kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]),Form("%.0f-%.0f%%",kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]),kNPtBins,kPtBins);
    int pTbinMax = 12;
    for (int iPtBin = 3; iPtBin < pTbinMax + 1; ++iPtBin)
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
          fRatio[iCent]->SetBinError(iPtBin, TMath::Sqrt(antiSpecErr * antiSpecErr / antiSpec / antiSpec + specErr * specErr / spec / spec));
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
    fRatio[iCent]->SetTitle(Form("%.0f-%.0f%%", kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
    //fRatio[iCent]->Fit("pol0");
    fRatio[iCent]->GetXaxis()->SetRangeUser(1.0,2.0);
    fRatio[iCent]->Write();

    SysError.GetXaxis()->SetRangeUser(1.,2.);
    SysError.GetXaxis()->SetTitle("#it{p}_{T} (GeV/#it{c})");
    SysError.GetYaxis()->SetTitle("Systematic Uncertainty");
    SysError.SetMinimum(0.);
    TCanvas cSysError(SysError.GetName(),SysError.GetTitle());
    SysError.Draw("histo");
    cSysError.Print(Form("%s.pdf",SysError.GetName()));
    SysError.Write();
    
    /* TCanvas cRatio(Form("cRatio_%.0f_%.0f", kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]), "cRatio");
    cRatio.SetTicks(1, 1);
    cRatio.cd();
    fRatio[iCent]->GetXaxis()->SetRangeUser(1.0,2.0);
    fRatio[iCent]->GetYaxis()->SetRangeUser(0.88, 1.12);
    fRatio[iCent]->Draw("");
    TLatex chi2(1.5, 1.08, Form("#chi^{2}/NDF = %.2f/%d", fRatio[iCent]->GetFunction("pol0")->GetChisquare(), fRatio[iCent]->GetFunction("pol0")->GetNDF()));
    chi2.SetTextSize(28);
    TLatex p0(1.5, 1.10, Form("R = %.4f #pm %.4f", fRatio[iCent]->GetFunction("pol0")->GetParameter(0), fRatio[iCent]->GetFunction("pol0")->GetParError(0)));
    p0.SetTextSize(28);
    chi2.Draw("same");
    p0.Draw("same");
    cRatio.Print(Form("%s/%s.pdf", kPlotDir, fRatio[iCent]->GetName())); */
  }
  outFile.Close();
}