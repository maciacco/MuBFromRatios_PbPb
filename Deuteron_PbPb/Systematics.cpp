// Systematics.cpp
// This macro computes systematic errors on pol0 fit parameter

// TODO: update the generation of random cuts

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

#include "../utils/Config.h"

using namespace deuteron;

// #define USE_COUNTER

void Systematics(const int points = kNPoints, const bool cutVar = true, const bool binCountingVar = true, const bool expVar = true, const bool sigmoidVar = true, const char *outFileName = "SystematicsAll")
{
  gStyle->SetOptStat(110001110);
  gStyle->SetStatX(0.87);
  gStyle->SetStatY(0.85);
  TStopwatch swatch;
  swatch.Start(true);

  TFile *specFile = TFile::Open(Form("%s/SpectraDeuteronSys.root", kOutDir));
  TFile *outFile = TFile::Open(Form("%s/%s.root", kOutDir, outFileName), "recreate");
  TDirectory *cdHist = outFile->mkdir("hist");

  for (int iC = 0; iC < kNCentClasses-1; ++iC) // TODO: extend the analysis to the third centrality class as well
  {
    TDirectory *cdFits = outFile->mkdir(Form("fits_%.0f_%.0f", kCentBinsLimitsDeuteron[iC][0], kCentBinsLimitsDeuteron[iC][1]));
    TH1D fFitPar(Form("fFitPar_%.0f_%.0f", kCentBinsLimitsDeuteron[iC][0], kCentBinsLimitsDeuteron[iC][1]), Form("%.0f-%.0f%%", kCentBinsLimitsDeuteron[iC][0], kCentBinsLimitsDeuteron[iC][1]), 3000, 0.8, 1.);
    TH1D fProb(Form("fProb_%.0f_%.0f", kCentBinsLimitsDeuteron[iC][0], kCentBinsLimitsDeuteron[iC][1]), Form("%.0f-%.0f%%", kCentBinsLimitsDeuteron[iC][0], kCentBinsLimitsDeuteron[iC][1]), 1000., 0., 1.0);
    TH1D fRatio("fRatio", "fRatio", kNPtBins, kPtBins);

    // fill and fit ratio histogram nPoint times
    int iP = 0;
    while (iP < kNPoints)
    {
      double nUsedPtBins = 22;
      for (int iPtBin = 4; iPtBin < nUsedPtBins+1; ++iPtBin)
      {
        // extract variable which the variation is applied to
        int cutVariable = gRandom->Rndm() * 3;

        // extract cut
        int cutIndex = 0;
        if (cutVariable == 0) cutIndex = gRandom->Rndm() * kNCutDCAz;
        else if (cutVariable == 1)
        {
          cutIndex = gRandom->Rndm() * kNTPCPidSigmas;
        }
        else cutIndex = gRandom->Rndm() * kNCutTPCClusters;

        // extract background flag
        int bkgFlag = 0;// gRandom->Rndm() * 2;

        int sigmoidFlagRnd = 1;
        if (sigmoidVar)
        {
          if (fRatio.GetBinCenter(iPtBin) < 1.6)
          {
            (gRandom->Rndm() > .5) ? sigmoidFlagRnd = 1 : sigmoidFlagRnd = 0;
          }
        }

        auto fullCutSettingsSigm = Form("%s%d_%d_%d", cutSettings[cutVariable], cutIndex, bkgFlag, sigmoidFlagRnd);

        TH1D *h = (TH1D *)specFile->Get(Form("%s/fRatio_%.0f_%.0f", fullCutSettingsSigm, kCentBinsLimitsDeuteron[iC][0], kCentBinsLimitsDeuteron[iC][1]));
        fRatio.SetBinContent(iPtBin, h->GetBinContent(h->FindBin(fRatio.GetBinCenter(iPtBin))));
        fRatio.SetBinError(iPtBin, h->GetBinError(h->FindBin(fRatio.GetBinCenter(iPtBin))));
        // if(iPtBin == nUsedPtBins)
        // std::cout<<"fullCutSettings = "<<fullCutSettingsSigm<<", bin center = "<<fRatio.GetBinCenter(iPtBin)<<", content = "<< h->GetBinContent(h->FindBin(fRatio.GetBinCenter(iPtBin)))<<std::endl;
      }
      std::cout << "p=" << iP << std::endl;
      TF1 fitFunc("fitFunc", "pol0");
      auto fit = fRatio.Fit(&fitFunc, "QS");

      int ndf = 18;
      if (fit->Status() == 0 && fit->Prob() > 0.025 && fit->Prob() < 0.975 && fit->Ndf() == ndf)
      { // check chi2
        fFitPar.Fill(fitFunc.GetParameter(0));
        fProb.Fill(fitFunc.GetProb());
        cdFits->cd();
        fRatio.Write();
        ++iP;
      }
      fRatio.Reset();
    }

    cdHist->cd();
    fFitPar.GetYaxis()->SetTitle("Entries");
    fFitPar.GetXaxis()->SetTitle("R (#bar{d}/d)");
    fFitPar.SetDrawOption("histo");
    fFitPar.Rebin(8);
    fFitPar.SetFillStyle(3345);
    fFitPar.SetLineWidth(2);
    fFitPar.SetLineColor(kBlue);
    fFitPar.SetFillColor(kBlue);
    fFitPar.Write();

    TCanvas cFitPar("cFitPar", "cFitPar");
    cFitPar.cd();
    cFitPar.SetTicks(1, 1);
    fFitPar.GetXaxis()->SetRangeUser(fFitPar.GetMean()-5*fFitPar.GetRMS(),fFitPar.GetMean()+5*fFitPar.GetRMS());
    fFitPar.Draw("");
    cFitPar.Print(Form("%s/Systematics_%s.png", kPlotDir, fFitPar.GetName()));

    fProb.GetYaxis()->SetTitle("Entries");
    fProb.GetXaxis()->SetTitle("#it{P}( #chi^{2} > #chi^{2}_{#it{obs}} )");
    fProb.Write();
  }
  outFile->Close();
  swatch.Stop();
  std::cout << "Elapsed (real) time = " << swatch.RealTime() << " s" << std::endl;
}
