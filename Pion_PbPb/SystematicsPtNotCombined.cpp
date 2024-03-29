// SystematicsPtNotCombined.cpp

#include <stdlib.h>
#include <string.h>

#include <TFile.h>
#include <TStyle.h>
#include <TDirectory.h>
#include <TFitResultPtr.h>
#include <TFitResult.h>
#include <Riostream.h>
#include <TString.h>
#include <TStopwatch.h>
#include <TRandom.h>
#include <TH1D.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TPaveStats.h>
#include <TGraphErrors.h>

#include "../utils/Config.h"

using namespace pion;

// #define USE_COUNTER
const bool sys_eff_error = true;
const bool kSmooth = true;
const int used_pt_bins = 27;
const int nTrials=10000;

bool barlow_criterion = true;

void reject_entry(TH1D* proj, double up_threshold, double low_threshold=-999.){
  for(int iB=1;iB<(proj->GetNbinsX()+1);++iB){
    if (proj->GetBinContent(iB)>0 && (proj->GetBinCenter(iB)>up_threshold || proj->GetBinCenter(iB)<low_threshold)){
      proj->SetBinContent(iB,0);
      proj->SetBinError(iB,0);
      std::cout<<"reject point!"<<std::endl;
    }
  }
}

void SystematicsPtNotCombined(const int points = kNPoints, const bool cutVar = true, const bool binCountingVar = true, const bool expVar = true, const bool sigmoidVar = true, const char *outFileName = "SystematicsAllEPtNotCombined_extend2")
{
  gStyle->SetTextFont(44);
  gStyle->SetOptStat(110001110);
  gStyle->SetStatX(0.87);
  gStyle->SetStatY(0.85);
  TStopwatch swatch;
  swatch.Start(true);

  TFile *specFile = TFile::Open(Form("%s/SpectraPionSys_extend2.root", kOutDir));
  TFile *specFile_def = TFile::Open(Form("%s/SpectraPion.root", kOutDir));
  TFile *hijingFile = TFile::Open("../HIJINGRatios.root");
  TFile *inFileSec = TFile::Open(Form("%s/PrimaryPion.root", kOutDir));
  TFile *effFile = TFile::Open(Form("%s/EfficiencyPionprova.root", kOutDir));
  TFile *outFile = TFile::Open(Form("%s/%s.root", kOutDir, outFileName), "recreate");

  for (int iC = 0; iC < kNCentClasses; ++iC) // TODO: extend the analysis to the third centrality class as well
  {
    TH1D fSystematicUncertaintyDCAxy(Form("fSystematicUncertaintyDCAxy_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), kNPtBins, kPtBins);
    TH1D fSystematicUncertaintyDCAz(Form("fSystematicUncertaintyDCAz_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), kNPtBins, kPtBins);
    TH1D fSystematicUncertaintyTPCCls(Form("fSystematicUncertaintyTPCCls_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), kNPtBins, kPtBins);
    TH1D fSystematicUncertaintyPID(Form("fSystematicUncertaintyPID_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), kNPtBins, kPtBins);
    TH1D fSystematicUncertaintyChi2TPC(Form("fSystematicUncertaintyChi2TPC_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), kNPtBins, kPtBins);
    TH1D fSystematicUncertaintyROIDown(Form("fSystematicUncertaintyROIDown_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), kNPtBins, kPtBins);
    TH1D fSystematicUncertaintyROIUp(Form("fSystematicUncertaintyROIUp_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), kNPtBins, kPtBins);
    TH1D fSystematicUncertaintyMismatchDown(Form("fSystematicUncertaintyMismatchDown_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), kNPtBins, kPtBins);
    TH1D fSystematicUncertaintyMismatchUp(Form("fSystematicUncertaintyMismatchUp_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), kNPtBins, kPtBins);
    TH1D fSystematicUncertaintyEff(Form("fSystematicUncertaintyEff_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), kNPtBins, kPtBins);
    TH1D fSystematicUncertaintyTFF(Form("fSystematicUncertaintyTFF_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), kNPtBins, kPtBins);
    TH1D fSystematicUncertaintyPrim(Form("fSystematicUncertaintyPrim_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), kNPtBins, kPtBins);
    TH1D fSystematicUncertaintyTotal(Form("fSystematicUncertaintyTotal_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), kNPtBins, kPtBins);
    TH1D fSystematicUncertaintyTotalPtCorrelated(Form("fSystematicUncertaintyTotalPtCorrelated_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), kNPtBins, kPtBins);
    TH1D fRatioFromVariationsTot(Form("fRatioFromVariationsTot_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), kNPtBins, kPtBins);
    double ratioBins[3000];
    for (int iRatioBins = 0; iRatioBins < 3000; iRatioBins++){
      ratioBins[iRatioBins]=0.85+iRatioBins*0.0001;
    }
    TH2D fRatiosVsPtDCAxy(Form("fRatiosVsPtDCAxy_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),kNPtBins,kPtBins,2999,ratioBins);
    TH2D fRatiosVsPtDCAz(Form("fRatiosVsPtDCAz_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),kNPtBins,kPtBins,2999,ratioBins);
    TH2D fRatiosVsPtTPCCls(Form("fRatiosVsPtTPCCls_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),kNPtBins,kPtBins,2999,ratioBins);
    TH2D fRatiosVsPtPID(Form("fRatiosVsPtPID_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),kNPtBins,kPtBins,2999,ratioBins);
    TH2D fRatiosVsPtChi2TPC(Form("fRatiosVsPtChi2TPC_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),kNPtBins,kPtBins,2999,ratioBins);
    TH2D fRatiosVsPtROIDown(Form("fRatiosVsPtROIDown_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),kNPtBins,kPtBins,2999,ratioBins);
    TH2D fRatiosVsPtROIUp(Form("fRatiosVsPtROIUp_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),kNPtBins,kPtBins,2999,ratioBins);
    TH2D fRatiosVsPtMismatchDown(Form("fRatiosVsPtMismatchDown_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),kNPtBins,kPtBins,2999,ratioBins);
    TH2D fRatiosVsPtMismatchUp(Form("fRatiosVsPtMismatchUp_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),kNPtBins,kPtBins,2999,ratioBins);
    TH2D fRatiosVsPtPrim(Form("fRatiosVsPtPrim_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),kNPtBins,kPtBins,2999,ratioBins);
    TH2D fRatiosVsPtTot(Form("fRatiosVsPt_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),kNPtBins,kPtBins,2999,ratioBins);
    
    // total cuts
    int cutVariable=0;
    int bkgFlag = 1;
    int sigmoidFlag = 1;
    int iNsigmaDown = 1;
    int iNsigmaUp = 1;
    int iNsigmaMismatchDown = 1;
    int iNsigmaMismatchUp = 1;
    for (int iTrackCuts=0; iTrackCuts<7/* kNTrackCuts */; ++iTrackCuts){
      for (int iROIDown=0; iROIDown<3; ++iROIDown){
        for (int iROIUp=0; iROIUp<3; ++iROIUp){
          for (int iSigmoid=0; iSigmoid<1; ++iSigmoid){
            auto tmpCutSettings = trackCutSettings[iTrackCuts];
            auto cutIndex = trackCutIndexes[iTrackCuts];
            auto tmpCutIndex = Form("%d",cutIndex);
            if ( iTrackCuts == 0 )
            {
              tmpCutIndex = Form("");
              tmpCutSettings = Form("");
            }
            int iMismatchDown=iNsigmaMismatchDown;
            int iMismatchUp=iNsigmaMismatchUp;
            auto fullCutSettingsSigm = Form("%s%s_%d_%d_%d_%d_%d_%d", tmpCutSettings, tmpCutIndex, bkgFlag, iSigmoid, iROIDown, iROIUp, iMismatchDown, iMismatchUp);
            std::cout << fullCutSettingsSigm <<std::endl;
            TH1D *h = (TH1D *)specFile->Get(Form("%s/fRatio_%.0f_%.0f", fullCutSettingsSigm, kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
            for(int iPtBins=6;iPtBins<used_pt_bins-1;++iPtBins){
              fRatiosVsPtTot.Fill(kPtBins[iPtBins],h->GetBinContent(iPtBins+1));
            }
          }
        }
      }
    }

    TH1D *hDefault = (TH1D *)specFile->Get(Form("_1_0_1_1_1_1/fRatio_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
    // DCAz cuts
    cutVariable=0;
    bkgFlag = 1;
    sigmoidFlag = 0;
    iNsigmaUp = 1;
    iNsigmaDown = 1;
    for (int iTrackCuts=1; iTrackCuts<5; ++iTrackCuts){
      auto tmpCutSettings = trackCutSettings[iTrackCuts];
      auto cutIndex = trackCutIndexes[iTrackCuts];
      auto tmpCutIndex = Form("%d",cutIndex);
      if ( iTrackCuts == 0 )
      {
        tmpCutIndex = Form("");
        tmpCutSettings = Form("");
      }
      auto fullCutSettingsSigm = Form("%s%s_%d_%d_%d_%d_%d_%d", tmpCutSettings, tmpCutIndex, bkgFlag, sigmoidFlag, iNsigmaDown, iNsigmaUp, iNsigmaMismatchDown, iNsigmaMismatchUp);
      std::cout << fullCutSettingsSigm <<std::endl;
      TH1D *h = (TH1D *)specFile->Get(Form("%s/fRatio_%.0f_%.0f", fullCutSettingsSigm, kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
      for(int iPtBins=6;iPtBins<used_pt_bins-1;++iPtBins){
        double h_content = h->GetBinContent(iPtBins+1);
        double h_error = h->GetBinError(iPtBins+1);
        double h_0_content = hDefault->GetBinContent(iPtBins+1);
        double h_0_error = hDefault->GetBinError(iPtBins+1);
        double z = (h_content-h_error)/TMath::Sqrt(h_0_error*h_0_error+h_error*h_error);
        if ((std::abs(z)>2. && barlow_criterion) || !barlow_criterion) fRatiosVsPtDCAz.Fill(kPtBins[iPtBins],h->GetBinContent(iPtBins+1));
      }
    }

    // PID cuts
    cutVariable=0;
    bkgFlag = 1;
    sigmoidFlag = 0;
    iNsigmaUp = 1;
    iNsigmaDown = 1;
    for (int iTrackCuts=5; iTrackCuts<7; ++iTrackCuts){
      auto tmpCutSettings = trackCutSettings[iTrackCuts];
      auto cutIndex = trackCutIndexes[iTrackCuts];
      auto tmpCutIndex = Form("%d",cutIndex);
      if ( iTrackCuts == 0 )
      {
        tmpCutIndex = Form("");
        tmpCutSettings = Form("");
      }
      auto fullCutSettingsSigm = Form("%s%s_%d_%d_%d_%d_%d_%d", tmpCutSettings, tmpCutIndex, bkgFlag, sigmoidFlag, iNsigmaDown, iNsigmaUp, iNsigmaMismatchDown, iNsigmaMismatchUp);
      std::cout << fullCutSettingsSigm <<std::endl;
      TH1D *h = (TH1D *)specFile->Get(Form("%s/fRatio_%.0f_%.0f", fullCutSettingsSigm, kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
      for(int iPtBins=6;iPtBins<used_pt_bins-1;++iPtBins){
        double h_content = h->GetBinContent(iPtBins+1);
        double h_error = h->GetBinError(iPtBins+1);
        double h_0_content = hDefault->GetBinContent(iPtBins+1);
        double h_0_error = hDefault->GetBinError(iPtBins+1);
        double z = (h_content-h_error)/TMath::Sqrt(h_0_error*h_0_error+h_error*h_error);
        if ((std::abs(z)>2. && barlow_criterion) || !barlow_criterion) fRatiosVsPtPID.Fill(kPtBins[iPtBins],h->GetBinContent(iPtBins+1));
      }
    }

    // TPCCls cuts
    cutVariable=0;
    bkgFlag = 1;
    sigmoidFlag = 0;
    iNsigmaDown = 1;
    iNsigmaUp = 1;
    for (int iTrackCuts=7; iTrackCuts<11; ++iTrackCuts){
      auto tmpCutSettings = trackCutSettings[iTrackCuts];
      auto cutIndex = trackCutIndexes[iTrackCuts];
      auto tmpCutIndex = Form("%d",cutIndex);
      if ( iTrackCuts == 0 )
      {
        tmpCutIndex = Form("");
        tmpCutSettings = Form("");
      }
      auto fullCutSettingsSigm = Form("%s%s_%d_%d_%d_%d_%d_%d", tmpCutSettings, tmpCutIndex, bkgFlag, sigmoidFlag, iNsigmaDown, iNsigmaUp, iNsigmaMismatchDown, iNsigmaMismatchUp);
      std::cout << fullCutSettingsSigm <<std::endl;
      TH1D *h = (TH1D *)specFile->Get(Form("%s/fRatio_%.0f_%.0f", fullCutSettingsSigm, kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
      for(int iPtBins=6;iPtBins<used_pt_bins-1;++iPtBins){
        double h_content = h->GetBinContent(iPtBins+1);
        double h_error = h->GetBinError(iPtBins+1);
        double h_0_content = hDefault->GetBinContent(iPtBins+1);
        double h_0_error = hDefault->GetBinError(iPtBins+1);
        double z = (h_content-h_error)/TMath::Sqrt(h_0_error*h_0_error+h_error*h_error);
        if ((std::abs(z)>2. && barlow_criterion) || !barlow_criterion) fRatiosVsPtTPCCls.Fill(kPtBins[iPtBins],h->GetBinContent(iPtBins+1));
      }
    }

    // DCAxy cuts
    cutVariable=0;
    bkgFlag = 1;
    sigmoidFlag = 0;
    iNsigmaUp = 1;
    for (int iTrackCuts=11; iTrackCuts<kNTrackCuts-2; ++iTrackCuts){
      auto tmpCutSettings = trackCutSettings[iTrackCuts];
      auto cutIndex = trackCutIndexes[iTrackCuts];
      auto tmpCutIndex = Form("%d",cutIndex);
      if ( iTrackCuts == 0 )
      {
        tmpCutIndex = Form("");
        tmpCutSettings = Form("");
      }
      auto fullCutSettingsSigm = Form("%s%s_%d_%d_%d_%d_%d_%d", tmpCutSettings, tmpCutIndex, bkgFlag, sigmoidFlag, iNsigmaDown, iNsigmaUp, iNsigmaMismatchDown, iNsigmaMismatchUp);
      std::cout << fullCutSettingsSigm <<std::endl;
      TH1D *h = (TH1D *)specFile->Get(Form("%s/fRatio_%.0f_%.0f", fullCutSettingsSigm, kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
      for(int iPtBins=6;iPtBins<used_pt_bins-1;++iPtBins){
        double h_content = h->GetBinContent(iPtBins+1);
        double h_error = h->GetBinError(iPtBins+1);
        double h_0_content = hDefault->GetBinContent(iPtBins+1);
        double h_0_error = hDefault->GetBinError(iPtBins+1);
        double z = (h_content-h_error)/TMath::Sqrt(h_0_error*h_0_error+h_error*h_error);
        if ((std::abs(z)>2. && barlow_criterion) || !barlow_criterion) fRatiosVsPtDCAxy.Fill(kPtBins[iPtBins],h->GetBinContent(iPtBins+1));
      }
    }

    // chi2tpc cuts
    cutVariable=0;
    bkgFlag = 1;
    sigmoidFlag = 0;
    iNsigmaUp = 1;
    for (int iTrackCuts=15; iTrackCuts<kNTrackCuts; ++iTrackCuts){
      auto tmpCutSettings = trackCutSettings[iTrackCuts];
      auto cutIndex = trackCutIndexes[iTrackCuts];
      auto tmpCutIndex = Form("%d",cutIndex);
      if ( iTrackCuts == 0 )
      {
        tmpCutIndex = Form("");
        tmpCutSettings = Form("");
      }
      auto fullCutSettingsSigm = Form("%s%s_%d_%d_%d_%d_%d_%d", tmpCutSettings, tmpCutIndex, bkgFlag, sigmoidFlag, iNsigmaDown, iNsigmaUp, iNsigmaMismatchDown, iNsigmaMismatchUp);
      std::cout << fullCutSettingsSigm <<std::endl;
      TH1D *h = (TH1D *)specFile->Get(Form("%s/fRatio_%.0f_%.0f", fullCutSettingsSigm, kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
      for(int iPtBins=6;iPtBins<used_pt_bins-1;++iPtBins){
        double h_content = h->GetBinContent(iPtBins+1);
        double h_error = h->GetBinError(iPtBins+1);
        double h_0_content = hDefault->GetBinContent(iPtBins+1);
        double h_0_error = hDefault->GetBinError(iPtBins+1);
        double z = (h_content-h_error)/TMath::Sqrt(h_0_error*h_0_error+h_error*h_error);
        if ((std::abs(z)>2. && barlow_criterion) || !barlow_criterion) fRatiosVsPtChi2TPC.Fill(kPtBins[iPtBins],h->GetBinContent(iPtBins+1));
      }
    }

    // efficiency error
    TH1D *h_a_eff = (TH1D *)effFile->Get(Form("_/fAEff_TOF_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
    TH1D *h_m_eff = (TH1D *)effFile->Get(Form("_/fMEff_TOF_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
    for(int iPtBin=7;iPtBin<used_pt_bins;++iPtBin){
      double err_eff_a=h_a_eff->GetBinError(iPtBin);
      double eff_a=h_a_eff->GetBinContent(iPtBin);
      double err_eff_m=h_m_eff->GetBinError(iPtBin);
      double eff_m=h_m_eff->GetBinContent(iPtBin);
      fSystematicUncertaintyEff.SetBinContent(iPtBin,TMath::Sqrt(err_eff_a*err_eff_a/eff_a/eff_a+err_eff_m*err_eff_m/eff_m/eff_m));
      fSystematicUncertaintyEff.SetBinError(iPtBin,0);
    }

    // primary fraction (TFF) error
    for(int iPtBin=7;iPtBin<used_pt_bins;++iPtBin){
      double primaryRelativeError[2];
      for (int iMatt = 0; iMatt < 2; ++iMatt){
        TH1D *h_sec = (TH1D*)inFileSec->Get(Form("f%sPrimFrac_%.0f_%.0f", kAntimatterMatter[iMatt], kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
        double primaryError=h_sec->GetBinError(iPtBin);
        double primary=h_sec->GetBinContent(iPtBin);
        primaryRelativeError[iMatt]=primaryError/primary;
      }
      fSystematicUncertaintyTFF.SetBinContent(iPtBin,TMath::Sqrt(primaryRelativeError[0]*primaryRelativeError[0]+primaryRelativeError[1]*primaryRelativeError[1]));
      fSystematicUncertaintyTFF.SetBinError(iPtBin,0);
    }

    // roi error down
    cutVariable=0;
    iNsigmaUp = 1;
    iNsigmaDown = 0;
    for (int iRoiDown=0; iRoiDown<3; ++iRoiDown){
      auto tmpCutSettings = Form("");
      auto tmpCutIndex = Form("");
      iNsigmaDown=iRoiDown;
      auto fullCutSettingsSigm = Form("%s%s_%d_%d_%d_%d_%d_%d", tmpCutSettings, tmpCutIndex, bkgFlag, sigmoidFlag, iNsigmaDown, iNsigmaUp, iNsigmaMismatchDown, iNsigmaMismatchUp);
      std::cout << fullCutSettingsSigm <<std::endl;
      TH1D *h = (TH1D *)specFile->Get(Form("%s/fRatio_%.0f_%.0f", fullCutSettingsSigm, kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
      for(int iPtBins=6;iPtBins<used_pt_bins-1;++iPtBins){
        double h_content = h->GetBinContent(iPtBins+1);
        double h_error = h->GetBinError(iPtBins+1);
        double h_0_content = hDefault->GetBinContent(iPtBins+1);
        double h_0_error = hDefault->GetBinError(iPtBins+1);
        double z = (h_content-h_error)/TMath::Sqrt(h_0_error*h_0_error+h_error*h_error);
        if ((std::abs(z)>2. && barlow_criterion) || !barlow_criterion) fRatiosVsPtROIDown.Fill(kPtBins[iPtBins],h->GetBinContent(iPtBins+1));
      }
    }

    // roi error up
    cutVariable=0;
    iNsigmaUp = 0;
    iNsigmaDown = 1;
    for (int iRoiUp=0; iRoiUp<3; ++iRoiUp){
      auto tmpCutSettings = Form("");
      auto tmpCutIndex = Form("");
      iNsigmaUp=iRoiUp;
      auto fullCutSettingsSigm = Form("%s%s_%d_%d_%d_%d_%d_%d", tmpCutSettings, tmpCutIndex, bkgFlag, sigmoidFlag, iNsigmaDown, iNsigmaUp, iNsigmaMismatchDown, iNsigmaMismatchUp);
      std::cout << fullCutSettingsSigm <<std::endl;
      TH1D *h = (TH1D *)specFile->Get(Form("%s/fRatio_%.0f_%.0f", fullCutSettingsSigm, kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
      for(int iPtBins=6;iPtBins<used_pt_bins-1;++iPtBins){
        double h_content = h->GetBinContent(iPtBins+1);
        double h_error = h->GetBinError(iPtBins+1);
        double h_0_content = hDefault->GetBinContent(iPtBins+1);
        double h_0_error = hDefault->GetBinError(iPtBins+1);
        double z = (h_content-h_error)/TMath::Sqrt(h_0_error*h_0_error+h_error*h_error);
        if ((std::abs(z)>2. && barlow_criterion) || !barlow_criterion) fRatiosVsPtROIUp.Fill(kPtBins[iPtBins],h->GetBinContent(iPtBins+1));
      }
    }

    // mismatch error down
    cutVariable=0;
    iNsigmaUp = 1;
    iNsigmaDown = 1;
    for (int iMismatchDown=0; iMismatchDown<3; ++iMismatchDown){
      auto tmpCutSettings = Form("");
      auto tmpCutIndex = Form("");
      iNsigmaMismatchDown=iMismatchDown;
      auto fullCutSettingsSigm = Form("%s%s_%d_%d_%d_%d_%d_%d", tmpCutSettings, tmpCutIndex, bkgFlag, sigmoidFlag, iNsigmaDown, iNsigmaUp, iNsigmaMismatchDown, iNsigmaMismatchUp);
      std::cout << fullCutSettingsSigm <<std::endl;
      TH1D *h = (TH1D *)specFile->Get(Form("%s/fRatio_%.0f_%.0f", fullCutSettingsSigm, kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
      for(int iPtBins=6;iPtBins<used_pt_bins-1;++iPtBins){
        double h_content = h->GetBinContent(iPtBins+1);
        double h_error = h->GetBinError(iPtBins+1);
        double h_0_content = hDefault->GetBinContent(iPtBins+1);
        double h_0_error = hDefault->GetBinError(iPtBins+1);
        double z = (h_content-h_error)/TMath::Sqrt(h_0_error*h_0_error+h_error*h_error);
        if ((std::abs(z)>2. && barlow_criterion) || !barlow_criterion) fRatiosVsPtMismatchDown.Fill(kPtBins[iPtBins],h->GetBinContent(iPtBins+1));
      }
    }

    // mismatch error up
    cutVariable=0;
    iNsigmaUp = 1;
    iNsigmaDown = 1;
    iNsigmaMismatchDown=1;
    for (int iMismatchUp=0; iMismatchUp<3; ++iMismatchUp){
      auto tmpCutSettings = Form("");
      auto tmpCutIndex = Form("");
      iNsigmaMismatchUp=iMismatchUp;
      auto fullCutSettingsSigm = Form("%s%s_%d_%d_%d_%d_%d_%d", tmpCutSettings, tmpCutIndex, bkgFlag, sigmoidFlag, iNsigmaDown, iNsigmaUp, iNsigmaMismatchDown, iNsigmaMismatchUp);
      std::cout << fullCutSettingsSigm <<std::endl;
      TH1D *h = (TH1D *)specFile->Get(Form("%s/fRatio_%.0f_%.0f", fullCutSettingsSigm, kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
      for(int iPtBins=6;iPtBins<used_pt_bins-1;++iPtBins){
        double h_content = h->GetBinContent(iPtBins+1);
        double h_error = h->GetBinError(iPtBins+1);
        double h_0_content = hDefault->GetBinContent(iPtBins+1);
        double h_0_error = hDefault->GetBinError(iPtBins+1);
        double z = (h_content-h_error)/TMath::Sqrt(h_0_error*h_0_error+h_error*h_error);
        if ((std::abs(z)>2. && barlow_criterion) || !barlow_criterion) fRatiosVsPtMismatchUp.Fill(kPtBins[iPtBins],h->GetBinContent(iPtBins+1));
      }
    }

    // prim error
    cutVariable=0;
    iNsigmaUp = 1;
    iNsigmaDown = 1;
    iNsigmaMismatchUp=1;
    for (int iPrim=0; iPrim<1; ++iPrim){
      auto tmpCutSettings = Form("dcaz");
      auto tmpCutIndex = Form("2");
      sigmoidFlag=iPrim;
      auto fullCutSettingsSigm = Form("%s%s_%d_%d_%d_%d_%d_%d", tmpCutSettings, tmpCutIndex, bkgFlag, sigmoidFlag, iNsigmaDown, iNsigmaUp, iNsigmaMismatchDown, iNsigmaMismatchUp);
      std::cout << fullCutSettingsSigm <<std::endl;
      TH1D *h = (TH1D *)specFile->Get(Form("%s/fRatio_%.0f_%.0f", fullCutSettingsSigm, kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
      for(int iPtBins=6;iPtBins<used_pt_bins-1;++iPtBins){
        fRatiosVsPtPrim.Fill(kPtBins[iPtBins],h->GetBinContent(iPtBins+1));
      }
    }

    for(int iPtBins=7;iPtBins<used_pt_bins;++iPtBins){
      // track cuts
     TH1D *proj=fRatiosVsPtDCAz.ProjectionY("py",iPtBins,iPtBins);
      if (proj->GetEntries()==0 || proj->GetMean()==0){
        fSystematicUncertaintyDCAz.SetBinContent(iPtBins,0);
      }
      else
        fSystematicUncertaintyDCAz.SetBinContent(iPtBins,proj->GetRMS()/proj->GetMean());
      fSystematicUncertaintyDCAz.SetBinError(iPtBins,0);

      proj=fRatiosVsPtPID.ProjectionY("py",iPtBins,iPtBins);
      if (proj->GetEntries()==0 || proj->GetMean()==0){
        fSystematicUncertaintyPID.SetBinContent(iPtBins,0);
      }
      else
        fSystematicUncertaintyPID.SetBinContent(iPtBins,proj->GetRMS()/proj->GetMean());
      fSystematicUncertaintyPID.SetBinError(iPtBins,0);
      if (iC==4 && iPtBins==26)
        fSystematicUncertaintyPID.SetBinContent(iPtBins,0);

      proj=fRatiosVsPtTPCCls.ProjectionY("py",iPtBins,iPtBins);
      //reject_entry(proj,1.01,0.97);
      if (proj->GetEntries()==0 || proj->GetMean()==0){
        fSystematicUncertaintyTPCCls.SetBinContent(iPtBins,0);
      }
      else
        fSystematicUncertaintyTPCCls.SetBinContent(iPtBins,proj->GetRMS()/proj->GetMean());
      fSystematicUncertaintyTPCCls.SetBinError(iPtBins,0);

      proj=fRatiosVsPtDCAxy.ProjectionY("py",iPtBins,iPtBins);
      if (proj->GetEntries()==0 || proj->GetMean()==0){
        fSystematicUncertaintyDCAxy.SetBinContent(iPtBins,0);
      }
      else
        fSystematicUncertaintyDCAxy.SetBinContent(iPtBins,proj->GetRMS()/proj->GetMean());
      fSystematicUncertaintyDCAxy.SetBinError(iPtBins,0);

      proj=fRatiosVsPtChi2TPC.ProjectionY("py",iPtBins,iPtBins);
      if (proj->GetEntries()==0 || proj->GetMean()==0){
        fSystematicUncertaintyChi2TPC.SetBinContent(iPtBins,0);
      }
      else
        fSystematicUncertaintyChi2TPC.SetBinContent(iPtBins,proj->GetRMS()/proj->GetMean());
      fSystematicUncertaintyChi2TPC.SetBinError(iPtBins,0);

      // roi up
      proj=fRatiosVsPtROIUp.ProjectionY("py",iPtBins,iPtBins);
      if (proj->GetEntries()==0 || proj->GetMean()==0){
        fSystematicUncertaintyROIUp.SetBinContent(iPtBins,0);
      }
      else
        fSystematicUncertaintyROIUp.SetBinContent(iPtBins,proj->GetRMS()/proj->GetMean());
      fSystematicUncertaintyROIUp.SetBinError(iPtBins,0);

      // roi down
      proj=fRatiosVsPtROIDown.ProjectionY("py",iPtBins,iPtBins);
      if (proj->GetEntries()==0 || proj->GetMean()==0){
        fSystematicUncertaintyROIDown.SetBinContent(iPtBins,0);
      }
      else
        fSystematicUncertaintyROIDown.SetBinContent(iPtBins,proj->GetRMS()/proj->GetMean());
      fSystematicUncertaintyROIDown.SetBinError(iPtBins,0);

      // Mismatch up
      proj=fRatiosVsPtMismatchUp.ProjectionY("py",iPtBins,iPtBins);
      if (proj->GetEntries()==0 || proj->GetMean()==0){
        fSystematicUncertaintyMismatchUp.SetBinContent(iPtBins,0);
      }
      else
        fSystematicUncertaintyMismatchUp.SetBinContent(iPtBins,proj->GetRMS()/proj->GetMean());
      fSystematicUncertaintyMismatchUp.SetBinError(iPtBins,0);

      // Mismatch down
      proj=fRatiosVsPtMismatchDown.ProjectionY("py",iPtBins,iPtBins);
      if (proj->GetEntries()==0 || proj->GetMean()==0){
        fSystematicUncertaintyMismatchDown.SetBinContent(iPtBins,0);
      }
      else
        fSystematicUncertaintyMismatchDown.SetBinContent(iPtBins,proj->GetRMS()/proj->GetMean());
      fSystematicUncertaintyMismatchDown.SetBinError(iPtBins,0);
      if (iC==4 && iPtBins==15)
        fSystematicUncertaintyMismatchDown.SetBinContent(iPtBins,0);

      // prim
      proj=fRatiosVsPtPrim.ProjectionY("py",iPtBins,iPtBins);
      if (proj->GetEntries()==0 || proj->GetMean()==0){
        fSystematicUncertaintyPrim.SetBinContent(iPtBins,0);
      }
      else
        fSystematicUncertaintyPrim.SetBinContent(iPtBins,proj->GetRMS()/proj->GetMean());
      fSystematicUncertaintyPrim.SetBinError(iPtBins,0);

      // tot
      proj=fRatiosVsPtTot.ProjectionY("py",iPtBins,iPtBins);
      double mean = proj->GetMean();
      double rms = proj->GetRMS();
      // reject outliers
      double rejection_criterion=3.; // 3 sigma rejection
      int count_outliers = 999;
      while (count_outliers>0){
        count_outliers = 0;
        mean=proj->GetMean();
        rms=proj->GetRMS();
        for(int iB=1;iB<=proj->GetNbinsX();++iB){
          double tmp=proj->GetBinCenter(iB);
          bool isFilled=proj->GetBinContent(iB)>0;
          if (std::abs(tmp-mean)>rejection_criterion*rms && isFilled){
            proj->SetBinContent(iB,0.);
            count_outliers++;
          }
        }
        std::cout << "outliers found = " << count_outliers << std::endl;
      }
      
      fRatioFromVariationsTot.SetBinContent(iPtBins,proj->GetMean());
      fRatioFromVariationsTot.SetBinError(iPtBins,0);
    }
    TCanvas cRatioVsPtTot("c","c");
    fRatiosVsPtTot.GetXaxis()->SetRangeUser(0.7,1.6);
    fRatiosVsPtTot.GetYaxis()->SetRangeUser(0.92,1.08);
    fRatiosVsPtTot.GetXaxis()->SetTitle("#it{p}_{T} (GeV/#it{c})");
    fRatiosVsPtTot.GetYaxis()->SetTitle("Ratio #pi^{-}/#pi^{+}");
    fRatiosVsPtTot.Draw("colz");
    cRatioVsPtTot.Print(Form("%s.pdf",fRatiosVsPtTot.GetName()));
    fRatiosVsPtTot.Write();
    fRatioFromVariationsTot.Write();
    fRatiosVsPtDCAxy.Write();
    fRatiosVsPtDCAz.Write();
    fRatiosVsPtPID.Write();
    fRatiosVsPtChi2TPC.Write();
    fRatiosVsPtTPCCls.Write();
    fRatiosVsPtPrim.Write();
    if (kSmooth){
      fSystematicUncertaintyDCAz.GetXaxis()->SetRangeUser(0.7, 1.6);
      fSystematicUncertaintyDCAxy.GetXaxis()->SetRangeUser(0.7, 1.6);
      fSystematicUncertaintyTPCCls.GetXaxis()->SetRangeUser(0.7, 1.6);
      fSystematicUncertaintyPID.GetXaxis()->SetRangeUser(0.7, 1.6);
      fSystematicUncertaintyChi2TPC.GetXaxis()->SetRangeUser(0.7, 1.6);
      fSystematicUncertaintyROIDown.GetXaxis()->SetRangeUser(0.7, 1.6);
      fSystematicUncertaintyROIUp.GetXaxis()->SetRangeUser(0.7, 1.6);
      fSystematicUncertaintyMismatchDown.GetXaxis()->SetRangeUser(0.7, 1.6);
      fSystematicUncertaintyMismatchUp.GetXaxis()->SetRangeUser(0.7, 1.6);
      fSystematicUncertaintyEff.GetXaxis()->SetRangeUser(0.7, 1.4);
      fSystematicUncertaintyTFF.GetXaxis()->SetRangeUser(0.7, 1.4);
      fSystematicUncertaintyDCAz.Smooth(1,"R");
      fSystematicUncertaintyDCAxy.Smooth(1,"R");
      fSystematicUncertaintyTPCCls.Smooth(1,"R");
      fSystematicUncertaintyPID.Smooth(1,"R");
      fSystematicUncertaintyChi2TPC.Smooth(1,"R");
      fSystematicUncertaintyROIDown.Smooth(1,"R");
      fSystematicUncertaintyROIUp.Smooth(1,"R");
      fSystematicUncertaintyMismatchDown.Smooth(1,"R");
      fSystematicUncertaintyMismatchUp.Smooth(1,"R");
      fSystematicUncertaintyEff.Smooth(1,"R");
      fSystematicUncertaintyTFF.Smooth(1,"R");
      fSystematicUncertaintyEff.GetXaxis()->SetRangeUser(0.7, 1.6);
      fSystematicUncertaintyTFF.GetXaxis()->SetRangeUser(0.7, 1.6);
    }
    fSystematicUncertaintyDCAxy.Write();
    fSystematicUncertaintyDCAz.Write();
    fSystematicUncertaintyPID.Write();
    fSystematicUncertaintyChi2TPC.Write();
    fSystematicUncertaintyTPCCls.Write();
    fSystematicUncertaintyPrim.Write();
    fSystematicUncertaintyROIUp.Write();
    fSystematicUncertaintyROIDown.Write();
    fSystematicUncertaintyMismatchUp.Write();
    fSystematicUncertaintyMismatchDown.Write();
    fSystematicUncertaintyEff.Write();
    fSystematicUncertaintyTFF.Write();

    // * * * * * * * * * * * * * * * * * * * * * * * * * //
    //   C O M P U T E   P T   C O R R E L A T I O N S   //
    // * * * * * * * * * * * * * * * * * * * * * * * * * //
    TH2D fSystematicsCorrelationsDCAxy(Form("fSystematicsCorrelationsDCAxy_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),5,0.095,0.145,1000,-.01,.01);
    TH2D fSystematicsCorrelationsDCAz(Form("fSystematicsCorrelationsDCAz_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),31,0.475,2.025,1000,-.01,.01);
    TH2D fSystematicsCorrelationsChi2TPC(Form("fSystematicsCorrelationsChi2TPC_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),26,1.995,2.255,1000,-.01,.01);
    TH2D fSystematicsCorrelationsTPCCls(Form("fSystematicsCorrelationsTPCCls_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),24,58.5,82.5,1000,-.01,.01);
    TH2D fSystematicsCorrelationsPID(Form("fSystematicsCorrelationsPID_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),25,3.245,3.505,1000,-.01,.01);
    TH2D fSystematicsCorrelationsROIUp(Form("fSystematicsCorrelationsROIUp_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),22,9.9,12.1,1000,-.01,.01);
    TH2D fSystematicsCorrelationsROIDown(Form("fSystematicsCorrelationsROIDown_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),11,0.95,2.05,1000,-.01,.01);
    TH2D fSystematicsCorrelationsMismatchDown(Form("fSystematicsCorrelationsMismatchDown_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),51,7.99,9.01,1000,-.01,.01);
    TH2D fSystematicsCorrelationsMismatchUp(Form("fSystematicsCorrelationsMismatchUp_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),21,12.45,14.55,1000,-.01,.01);

    // track cuts
    cutVariable=0;
    bkgFlag = 1;
    sigmoidFlag = 0;
    iNsigmaUp = 1;
    hDefault = (TH1D *)specFile->Get(Form("_1_0_1_1_1_1/fRatio_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
    for (int iTrackCuts=1; iTrackCuts<kNTrackCuts; ++iTrackCuts){
      TString tmpCutSettings = trackCutSettings[iTrackCuts];
      auto cutIndex = trackCutIndexes[iTrackCuts];
      auto tmpCutIndex = Form("%d",cutIndex);
      auto fullCutSettingsSigm = Form("%s%s_%d_%d_%d_%d_%d_%d", tmpCutSettings.Data(), tmpCutIndex, bkgFlag, sigmoidFlag, iNsigmaDown, iNsigmaUp, iNsigmaMismatchDown, iNsigmaMismatchUp);
      //std::cout << fullCutSettingsSigm <<std::endl;
      TH1D *h = (TH1D *)specFile->Get(Form("%s/fRatio_%.0f_%.0f", fullCutSettingsSigm, kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
      for(int iPtBins=6;iPtBins<used_pt_bins-1;++iPtBins){
        double h_content = h->GetBinContent(iPtBins+1);
        double h_error = h->GetBinError(iPtBins+1);
        double h_0_content = hDefault->GetBinContent(iPtBins+1);
        double h_0_error = hDefault->GetBinError(iPtBins+1);
        double z = (h_content-h_error)/TMath::Sqrt(h_0_error*h_0_error+h_error*h_error);
        if ((std::abs(z)>2. && barlow_criterion) || !barlow_criterion) {
          if (tmpCutSettings.EqualTo("dcaz"))
          {
            int cutVal = cutIndex;
            if (cutIndex > 1) cutVal = cutIndex+1;
            fSystematicsCorrelationsDCAz.Fill(kCutDCAz[cutVal],h->GetBinContent(iPtBins+1)-hDefault->GetBinContent(iPtBins+1));
          }
          if (tmpCutSettings.EqualTo("dcaxy"))
          {
            int cutVal = cutIndex;
            if (cutIndex > 1) cutVal = cutIndex+1;
            fSystematicsCorrelationsDCAxy.Fill(kCutDCAxy[cutVal],h->GetBinContent(iPtBins+1)-hDefault->GetBinContent(iPtBins+1));
            std::cout << "cutDCAxy = " << kCutDCAxy[cutVal] << " = " << h->GetBinContent(iPtBins+1)-hDefault->GetBinContent(iPtBins+1) << std::endl;
          }
          if (tmpCutSettings.EqualTo("tpc"))
          {
            int cutVal = cutIndex;
            if (cutIndex > 1) cutVal = cutIndex+1;
            fSystematicsCorrelationsTPCCls.Fill(kCutTPCClusters[cutVal],h->GetBinContent(iPtBins+1)-hDefault->GetBinContent(iPtBins+1));
          }
          if (tmpCutSettings.EqualTo("pid"))
          {
            int cutVal = cutIndex+1;
            fSystematicsCorrelationsPID.Fill(kTPCPidSigmas[cutVal],h->GetBinContent(iPtBins+1)-hDefault->GetBinContent(iPtBins+1));
          }
          if (tmpCutSettings.EqualTo("chisquare"))
          {
            int cutVal = cutIndex;
            fSystematicsCorrelationsChi2TPC.Fill(kCutChi2TPCVariations[cutVal],h->GetBinContent(iPtBins+1)-hDefault->GetBinContent(iPtBins+1));
          }
        }
      }
    }

    // left limit
    cutVariable=0;
    bkgFlag = 1;
    sigmoidFlag = 0;
    iNsigmaUp = 1;
    iNsigmaDown = 1;
    iNsigmaMismatchUp = 1;
    iNsigmaMismatchDown = 1;
    hDefault = (TH1D *)specFile->Get(Form("_1_0_1_1_1_1/fRatio_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
    for (int iROI=0; iROI<2; ++iROI){
      TString tmpCutSettings = Form("");
      auto cutIndex = 0;
      auto tmpCutIndex = Form("");
      iROI == 0 ? iNsigmaDown=iROI : iNsigmaDown=iROI+1;
      auto fullCutSettingsSigm = Form("%s%s_%d_%d_%d_%d_%d_%d", tmpCutSettings.Data(), tmpCutIndex, bkgFlag, sigmoidFlag, iNsigmaDown, iNsigmaUp, iNsigmaMismatchDown, iNsigmaMismatchUp);
      //std::cout << fullCutSettingsSigm <<std::endl;
      TH1D *h = (TH1D *)specFile->Get(Form("%s/fRatio_%.0f_%.0f", fullCutSettingsSigm, kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
      for(int iPtBins=6;iPtBins<used_pt_bins-1;++iPtBins){
        double cutVal = 1.;
        if (iROI==1) cutVal=2.;
        fSystematicsCorrelationsROIDown.Fill(cutVal,h->GetBinContent(iPtBins+1)-hDefault->GetBinContent(iPtBins+1));
      }
    }

    // right limit
    cutVariable=0;
    bkgFlag = 1;
    sigmoidFlag = 0;
    iNsigmaUp = 1;
    iNsigmaDown = 1;
    iNsigmaMismatchUp = 1;
    iNsigmaMismatchDown = 1;
    hDefault = (TH1D *)specFile->Get(Form("_1_0_1_1_1_1/fRatio_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
    for (int iROI=0; iROI<2; ++iROI){
      TString tmpCutSettings = Form("");
      auto cutIndex = 0;
      auto tmpCutIndex = Form("");
      iROI == 0 ? iNsigmaUp=iROI : iNsigmaUp=iROI+1;
      auto fullCutSettingsSigm = Form("%s%s_%d_%d_%d_%d_%d_%d", tmpCutSettings.Data(), tmpCutIndex, bkgFlag, sigmoidFlag, iNsigmaDown, iNsigmaUp, iNsigmaMismatchDown, iNsigmaMismatchUp);
      //std::cout << fullCutSettingsSigm <<std::endl;
      TH1D *h = (TH1D *)specFile->Get(Form("%s/fRatio_%.0f_%.0f", fullCutSettingsSigm, kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
      for(int iPtBins=6;iPtBins<used_pt_bins-1;++iPtBins){
        double cutVal = 10.;
        if (iROI==1) cutVal=12.;
        fSystematicsCorrelationsROIUp.Fill(cutVal,h->GetBinContent(iPtBins+1)-hDefault->GetBinContent(iPtBins+1));
      }
    }
    // mismatch left limit
    cutVariable=0;
    bkgFlag = 1;
    sigmoidFlag = 0;
    iNsigmaUp = 1;
    iNsigmaDown = 1;
    iNsigmaMismatchUp = 1;
    iNsigmaMismatchDown = 1;
    hDefault = (TH1D *)specFile->Get(Form("_1_0_1_1_1_1/fRatio_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
    for (int iMismatch=0; iMismatch<2; ++iMismatch){
      TString tmpCutSettings = Form("");
      auto cutIndex = 0;
      auto tmpCutIndex = Form("");
      iMismatch == 0 ? iNsigmaMismatchDown=iMismatch : iNsigmaMismatchDown=iMismatch+1;
      auto fullCutSettingsSigm = Form("%s%s_%d_%d_%d_%d_%d_%d", tmpCutSettings.Data(), tmpCutIndex, bkgFlag, sigmoidFlag, iNsigmaDown, iNsigmaUp, iNsigmaMismatchDown, iNsigmaMismatchUp);
      //std::cout << fullCutSettingsSigm <<std::endl;
      TH1D *h = (TH1D *)specFile->Get(Form("%s/fRatio_%.0f_%.0f", fullCutSettingsSigm, kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
      for(int iPtBins=6;iPtBins<used_pt_bins-1;++iPtBins){
        double cutVal = 8.;
        if (iMismatch==1) cutVal=9.;
        fSystematicsCorrelationsMismatchDown.Fill(cutVal,h->GetBinContent(iPtBins+1)-hDefault->GetBinContent(iPtBins+1));
      }
    }

    // mismatch right limit
    cutVariable=0;
    bkgFlag = 1;
    sigmoidFlag = 0;
    iNsigmaUp = 1;
    iNsigmaDown=1;
    iNsigmaMismatchDown = 1;
    hDefault = (TH1D *)specFile->Get(Form("_1_0_1_1_1_1/fRatio_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
    for (int iMismatch=0; iMismatch<2; ++iMismatch){
      TString tmpCutSettings = Form("");
      auto cutIndex = 0;
      auto tmpCutIndex = Form("");
      iMismatch == 0 ? iNsigmaMismatchUp=iMismatch : iNsigmaMismatchUp=iMismatch+1;
      auto fullCutSettingsSigm = Form("%s%s_%d_%d_%d_%d_%d_%d", tmpCutSettings.Data(), tmpCutIndex, bkgFlag, sigmoidFlag, iNsigmaDown, iNsigmaUp, iNsigmaMismatchDown, iNsigmaMismatchUp);
      //printf(Form("%s%s_%d_%d_%d_%d_%d_%d", tmpCutSettings.Data(), tmpCutIndex, bkgFlag, sigmoidFlag, iNsigmaDown, iNsigmaUp, iNsigmaMismatchDown, iNsigmaMismatchUp));
      //std::cout << fullCutSettingsSigm <<std::endl;
      TH1D *h = (TH1D *)specFile->Get(Form("%s/fRatio_%.0f_%.0f", fullCutSettingsSigm, kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
      for(int iPtBins=6;iPtBins<used_pt_bins-1;++iPtBins){
        double cutVal = 12.5;
        if (iMismatch==1) cutVal=14.5;
        std::cout<<h->GetBinContent(iPtBins+1)-hDefault->GetBinContent(iPtBins+1)<<std::endl;
        fSystematicsCorrelationsMismatchUp.Fill(cutVal,h->GetBinContent(iPtBins+1)-hDefault->GetBinContent(iPtBins+1));
      }
    }

    std::cout << "* * * * * * * * * *"<<std::endl;
    std::cout << "Centrality = " << iC <<std::endl;
    double correlationDCAz = fSystematicsCorrelationsDCAz.GetCorrelationFactor();
    double covDCAz = fSystematicsCorrelationsDCAz.GetCovariance();
    double xStdDevDCAz = fSystematicsCorrelationsDCAz.GetRMS(1);
    double yStdDevDCAz = fSystematicsCorrelationsDCAz.GetRMS(2);
    std::cout << "CorrelationDCAz = " << correlationDCAz << std::endl;// "; mine = " << covDCAz/xStdDevDCAz/yStdDevDCAz <<std::endl;
    double correlationDCAxy = fSystematicsCorrelationsDCAxy.GetCorrelationFactor();
    std::cout << "CorrelationDCAxy = " << correlationDCAxy <<std::endl;
    double correlationTPCCls = fSystematicsCorrelationsTPCCls.GetCorrelationFactor();
    std::cout << "CorrelationTPCCls = " << correlationTPCCls <<std::endl;
    double correlationPID = fSystematicsCorrelationsPID.GetCorrelationFactor();
    std::cout << "CorrelationPID = " << correlationPID <<std::endl;
    double correlationChi2TPC = fSystematicsCorrelationsChi2TPC.GetCorrelationFactor();
    std::cout << "CorrelationChi2TPC = " << correlationChi2TPC <<std::endl;
    std::cout << "* * * * * * * * * *"<<std::endl;
    double correlationROIUp = fSystematicsCorrelationsROIUp.GetCorrelationFactor();
    std::cout << "CorrelationROIUp = " << correlationROIUp <<std::endl;
    std::cout << "* * * * * * * * * *"<<std::endl;
    double correlationROIDown = fSystematicsCorrelationsROIDown.GetCorrelationFactor();
    std::cout << "CorrelationROIDown = " << correlationROIDown <<std::endl;
    std::cout << "* * * * * * * * * *"<<std::endl;
    double correlationMismatchUp = fSystematicsCorrelationsMismatchUp.GetCorrelationFactor();
    std::cout << "CorrelationMismatchUp = " << correlationMismatchUp <<std::endl;
    std::cout << "* * * * * * * * * *"<<std::endl;
    double correlationMismatchDown = fSystematicsCorrelationsMismatchDown.GetCorrelationFactor();
    std::cout << "CorrelationMismatchDown = " << correlationMismatchDown <<std::endl;
    std::cout << "* * * * * * * * * *"<<std::endl;

    fSystematicsCorrelationsDCAz.Write();
    fSystematicsCorrelationsDCAxy.Write();
    fSystematicsCorrelationsTPCCls.Write();
    fSystematicsCorrelationsPID.Write();
    fSystematicsCorrelationsChi2TPC.Write();
    fSystematicsCorrelationsROIDown.Write();
    fSystematicsCorrelationsROIUp.Write();
    fSystematicsCorrelationsMismatchDown.Write();
    fSystematicsCorrelationsMismatchUp.Write();
    
    for(int iPtBins=7;iPtBins<used_pt_bins;++iPtBins){
      double pt=fSystematicUncertaintyDCAz.GetBinCenter(iPtBins);
      // sigmas
      double sigma_DCAz=fSystematicUncertaintyDCAz.GetBinContent(iPtBins);
      double sigma_PID=fSystematicUncertaintyPID.GetBinContent(iPtBins);
      double sigma_TPCCls=fSystematicUncertaintyTPCCls.GetBinContent(iPtBins);
      double sigma_DCAxy=fSystematicUncertaintyDCAxy.GetBinContent(iPtBins);
      double sigma_Chi2TPC=fSystematicUncertaintyChi2TPC.GetBinContent(iPtBins);
      double sigma_ROI_down=fSystematicUncertaintyROIDown.GetBinContent(iPtBins);
      double sigma_ROI_up=fSystematicUncertaintyROIUp.GetBinContent(iPtBins);
      double sigma_Mismatch_down=fSystematicUncertaintyMismatchDown.GetBinContent(iPtBins);
      double sigma_Mismatch_up=fSystematicUncertaintyMismatchUp.GetBinContent(iPtBins);
      if (iC==2 && pt>1.4 && pt<1.5) sigma_Mismatch_down=0.;
      double sigma_Prim=fSystematicUncertaintyPrim.GetBinContent(iPtBins);
      double sigma_Eff=fSystematicUncertaintyEff.GetBinContent(iPtBins);
      double sigma_TFF=fSystematicUncertaintyTFF.GetBinContent(iPtBins);
      double fraction_uncorr_DCAz=1.-std::abs(correlationDCAz);
      double fraction_uncorr_PID=1.-std::abs(correlationPID);
      double fraction_uncorr_TPCCls=1.-std::abs(correlationTPCCls);
      double fraction_uncorr_ROI_up=1.-std::abs(correlationROIUp);
      double fraction_uncorr_ROI_down=1.-std::abs(correlationROIDown);
      double fraction_uncorr_Mismatch_up=1.-std::abs(correlationMismatchUp);
      double fraction_uncorr_Mismatch_down=1.-std::abs(correlationMismatchDown);
      double fraction_uncorr_DCAxy=1.-std::abs(correlationDCAxy);
      double fraction_uncorr_Chi2TPC=1.-std::abs(correlationChi2TPC);

      double totSys=TMath::Sqrt(sigma_DCAz*sigma_DCAz*fraction_uncorr_DCAz*fraction_uncorr_DCAz
      +sigma_PID*sigma_PID*fraction_uncorr_PID*fraction_uncorr_PID
      +sigma_TPCCls*sigma_TPCCls*fraction_uncorr_TPCCls*fraction_uncorr_TPCCls
      +sigma_DCAxy*sigma_DCAxy*fraction_uncorr_DCAxy*fraction_uncorr_DCAxy
      +sigma_Chi2TPC*sigma_Chi2TPC*fraction_uncorr_Chi2TPC*fraction_uncorr_Chi2TPC
      +sigma_ROI_down*sigma_ROI_down*fraction_uncorr_ROI_down*fraction_uncorr_ROI_down
      +sigma_ROI_up*sigma_ROI_up*fraction_uncorr_ROI_up*fraction_uncorr_ROI_up
      +sigma_Mismatch_down*sigma_Mismatch_down*fraction_uncorr_Mismatch_down*fraction_uncorr_Mismatch_down
      +sigma_Mismatch_up*sigma_Mismatch_up*fraction_uncorr_Mismatch_up*fraction_uncorr_Mismatch_up
      +sigma_Prim*sigma_Prim
      +sigma_Eff*sigma_Eff
      +sigma_TFF*sigma_TFF);
      fSystematicUncertaintyTotal.SetBinContent(iPtBins,totSys);
      fSystematicUncertaintyTotal.SetBinError(iPtBins,0);

      double totSysPtCorrelated=TMath::Sqrt(sigma_DCAz*sigma_DCAz*correlationDCAz*correlationDCAz
      +sigma_PID*sigma_PID*correlationPID*correlationPID
      +sigma_TPCCls*sigma_TPCCls*correlationTPCCls*correlationTPCCls
      +sigma_ROI_down*sigma_ROI_down*correlationROIDown*correlationROIDown
      +sigma_ROI_up*sigma_ROI_up*correlationROIUp*correlationROIUp
      +sigma_Mismatch_down*sigma_Mismatch_down*correlationMismatchDown*correlationMismatchDown
      +sigma_Mismatch_up*sigma_Mismatch_up*correlationMismatchUp*correlationMismatchUp
      +sigma_DCAxy*sigma_DCAxy*correlationDCAxy*correlationDCAxy
      +sigma_Chi2TPC*sigma_Chi2TPC*correlationChi2TPC*correlationChi2TPC
      );
      fSystematicUncertaintyTotalPtCorrelated.SetBinContent(iPtBins,totSysPtCorrelated);
      fSystematicUncertaintyTotalPtCorrelated.SetBinError(iPtBins,0);
    }
    fSystematicUncertaintyTotal.GetXaxis()->SetRangeUser(0.7,1.6);
    fSystematicUncertaintyTotalPtCorrelated.GetXaxis()->SetRangeUser(0.7,1.6);

    fSystematicUncertaintyTotal.Write();
    fSystematicUncertaintyTotalPtCorrelated.Write();

    // Compute pt correlated systematic uncertainty
    TH1D hRatio(Form("fRatio_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),kNPtBins,kPtBins);
    for (int iPtBin=5;iPtBin<kNPtBins;++iPtBin){
      hRatio.SetBinContent(iPtBin,fRatioFromVariationsTot.GetBinContent(iPtBin));
      double ptMin=hRatio.GetBinLowEdge(iPtBin);
      hRatio.SetBinError(iPtBin,hRatio.GetBinContent(iPtBin)*fSystematicUncertaintyTotal.GetBinContent(iPtBin));
    }
    hRatio.Fit("pol0","QRS","",0.7,1.6);
    hRatio.Write();

    TH1D h_trial("h_trial","h_trial",kNPtBins,kPtBins);
    TH1D fRatioDistributionTrials(Form("fRatioDistributionTrials_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),Form("%.0f-%.0f%%", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]),5000,0.95,1.05);
    for (int iTrial=0;iTrial<nTrials;++iTrial){
      double nsigma=gRandom->Gaus(0,1);
      for (int iPtBin=5;iPtBin<kNPtBins;++iPtBin){
        h_trial.SetBinContent(iPtBin,fRatioFromVariationsTot.GetBinContent(iPtBin)+nsigma*fSystematicUncertaintyTotalPtCorrelated.GetBinContent(iPtBin));
        h_trial.SetBinError(iPtBin,hRatio.GetBinContent(iPtBin)*fSystematicUncertaintyTotal.GetBinContent(iPtBin));
      }
      h_trial.Fit("pol0","QR","",0.7,1.6);
      fRatioDistributionTrials.Fill(h_trial.GetFunction("pol0")->GetParameter(0));
    }
    //fRatioDistributionTrials.Write();
    
    for (int iPtBin=5;iPtBin<kNPtBins;++iPtBin){  
      fSystematicUncertaintyTotalPtCorrelated.SetBinContent(iPtBin,fSystematicUncertaintyTotalPtCorrelated.GetBinContent(iPtBin)*fRatioFromVariationsTot.GetBinContent(iPtBin));
    }

    TCanvas cRatio(Form("cRatio_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]), "cRatio");
    TLegend lPionRatio(0.2,0.65,0.4,0.85);
    //TH1D *hijingPionRatio=(TH1D*)hijingFile->Get(Form("fPionRatio_%.0f_%.0f", kCentBinsLimitsPion[iC][0], kCentBinsLimitsPion[iC][1]));
    //hijingPionRatio->GetXaxis()->SetRangeUser(0.7,1.6);
    cRatio.SetTicks(1, 1);
    hRatio.SetMarkerStyle(20);
    hRatio.SetMarkerSize(0.8);
    hRatio.GetXaxis()->SetTitle("#it{p}_{T} (GeV/#it{c})");
    hRatio.GetYaxis()->SetTitle("Ratio #pi^{-}/#pi^{+}");
    hRatio.GetXaxis()->SetRangeUser(0.71,1.6);
    hRatio.GetYaxis()->SetRangeUser(0.92, 1.08);
    hRatio.SetStats(0);
    hRatio.SetLineColor(kWhite);
    hRatio.SetMarkerColor(kWhite);
    hRatio.Draw();
    hRatio.SetLineColor(centrality_colors[iC]);
    hRatio.SetMarkerColor(centrality_colors[iC]);
    TGraphErrors gRatio(&hRatio);
    for (int iPtBin=5;iPtBin<kNPtBins;++iPtBin){  
      hRatio.SetBinError(iPtBin,fSystematicUncertaintyTotalPtCorrelated.GetBinContent(iPtBin)*fRatioFromVariationsTot.GetBinContent(iPtBin));
    }
    TGraphErrors gRatioCorr(&hRatio);
    gRatio.Draw("P5");
    gRatio.GetXaxis()->SetRangeUser(0.70,1.6);
    gRatio.GetYaxis()->SetRangeUser(0.92, 1.08);
    gRatioCorr.SetFillStyle(3145);
    gRatioCorr.SetFillColor(centrality_colors[iC]);
    gRatioCorr.Draw("P5 same");
    hRatio.GetFunction("pol0")->Draw("same");
    // hijingPionRatio->SetMarkerStyle(0);
    // hijingPionRatio->SetMarkerSize(0);
    // hijingPionRatio->SetFillColor(kGreen+1);
    // hijingPionRatio->SetFillStyle(3154);
    //hijingPionRatio->Draw("e3same");
    lPionRatio.AddEntry(&gRatio,"Data");
    lPionRatio.AddEntry(hRatio.GetFunction("pol0"),"Fit");
    //lPionRatio.AddEntry(hijingPionRatio,"HIJING");
    TLatex chi2(1.2, 1.06, Form("#chi^{2}/NDF = %.2f/%d", hRatio.GetFunction("pol0")->GetChisquare(), hRatio.GetFunction("pol0")->GetNDF()));
    chi2.SetTextSize(28);
    TLatex p0(1.2, 1.04, Form("R = %.4f #pm %.4f", hRatio.GetFunction("pol0")->GetParameter(0), hRatio.GetFunction("pol0")->GetParError(0)));
    p0.SetTextSize(28);
    chi2.Draw("same");
    p0.Draw("same");
    //lPionRatio.Draw("same");
    cRatio.Modified();
    cRatio.Write();
    cRatio.Print(Form("%s/%s.pdf", kPlotDir, hRatio.GetName()));

    TCanvas cPtCorrelatedError(fRatioDistributionTrials.GetName(),fRatioDistributionTrials.GetTitle());
    fRatioDistributionTrials.GetXaxis()->SetRangeUser(fRatioDistributionTrials.GetMean()-5*fRatioDistributionTrials.GetRMS(),fRatioDistributionTrials.GetMean()+5*fRatioDistributionTrials.GetRMS());
    fRatioDistributionTrials.GetXaxis()->SetTitle("R(#pi^{-}/#pi^{+})");
    fRatioDistributionTrials.GetYaxis()->SetTitle("Entries");
    fRatioDistributionTrials.Draw("");
    gPad->Update();
    TPaveStats* paveStat = (TPaveStats *)fRatioDistributionTrials.FindObject("stats");
    paveStat->SetOptStat(110001110);
    cPtCorrelatedError.Modified();
    cPtCorrelatedError.Print(Form("plots/%s.pdf",fRatioDistributionTrials.GetName()));
    fRatioDistributionTrials.Write();
    //std::cout<<"Correlations fitEff = "<<fSystematicsCorrelationsEffFit.GetCorrelationFactor()<<std::endl;

    // save ratio plots
    TLegend lSys(0.2,0.6,0.4,0.8);
    fSystematicUncertaintyTotal.GetXaxis()->SetRangeUser(0.7,1.6);
    fSystematicUncertaintyTotal.GetXaxis()->SetTitle("#it{p}_{T} (GeV/#it{c})");
    fSystematicUncertaintyTotal.GetYaxis()->SetTitle("Systematic Uncertainty");
    fSystematicUncertaintyTotal.SetMinimum(0.);
    fSystematicUncertaintyTotal.GetYaxis()->SetRangeUser(0.,0.02);
    TCanvas cSysError(fSystematicUncertaintyTotal.GetName(),fSystematicUncertaintyTotal.GetTitle());
    fSystematicUncertaintyTotal.SetLineWidth(2);
    fSystematicUncertaintyTotal.Draw("histo");
    fSystematicUncertaintyTotalPtCorrelated.SetLineColor(kRed);
    fSystematicUncertaintyTotalPtCorrelated.SetLineWidth(2);
    fSystematicUncertaintyTotalPtCorrelated.Draw("histo same");
    lSys.AddEntry(&fSystematicUncertaintyTotal,"#it{p}_{T}-uncorrelated");
    lSys.AddEntry(&fSystematicUncertaintyTotalPtCorrelated,"#it{p}_{T}-correlated");
    lSys.Draw("same");
    gPad->Update();
    paveStat = (TPaveStats *)fSystematicUncertaintyTotal.FindObject("stats");
    paveStat->SetOptStat(0);
    cSysError.Modified();
    cSysError.Print(Form("plots/%s.pdf",fSystematicUncertaintyTotal.GetName()));
    cSysError.Write();
  }
  outFile->Close();
  swatch.Stop();
  std::cout << "Elapsed (real) time = " << swatch.RealTime() << " s" << std::endl;
}
