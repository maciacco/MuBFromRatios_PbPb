// SignalUnbinnedMC.cpp

#include <stdlib.h>
#include <string>

#include <Riostream.h>
#include <TFile.h>
#include <TDirectory.h>
#include <TH3F.h>
#include <TH1D.h>
#include <TF1.h>
#include <TStyle.h>
#include <TString.h>
#include <TLatex.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TPad.h>

#include <RooRealVar.h>
#include <RooDataHist.h>
#include <RooPlot.h>
#include <RooArgList.h>
#include <RooAbsPdf.h>
#include <RooExponential.h>
#include <RooPolynomial.h>
#include <RooGaussian.h>
#include <RooAddPdf.h>
#include <RooBinning.h>
#include <RooFitResult.h>
#include <RooGenericPdf.h>
#include <RooCBShape.h>
#include <RooHist.h>
#include <RooFitResult.h>

#include "../utils/Utils.h"
#include "../utils/Config.h"
#include "../utils/RooGausExp.h"
#include "../utils/RooDSCBShape.h"
#include "../utils/RooGausDExp.h"

using namespace utils;
using namespace proton;


const double fit_peak = false;

const double kNSigma = 3; // define interval for bin counting

void SignalBinnedMC(const char *cutSettings = "", const double roi_nsigma = 8., const bool binCounting = false, const int bkg_shape = 1, const char *inFileDat = "mc_20g7_likeData_largeNsigma", const char *outFileName = "SignalProton", const char *outFileOption = "recreate", const bool extractSignal = true, const bool binCountingNoFit = false)
{
  int iNsigma = 0;
  if (roi_nsigma > 7.8 && roi_nsigma < 8.1) iNsigma = 1;
  else if (roi_nsigma > 8.1) iNsigma = 2;

  // make signal extraction plots directory
  system(Form("mkdir %s/signal_extraction", kPlotDir));
  system(Form("mkdir %s/signal_extraction_preliminary", kPlotDir));

  // killing RooFit output
  RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
  RooMsgService::instance().setSilentMode(true);
  gErrorIgnoreLevel = kError; // Suppressing warning outputs
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(1111);

  TFile *outFile = TFile::Open(TString::Format("%s/%s.root", kOutDir, outFileName), outFileOption); // output file
  if (outFile->GetDirectory(Form("%s_%d_%d", cutSettings, binCounting, bkg_shape)))
    return;
  TDirectory *dirOutFile = outFile->mkdir(Form("%s_%d_%d", cutSettings, binCounting, bkg_shape));
  TFile *inputMcFile = TFile::Open(TString::Format("%s/%s.root", kDataDir, inFileDat)); // open data TFile

  if (!inputMcFile)
  {
    if (kVerbose) std::cout << "File not found!" << std::endl; // check data TFile opening
    return;
  }

  // get TTList
  std::string listName_mcFalse = Form("nuclei_proton_mcFalse_%s", cutSettings);
  TTList *list_mcFalse = (TTList *)inputMcFile->Get(listName_mcFalse.data());

  // merge antimatter + matter histograms
  std::string histNameTPCA = Form("f%sTPCcounts", kAntimatterMatter[0]);
  std::string histNameTPCM = Form("f%sTPCcounts", kAntimatterMatter[1]);
  std::string histNameA = Form("f%sTOFnSigma", kAntimatterMatter[0]);
  std::string histNameM = Form("f%sTOFnSigma", kAntimatterMatter[1]);
  TH3F *fTPCSignalA = (TH3F *)list_mcFalse->Get(histNameTPCA.data());
  TH3F *fTOFSignalA = (TH3F *)list_mcFalse->Get(histNameA.data());
  TH3F *fTPCSignalAll = (TH3F *)fTPCSignalA->Clone(fTPCSignalA->GetName());
  TH3F *fTOFSignalAll = (TH3F *)fTOFSignalA->Clone(fTOFSignalA->GetName());
  TH3F *fTPCSignalM = (TH3F *)list_mcFalse->Get(histNameTPCM.data());
  TH3F *fTOFSignalM = (TH3F *)list_mcFalse->Get(histNameM.data());
  fTPCSignalAll->Add(fTPCSignalM);
  fTOFSignalAll->Add(fTOFSignalM);

  /////////////////////////////////////////////////////////////////////////////////////
  // FIT PROTON PEAK - SAVE PARAMETERS
  /////////////////////////////////////////////////////////////////////////////////////
  double fitParameterMean[kNCentClasses][kNPtBins];
  double fitParameterSigma[kNCentClasses][kNPtBins];
  double fitParameterAlphaL[kNCentClasses][kNPtBins];
  double fitParameterAlphaR[kNCentClasses][kNPtBins];
  /////////////////////////////////////////////////////////////////////////////////////

  for (int iMatt = 1; iMatt > -1; --iMatt)
  { // loop on antimatter/matter
    // Get histograms from file
    std::string histNameTPC = Form("f%sTPCcounts", kAntimatterMatter[iMatt]);
    std::string histName = Form("f%sTOFnSigma", kAntimatterMatter[iMatt]);
    TH3F *fTOFSignal1 = (TH3F *)list_mcFalse->Get(histName.data());
    fTOFSignal1->SetName("A");
    // TH3F *fTOFSignal2 = (TH3F *)list_20g7->Get(histName.data());
    // fTOFSignal1->SetName("B");
    if (!fTOFSignal1)
    {
      if (kVerbose) std::cout << "Hstogram not found!" << std::endl; // check data TFile opening
      return;
    }

    TH3F *fTPCSignal = (TH3F *)list_mcFalse->Get(histNameTPC.data());
    TH3F *fTOFSignal = (TH3F *)fTOFSignal1->Clone(histName.data());
    // if (ADD20g7)
    //   fTOFSignal->Add(fTOFSignal2);

    // make plot subdirectory
    system(Form("mkdir %s/signal_extraction/%s_%s_%d_%d", kPlotDir, kAntimatterMatter[iMatt], cutSettings, binCounting, bkg_shape));
    system(Form("mkdir %s/signal_extraction_preliminary/%s_%s_%d_%d", kPlotDir, kAntimatterMatter[iMatt], cutSettings, binCounting, bkg_shape));

    dirOutFile->cd();
    /* for (int iCent = 0; iCent < kNCentClasses; ++iCent) */
    for (int iCent = 0; iCent < kNCentClasses; ++iCent)
    {                                                                          // loop over centrality classes
      TH1D fTOFrawYield("fRawYield", "fRawYield", kNPtBins, kPtBins);          // declare raw yields histogram
      TH1D fTPCrawYield("fTPCRawYield", "fTPCRawYield", kNPtBins, kPtBins);          // declare raw yields histogram
      fTPCrawYield.GetXaxis()->SetRangeUser(0.3,0.85);
      TH1D fSignificance("fSignificance", "fSignificance", kNPtBins, kPtBins); // declare significance
      TH1D fSigma("fSigma", "fSigma", kNPtBins, kPtBins);
      TH1D fMean("fMean", "fMean", kNPtBins, kPtBins);
      TH1D fAlphaL("fAlphaL", "fAlphaL", kNPtBins, kPtBins);
      TH1D fAlphaR("fAlphaR", "fAlphaR", kNPtBins, kPtBins);
      int nUsedPtBins = 42; // up to 2.00 GeV/c with train binning

      for (int iPtBin = 1; iPtBin < nUsedPtBins + 1; ++iPtBin) // full train data binning
      { // loop on pT bins
        double roi_nsigma_up = roi_nsigma;
        double roi_nsigma_down = roi_nsigma;

        double ptMin = kPtBins[iPtBin-1];//fTOFrawYield.GetXaxis()->GetBinLowEdge(iPtBin);
        double ptMax = kPtBins[iPtBin];//fTOFrawYield.GetXaxis()->GetBinUpEdge(iPtBin);
        double centMin = kCentBinsLimitsProton[iCent][0];
        double centMax = kCentBinsLimitsProton[iCent][1];

        int pTbinsIndexMin = fTOFSignal->GetYaxis()->FindBin(ptMin);
        int pTbinsIndexMax = fTOFSignal->GetYaxis()->FindBin(ptMax - 0.005);
        int centBinMin = fTOFSignal->GetXaxis()->FindBin(kCentBinsLimitsProton[iCent][0]);      //kCentBinsProton[iCent][0];
        int centBinMax = fTOFSignal->GetXaxis()->FindBin(kCentBinsLimitsProton[iCent][1] - 1.); //kCentBinsProton[iCent][1];

        // project histogram
        if (kVerbose) std::cout << "Pt bins: min=" << ptMin << "; max=" << ptMax << "; indexMin=" << pTbinsIndexMin << "; indexMax=" << pTbinsIndexMax << "; centralityBinMin=" << centBinMin << "; centralityBinMax=" << centBinMax << std::endl;
        TH1D *tpcSignalProjection = fTPCSignal->ProjectionZ(Form("f%sTPCSignal_%.0f_%.0f_%.2f_%.2f", kAntimatterMatter[iMatt], centMin, centMax, fTPCSignal->GetYaxis()->GetBinLowEdge(pTbinsIndexMin), fTPCSignal->GetYaxis()->GetBinUpEdge(pTbinsIndexMax)), centBinMin, centBinMax, pTbinsIndexMin, pTbinsIndexMax);
        TH1D *tofSignalProjection = fTOFSignal->ProjectionZ(Form("f%sTOFSignal_%.0f_%.0f_%.2f_%.2f", kAntimatterMatter[iMatt], centMin, centMax, fTOFSignal->GetYaxis()->GetBinLowEdge(pTbinsIndexMin), fTOFSignal->GetYaxis()->GetBinUpEdge(pTbinsIndexMax)), centBinMin, centBinMax, pTbinsIndexMin, pTbinsIndexMax);
        tofSignalProjection->Rebin(2);

        // project histogram (antimatter + matter)
        TH1D *tpcSignalProjectionAll = fTPCSignalAll->ProjectionZ(Form("f%sTPCSignalAll_%.0f_%.0f_%.2f_%.2f", kAntimatterMatter[iMatt], centMin, centMax, fTPCSignalAll->GetYaxis()->GetBinLowEdge(pTbinsIndexMin), fTPCSignalAll->GetYaxis()->GetBinUpEdge(pTbinsIndexMax)), centBinMin, centBinMax, pTbinsIndexMin, pTbinsIndexMax);
        TH1D *tofSignalProjectionAll = fTOFSignalAll->ProjectionZ(Form("f%sTOFSignalAll_%.0f_%.0f_%.2f_%.2f", kAntimatterMatter[iMatt], centMin, centMax, fTOFSignalAll->GetYaxis()->GetBinLowEdge(pTbinsIndexMin), fTOFSignalAll->GetYaxis()->GetBinUpEdge(pTbinsIndexMax)), centBinMin, centBinMax, pTbinsIndexMin, pTbinsIndexMax);
        tofSignalProjectionAll->Rebin(1);

        // limits
        double nSigmaLeft = -20;
        double nSigmaRight = -10.;
        if (ptMin > 1.51)
        {
          if (ptMin > 1.55)
          {
            nSigmaLeft = -18.;
            nSigmaRight = -10.;
          };
          if (ptMin > 1.64)
          {
            nSigmaLeft = -16.;
            nSigmaRight = -10.;
          };
          if (ptMin > 1.89)
          {
            nSigmaLeft = -15.;
            nSigmaRight = -5.;
          };
          if (ptMin > 2.09)
          {
            nSigmaLeft = -14.;
            nSigmaRight = -5.;
          };
          if (ptMin > 2.19)
          {
            nSigmaLeft = -12.;
            nSigmaRight = -5.;
          };
          if (ptMin > 2.49)
          {
            nSigmaLeft = -9.;
            nSigmaRight = -5.;
          };
          tofSignalProjectionAll->GetXaxis()->SetRangeUser(nSigmaLeft, nSigmaRight);
          tofSignalProjection->GetXaxis()->SetRangeUser(nSigmaLeft, nSigmaRight);
          double maximum = tofSignalProjectionAll->GetBinCenter(tofSignalProjectionAll->GetMaximumBin());
          nSigmaLeft = maximum + 2.5;
          if (ptMin > 2.1) nSigmaLeft = maximum + 2.;
          if (ptMin > 2.49) nSigmaLeft = maximum + 1.;
          nSigmaRight = nSigmaLeft + 3.;
          if (kVerbose) std::cout << "nSigmaLeft = " << nSigmaLeft << std::endl;
        }
        double minNsigma = 15., maxNsigma = 25.;
        if (iCent==4||iCent==2) {maxNsigma = 30;}

        // DEFINE SIGNAL REGION
        TF1 signalRegionFit("signalRegionFit", "gaus", -20., 20.);
        signalRegionFit.SetParLimits(0, 1., 1.e7);
        signalRegionFit.SetParLimits(1, -0.4, 0.4);
        signalRegionFit.SetParLimits(2, 0.8, 1.6);
        signalRegionFit.SetLineColor(kBlue);
        tofSignalProjectionAll->GetXaxis()->SetRangeUser(-.5, .5);
        double maximum_signal = tofSignalProjectionAll->GetBinCenter(tofSignalProjectionAll->GetMaximumBin());
        tofSignalProjectionAll->GetXaxis()->SetRangeUser(-20., 20.);
        tofSignalProjectionAll->Fit("signalRegionFit", "QRL+", "", maximum_signal - 1., maximum_signal + 1.);
        double mean_tmp = signalRegionFit.GetParameter(1);
        double rms_tmp = signalRegionFit.GetParameter(2);

        // roofit data
        RooRealVar tofSignal("tofSignal", "n#sigma_{p}", nSigmaLeft, maxNsigma, "a.u.");

        // roofit histogram
        RooDataHist data("data", "data", RooArgList(tofSignal), tofSignalProjection);

        // roofit histogram
        RooDataHist dataAll("dataAll", "dataAll", RooArgList(tofSignal), tofSignalProjectionAll);
        if (kVerbose) std::cout << "Number of entries (roofit) = " << dataAll.sumEntries() << std::endl;
        if (kVerbose) std::cout << "Number of entries (root) = " << tofSignalProjectionAll->GetEntries() << std::endl;

        // build composite model
        RooRealVar mean("#mu", "mean", -1., 1., "a.u.");
        RooRealVar *sigma = new RooRealVar("#sigma", "sigma", 1.0, 1.4, "a.u.");
        RooRealVar *alphaL = new RooRealVar("#alpha_{L}", "alphaL", -1.5, -0.8);
        RooRealVar *alphaR = new RooRealVar("#alpha_{R}", "alphaR", 0.8, 1.5);

        RooAbsPdf *signal = new RooGausDExp("signal", "signal", tofSignal, mean, *sigma, *alphaL, *alphaR);

        RooAbsPdf *background1;
        RooAbsPdf *background0;
        RooAbsPdf *background2;
        RooRealVar *slope1;
        RooRealVar *slope2;
        RooRealVar *nBackground1;
        RooRealVar *nBackground2;
        RooAddPdf *modelPeak;
        RooFitResult *r;

        RooAddPdf *model;
        RooRealVar nSignal("N_{sig}", "nSignal", 1., 1.e2);

        slope1 = new RooRealVar("#tau_{1}", "slope1", -10., 10.);
        slope2 = new RooRealVar("#tau_{2}", "slope2",-0.9, -20.,20.);
        //slope2->setConstant();
        nBackground1 = new RooRealVar("#it{N}_{Bkg,1}", "nBackground1", 10., 0., 1.e8);

        if (bkg_shape == 1)
        { // expo

          if (ptMin < 1.29)
          {
            background1 = (RooAbsPdf *)new RooExponential("background1", "background1", tofSignal, *slope1);
            background2 = (RooAbsPdf *)new RooExponential("background2", "background2", tofSignal, *slope2);
            nBackground2 = new RooRealVar("#it{N}_{Bkg,2}", "nBackground2", 0., 0., 1.e8);
            nBackground2->setConstant();
            model = new RooAddPdf("model", "model", RooArgList(*background1), RooArgList(*nBackground1));
          }
          else
          {
            background1 = (RooAbsPdf *)new RooExponential("background1", "background1", tofSignal, *slope1);
            background0 = new RooAddPdf("background0","background0",RooArgList(*background1),RooArgList(*nBackground1));
            background2 = (RooAbsPdf *)new RooExponential("background2", "background2", tofSignal, *slope2);
            nBackground2 = new RooRealVar("#it{N}_{Bkg,2}", "nBackground2",0.,  0., 1.e8);
            //nBackground2->setConstant();
            model = new RooAddPdf("model", "model", RooArgList(*background1, *background2), RooArgList(*nBackground1, *nBackground2));
          }
        }
        else
          if (kVerbose) std::cout << "!!!!!" << std::endl; // TODO: UPDATE FOLLOWING BLOCK

        int covQ = -999;

        if (extractSignal)
        {
          roi_nsigma_down = roi_nsigma;
          roi_nsigma_up = roi_nsigma+1;
          if (ptMin > 1.51) roi_nsigma_down=roi_nsigma-2.5; // default = 6sigma
          if (ptMin > 1.99) {roi_nsigma_down=roi_nsigma-3.5; // default = 5sigma
            roi_nsigma_up=roi_nsigma+1.;
          }
          if (ptMin > 2.49) {
            roi_nsigma_down=roi_nsigma-4.; // default = 4sigma
            roi_nsigma_up=roi_nsigma+2;
          }
          if (iCent==4||iCent==2) roi_nsigma_up = 16;
          tofSignal.setRange("leftSideband", nSigmaLeft, mean_tmp - roi_nsigma_down * rms_tmp);
          tofSignal.setRange("rightSideband", mean_tmp + roi_nsigma_up * rms_tmp, maxNsigma);
          
          // fit TOF signal distribution
          if (ptMin>1.99){
            for(int I=0;I<2;++I)background1->fitTo(dataAll, RooFit::Range("rightSideband"));
            slope1->setConstant();
            for (int I=0;I<2;++I)background0->fitTo(data, RooFit::Range("rightSideband"));
            /* slope1->setConstant();*/
            nBackground1->setConstant();
          }
          else {
            for (int I=0;I<2;++I)model->fitTo(dataAll, RooFit::Range("leftSideband,rightSideband"));
          }
          for (int I=0;I<2;++I) r = model->fitTo(data, RooFit::Save(), RooFit::Range("leftSideband,rightSideband"));

          if (kVerbose) std::cout << "fit status: " << r->status() << ";" << std::endl;
          if (kVerbose) std::cout << "covariance quality: " << r->covQual() << std::endl;
          covQ = r->covQual();
          
          // signal range
          tofSignal.setRange("signalRange", mean_tmp - roi_nsigma_down * rms_tmp, mean_tmp + roi_nsigma_up * rms_tmp);
        }

        // frame
        int nBins = tofSignalProjection->GetNbinsX();
        TString plotTitle = TString::Format("%.2f#leq #it{p}_{T}<%.2f GeV/#it{c}, %.0f-%.0f%%", ptMin, ptMax, kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]);
        RooPlot *xframe = tofSignal.frame(RooFit::Bins(nBins), RooFit::Title(plotTitle), RooFit::Name(Form("f%sNSigma_%.0f_%.0f_%.2f_%.2f", kAntimatterMatter[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1], ptMin, ptMax)));
        data.plotOn(xframe, RooFit::Name("dataNsigma"));
        tofSignal.setRange("full", kTOFnSigmaMin, kTOFnSigmaMax);
        tofSignal.setRange("model", nSigmaLeft, maxNsigma);
        if (extractSignal)
        {
          model->plotOn(xframe, RooFit::Name("model"), RooFit::LineColor(kBlue), RooFit::NormRange("leftSideband,rightSideband"), RooFit::Range("leftSideband,rightSideband"));+
          model->paramOn(xframe, RooFit::Label(TString::Format("#chi^{2}/NDF = %2.2f", xframe->chiSquare("model", "dataNsigma"))), RooFit::Layout(0.58812,0.911028,0.861955));
          xframe->getAttLine()->SetLineWidth(0);
          xframe->getAttText()->SetTextFont(44);
          xframe->getAttText()->SetTextSize(16);
          xframe->remove("model",false);
          model->plotOn(xframe, RooFit::Name("model"), RooFit::LineColor(kBlue), RooFit::NormRange("leftSideband,rightSideband"), RooFit::Range("model"));
          xframe->GetYaxis()->SetMaxDigits(2);

          //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          // FIT PROTON PEAK
          //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          if (fit_peak){
            nBackground1->setConstant(true);
            nBackground2->setConstant(true);
            slope1->setConstant(true);
            slope2->setConstant(true);
            if (iMatt == 0){
              mean.setVal(fitParameterMean[iCent][iPtBin]);
              mean.setConstant(true);
              sigma->setVal(fitParameterSigma[iCent][iPtBin]);
              sigma->setConstant(true);
              alphaL->setVal(fitParameterAlphaL[iCent][iPtBin]);
              alphaL->setConstant(true);
              alphaR->setVal(fitParameterAlphaR[iCent][iPtBin]);
              alphaR->setConstant(true);
            }
            modelPeak = new RooAddPdf("model2", "model2", RooArgList(*background1, *background2, *signal), RooArgList(*nBackground1, *nBackground2, nSignal));
            modelPeak->fitTo(data, RooFit::Range("signalRange"));
            modelPeak->fitTo(data, RooFit::Range("signalRange"));
            if (iMatt == 1){
              fitParameterMean[iCent][iPtBin]=mean.getVal();
              fitParameterSigma[iCent][iPtBin]=sigma->getVal();
              fitParameterAlphaL[iCent][iPtBin]=alphaL->getVal();
              fitParameterAlphaR[iCent][iPtBin]=alphaR->getVal();
            }
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            modelPeak->plotOn(xframe, RooFit::Name("modelPeak"), RooFit::LineColor(kRed), RooFit::NormRange("signalRange"), RooFit::Range("signalRange"));
          }

          // background integral
          std::cout << "ptMin = " << ptMin << std::endl;
          double bkgIntegral_1 = (((RooAbsPdf *)(model->pdfList().at(0)))->createIntegral(RooArgSet(tofSignal), RooFit::NormSet(RooArgSet(tofSignal)), RooFit::Range("signalRange")))->getVal();
          double bkgIntegral_2 = 0;
          if (ptMin > 1.29) bkgIntegral_2 = (((RooAbsPdf *)(model->pdfList().at(1)))->createIntegral(RooArgSet(tofSignal), RooFit::NormSet(RooArgSet(tofSignal)), RooFit::Range("signalRange")))->getVal();
          double bkgIntegral_val = nBackground1->getVal() *bkgIntegral_1 + nBackground2->getVal() * bkgIntegral_2;

          double rawYield, rawYieldError, counts;
          if (binCounting)
          {
            // total counts
            counts = data.sumEntries(Form("tofSignal>%f && tofSignal<%f", mean_tmp - roi_nsigma_down * rms_tmp, mean_tmp + roi_nsigma_up * rms_tmp));
            rawYield = counts - bkgIntegral_val; // signal=counts-error
            if (kVerbose) std::cout << "Counts: " << counts << ", Background: "<< bkgIntegral_val << std::endl;
            rawYieldError = TMath::Sqrt(counts); // counts=signal+bkg => correct error from poisson statistics
            if (binCountingNoFit)
            {
              rawYield = data.sumEntries(Form("tofSignal>%f && tofSignal<%f", -0.7815 * kNSigma, 0.7815 * kNSigma)); // only for He4 in signal loss studies
            }
            if (r->status()!=0 || r->covQual()<1.5) rawYield=0;
          }
          else
          {
            rawYield = nSignal.getVal();
            rawYieldError = nSignal.getError();
          }

          tpcSignalProjectionAll->Rebin(1);
          tpcSignalProjection->Rebin(1);
          tpcSignalProjection->SetMarkerStyle(20);
          tpcSignalProjection->SetMarkerSize(1.);
          tpcSignalProjection->SetDrawOption("pe");
          tpcSignalProjection->SetMarkerColor(kBlack);
          tpcSignalProjection->GetXaxis()->SetLabelSize(0.04);
          tpcSignalProjection->GetXaxis()->SetTitleSize(0.04);
          tpcSignalProjection->GetXaxis()->SetTitle("n#sigma_{p}");
          tpcSignalProjection->GetYaxis()->SetTitle(Form("Entries / %.2f",tpcSignalProjection->GetBinWidth(1)));

          tpcSignalProjectionAll->GetXaxis()->SetRangeUser(-15.,-4.);
          double max_bkg = tpcSignalProjectionAll->GetBinCenter(tpcSignalProjectionAll->GetMaximumBin());
          for(int i=0;i<2;i++)tpcSignalProjectionAll->Fit("gaus","RLM+","",max_bkg-2.5,max_bkg+2.5);
          tpcSignalProjectionAll->GetXaxis()->SetRangeUser(-24.,24.);
          double left_fit_range_factor = 2.5;
          if (ptMin > 0.64 && ptMin<0.84) left_fit_range_factor = 2.;
          else if (ptMin > 0.84) left_fit_range_factor = -1.5;
          double left_range = -999;
          for (int iB=1;iB<tpcSignalProjectionAll->GetNbinsX();++iB){
            if (tpcSignalProjectionAll->GetBinCenter(iB)>(tpcSignalProjectionAll->GetFunction("gaus")->GetParameter(1)+left_fit_range_factor*tpcSignalProjectionAll->GetFunction("gaus")->GetParameter(2))){
              left_range=tpcSignalProjectionAll->GetXaxis()->GetBinLowEdge(iB);
              break;
            }
          }
          RooRealVar nSigmaTPC("nSigmaTPC","n#sigma_{TPC,p}",left_range,24.,"a.u.");
          RooDataHist hNSigmaTPC("hNSigmaTPC","hNSigmaTPC",RooArgList(nSigmaTPC),tpcSignalProjection);
          RooDataHist hNSigmaTPCAll("hNSigmaTPCAll","hNSigmaTPCAll",RooArgList(nSigmaTPC),tpcSignalProjectionAll);
          RooRealVar meanTPC("#mu","#mu",-2.,2.);
          RooRealVar sigmaTPC("#sigma","#sigma",0.8,3.);
          RooRealVar alpha1TPC("#alpha_{1}","#alpha_{1}",-3.,-0.8);
          RooRealVar alpha2TPC("#alpha_{2}","#alpha_{2}",0.8,3.);
          RooRealVar meanTPCBkg("#mu_{Bkg}","#mu_{Bkg}",-20.,-6.);
          if (ptMin>0.7) meanTPCBkg.setMax(-5.);
          if (ptMin>0.8) {
            meanTPCBkg.setMin(-8.);
            meanTPCBkg.setMax(-3.5);
          }
          RooRealVar sigmaTPCBkg("#sigma_{Bkg}","#sigma_{Bkg}",0.5,2.);
          RooRealVar tauTPCBkg("#tau_{Bkg}","#tau_{Bkg}",0.5,2.);
          RooGausDExp signalTPC("signalTPC","signalTPC",nSigmaTPC,meanTPC,sigmaTPC,alpha1TPC,alpha2TPC);
          RooGausExp bkgTPC("bkgTPC","bkgTPC",nSigmaTPC,meanTPCBkg,sigmaTPCBkg,tauTPCBkg);
          RooRealVar nSignalTPC("n_{Signal}","n_{Signal}",0.,1.e9);
          RooRealVar nBkgTPC("n_{Bkg}","n_{Bkg}",0.,1.e9);
          RooAddPdf modelTPC("modelTPC","modelTPC",RooArgList(signalTPC,bkgTPC),RooArgList(nSignalTPC,nBkgTPC));
          nSigmaTPC.setRange("fitRange",tpcSignalProjectionAll->GetFunction("gaus")->GetParameter(1)+left_fit_range_factor*tpcSignalProjectionAll->GetFunction("gaus")->GetParameter(2),2.);
          for (int i=0;i<2;++i)modelTPC.fitTo(hNSigmaTPCAll,RooFit::Range("fitRange"));
          for (int i=0;i<2;++i)modelTPC.fitTo(hNSigmaTPC,RooFit::Range("fitRange"));
          nSigmaTPC.setRange("signalRange",meanTPC.getVal()-5.*sigmaTPC.getVal(),meanTPC.getVal()+5.*sigmaTPC.getVal());
          RooPlot *frameTPC = nSigmaTPC.frame(RooFit::Name(tpcSignalProjection->GetName()),RooFit::Title(Form("%.2f#leq #it{p}_{T}<%.2f GeV/#it{c}, %.0f-%.0f%%", ptMin, ptMax, kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1])));
          hNSigmaTPC.plotOn(frameTPC,RooFit::Name("nsigmaTPC"));
          double lowTPCrange=3;
          double tpc_range_limit = (roi_nsigma-8.);
          double tpc_tmp_mean=0., tpc_tmp_rms=0.;
          nSigmaTPC.setRange("signalRange",meanTPC.getVal()-(lowTPCrange+tpc_range_limit)*sigmaTPC.getVal(),meanTPC.getVal()+(5.+tpc_range_limit)*sigmaTPC.getVal());
          modelTPC.plotOn(frameTPC,RooFit::Components("bkgTPC"),RooFit::LineColor(kGreen),RooFit::LineStyle(kDashed));
          modelTPC.plotOn(frameTPC,RooFit::Components("signalTPC"),RooFit::LineColor(kRed),RooFit::LineStyle(kDashed));
          modelTPC.plotOn(frameTPC,RooFit::Name("model"));
          modelTPC.paramOn(frameTPC,RooFit::Parameters(RooArgSet(meanTPC,sigmaTPC)),RooFit::Label(TString::Format("#chi^{2}/NDF = %2.2f", frameTPC->chiSquare("model", "nsigmaTPC"))), RooFit::Layout(0.58812,0.811028,0.561955));
          frameTPC->GetXaxis()->SetRangeUser(tpcSignalProjectionAll->GetFunction("gaus")->GetParameter(1)+left_fit_range_factor*tpcSignalProjectionAll->GetFunction("gaus")->GetParameter(2)+0.5,5);
          frameTPC->getAttText()->SetTextFont(44);
          frameTPC->getAttText()->SetTextSize(18);
          frameTPC->getAttLine()->SetLineWidth(0);
          frameTPC->Write();

          double background=0.;
          if (ptMin > 0.49){
            background = ((RooAbsPdf *)bkgTPC.createIntegral(RooArgSet(nSigmaTPC), RooFit::NormSet(RooArgSet(nSigmaTPC)), RooFit::Range("signalRange")))->getVal();
            background *= nBkgTPC.getVal();
          }
          TCanvas c(Form("%s",tpcSignalProjection->GetName()),Form("%s",tpcSignalProjection->GetName()));
          tpcSignalProjection->GetXaxis()->SetRangeUser(tpcSignalProjectionAll->GetFunction("gaus")->GetParameter(1)+left_fit_range_factor*tpcSignalProjectionAll->GetFunction("gaus")->GetParameter(2)+0.5,5);
          double rawYieldTPC = hNSigmaTPC.sumEntries(Form("nSigmaTPC>%f && nSigmaTPC<%f",meanTPC.getVal()-(lowTPCrange+tpc_range_limit)*sigmaTPC.getVal(),meanTPC.getVal()+(5.+tpc_range_limit)*sigmaTPC.getVal()));
          if(ptMin>0.49)rawYieldTPC-=background;
          double rawYieldErrorTPC = TMath::Sqrt(rawYieldTPC);
          c.SetLogy();
          frameTPC->SetMinimum(1);
          frameTPC->SetMaximum(tpcSignalProjection->GetMaximum()*1.2);
          frameTPC->Draw();
          c.Print(Form("%s/signal_extraction/%s_%s_%d_%d/TPC_cent_%.0f_%.0f_pt_%.2f_%.2f.png", kPlotDir, kAntimatterMatter[iMatt], cutSettings, binCounting, bkg_shape, kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1], ptMin, ptMax));

          // fill raw yield histogram
          fTPCrawYield.SetBinContent(fTPCrawYield.FindBin((ptMax + ptMin) / 2.), rawYieldTPC);
          fTPCrawYield.SetBinError(fTPCrawYield.FindBin((ptMax + ptMin) / 2.), rawYieldErrorTPC);
          if (ptMin>0.99 || r->covQual()<2.5 || r->status()!=0){
            fTPCrawYield.SetBinContent(fTPCrawYield.FindBin((ptMax + ptMin) / 2.), 0.);
            fTPCrawYield.SetBinError(fTPCrawYield.FindBin((ptMax + ptMin) / 2.), 0.);
          }
          

          // fill raw yield histogram
          fTOFrawYield.SetBinContent(fTOFrawYield.FindBin((ptMax + ptMin) / 2.), rawYield);
          fTOFrawYield.SetBinError(fTOFrawYield.FindBin((ptMax + ptMin) / 2.), rawYieldError);

          // fill significance histogram
          if (binCounting)
            fSignificance.SetBinContent(fSignificance.FindBin((ptMax + ptMin) / 2.), rawYield / rawYieldError);
          else
            //fSignificance.SetBinContent(fSignificance.FindBin((ptMax + ptMin) / 2.), rawYield / TMath::Sqrt(rawYield + bkgIntegral_val));
            fSignificance.SetBinError(fSignificance.FindBin((ptMax + ptMin) / 2.), 0.);

          // fill sigma histogram
          fSigma.SetBinContent(fSigma.FindBin((ptMax + ptMin) / 2.), signalRegionFit.GetParameter(2));
          fSigma.SetBinError(fSigma.FindBin((ptMax + ptMin) / 2.), signalRegionFit.GetParError(2));

          // fill mean histogram
          fMean.SetBinContent(fMean.FindBin((ptMax + ptMin) / 2.), signalRegionFit.GetParameter(1));
          fMean.SetBinError(fMean.FindBin((ptMax + ptMin) / 2.), signalRegionFit.GetParError(1));

          // fill alphaR histogram
          fAlphaR.SetBinContent(fAlphaR.FindBin((ptMax + ptMin) / 2.), alphaR->getVal());
          fAlphaR.SetBinError(fAlphaR.FindBin((ptMax + ptMin) / 2.), alphaR->getError());

          // fill alphaL histogram
          fAlphaL.SetBinContent(fAlphaL.FindBin((ptMax + ptMin) / 2.), alphaL->getVal());
          fAlphaL.SetBinError(fAlphaL.FindBin((ptMax + ptMin) / 2.), alphaL->getError());
        
        }
        // save to png
        TCanvas canv("canv", "canv");
        canv.cd();
        TPad *pad1 = new TPad("pad1", "pad1", 0.0, 0.4, 1.0, 1.0, 0);
        pad1->SetFillColor(0);
        pad1->SetFrameBorderMode(0);
        pad1->SetBottomMargin(0.06);
        pad1->SetLogy();
        pad1->Draw();
        TPad *pad2 = new TPad("pad2", "pad2", 0.0, 0.0, 1.0, 0.4, 0);
        pad2->SetFillColor(0);
        pad2->SetFrameBorderMode(0);
        pad2->SetFillColor(0);
        pad2->SetFrameBorderMode(0);
        pad2->SetTopMargin(0.10);
        pad2->SetBottomMargin(0.25);
        pad2->Draw();
        pad1->cd();
        xframe->GetXaxis()->SetLabelOffset(0.005);
        xframe->GetXaxis()->SetTitle("");
        xframe->GetXaxis()->SetTitleOffset(-0.005);
        xframe->Draw("");
        xframe->GetXaxis()->SetLabelSize(0.06);
        xframe->GetYaxis()->SetLabelSize(0.06);
        //xframe->GetYaxis()->SetRangeUser(-1.e4, 2.e4);
        xframe->GetYaxis()->SetTitleSize(0.07);
        xframe->GetYaxis()->SetTitleOffset(0.72);
        tofSignalProjection->GetXaxis()->SetRangeUser(-0.5, 0.5);
        double peakMaximum = tofSignalProjection->GetBinContent(tofSignalProjection->GetMaximumBin());
        if (kVerbose) std::cout << "peakMaximum=" << peakMaximum << std::endl;
        TLine lsx(mean_tmp - roi_nsigma_down * rms_tmp, 0, mean_tmp - roi_nsigma_down * rms_tmp, peakMaximum);
        lsx.SetLineStyle(kDashed);
        lsx.Draw("same");
        TLine ldx(mean_tmp + roi_nsigma_up * rms_tmp, 0, mean_tmp + roi_nsigma_up * rms_tmp, peakMaximum*0.75);
        if(ptMin>1.19)ldx.SetY2(peakMaximum*0.6);
        ldx.SetLineStyle(kDashed);
        ldx.Draw("same");
        canv.SetName(plotTitle);
        TLatex covarianceQuality;
        covarianceQuality.SetNDC();
        covarianceQuality.SetX(0.55);
        covarianceQuality.SetY(0.4);
        covarianceQuality.SetTitle(Form("Covariance quality = %d", covQ));
        pad1->Update();

        RooHist *hresid;
        RooPlot *frame2;

        if (extractSignal)
        {
          auto xframe1 = (RooPlot *)xframe->Clone();
          hresid = xframe1->residHist();
          frame2 = tofSignal.frame(RooFit::Title(""));
          frame2->addPlotable(hresid, "P");
          frame2->GetYaxis()->SetMaxDigits(2);
        }

        tofSignalProjection->Write();
        tpcSignalProjection->Write();
        xframe->Write();

        pad2->cd();
        frame2->Draw("");
        frame2->GetXaxis()->SetLabelOffset(0.005);
        frame2->GetXaxis()->SetTitle("n#sigma_{p} (a.u.)");
        frame2->GetXaxis()->SetTitleOffset(0.9);
        frame2->GetYaxis()->SetNdivisions(5);
        frame2->GetXaxis()->SetTitleSize(0.11);
        frame2->GetXaxis()->SetLabelSize(0.095);
        frame2->GetYaxis()->SetLabelSize(0.09);
        //frame2->GetYaxis()->SetRangeUser(-1.e4, 2.e4);
        frame2->GetYaxis()->SetTitleSize(0.095);
        frame2->GetYaxis()->SetTitleOffset(0.52);
        frame2->GetYaxis()->SetTitle(Form("#frac{ Ev. - Bg. }{ %.1f a.u. }", tofSignalProjection->GetXaxis()->GetBinWidth(2)));
        frame2->SetTitle("   ");
        //covarianceQuality.Draw("same");
        //canv.SetLogy();
        TLine lsx1(mean_tmp - roi_nsigma_down * rms_tmp, -1.e4, mean_tmp - roi_nsigma_down * rms_tmp, 2.e4);
        lsx1.SetLineStyle(kDashed);
        //lsx1.Draw("same");
        TLine ldx1(mean_tmp + roi_nsigma_up * rms_tmp, -1.e4, mean_tmp + roi_nsigma_up * rms_tmp, 2.e4);
        ldx1.SetLineStyle(kDashed);
        //ldx1.Draw("same");
        TLine zero(nSigmaLeft, 0, maxNsigma, 0);
        zero.SetLineStyle(kDashed);
        zero.Draw("same");
        pad2->Update();
        canv.Write(Form("%s_%s_%d_%d_cent_%.0f_%.0f_pt_%.2f_%.2f", kAntimatterMatter[iMatt], cutSettings, binCounting, bkg_shape, kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1], ptMin, ptMax));
        canv.Print(Form("%s/signal_extraction/%s_%s_%d_%d/cent_%.0f_%.0f_pt_%.2f_%.2f.pdf", kPlotDir, kAntimatterMatter[iMatt], cutSettings, binCounting, bkg_shape, kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1], ptMin, ptMax));
        tofSignalProjection->SetMarkerStyle(20);
        tofSignalProjection->SetMarkerSize(0.8);
        TCanvas canv1("c1", "c1");
        tofSignalProjection->Draw("pe");
        canv1.Print(Form("%s/signal_extraction_preliminary/%s_%s_%d_%d/cent_%.0f_%.0f_pt_%.2f_%.2f.pdf", kPlotDir, kAntimatterMatter[iMatt], cutSettings, binCounting, bkg_shape, kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1], ptMin, ptMax));
      } // end of loop on pt bins

      // set raw yield histogram style and write to file
      fTPCrawYield.SetTitle(TString::Format("%s raw yield, %.0f-%.0f%%", kAntimatterMatterLabel[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      fTPCrawYield.SetName(TString::Format("f%sTPCrawYield_%.0f_%.0f", kAntimatterMatter[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      fTPCrawYield.GetXaxis()->SetTitle("#it{p}_{T} (GeV/#it{c})");
      fTPCrawYield.GetYaxis()->SetTitle("#it{N_{raw}}");
      fTPCrawYield.SetMarkerStyle(20);
      fTPCrawYield.SetMarkerSize(0.8);
      fTPCrawYield.SetOption("PE");
      fTPCrawYield.Write();

      // set raw yield histogram style and write to file
      fTOFrawYield.SetTitle(TString::Format("%s raw yield, %.0f-%.0f%%", kAntimatterMatterLabel[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      fTOFrawYield.SetName(TString::Format("f%sTOFrawYield_%.0f_%.0f", kAntimatterMatter[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      fTOFrawYield.GetXaxis()->SetTitle("#it{p}_{T} (GeV/#it{c})");
      fTOFrawYield.GetYaxis()->SetTitle("#it{N_{raw}}");
      fTOFrawYield.SetMarkerStyle(20);
      fTOFrawYield.SetMarkerSize(0.8);
      fTOFrawYield.SetOption("PE");
      fTOFrawYield.Write();

      // set significance histogram style and write to file
      fSignificance.SetTitle(TString::Format("%s significance, %.0f-%.0f%%", kAntimatterMatterLabel[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      fSignificance.SetName(TString::Format("f%sSignificance_%.0f_%.0f", kAntimatterMatter[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      fSignificance.GetXaxis()->SetTitle("#it{p}_{T} (GeV/#it{c})");
      fSignificance.GetYaxis()->SetTitle("S/#sqrt{N}");
      fSignificance.SetMarkerStyle(20);
      fSignificance.SetMarkerSize(0.8);
      fSignificance.SetOption("PE");
      fSignificance.Write();

      // set sigma histogram style and write to file
      fSigma.SetTitle(TString::Format("%s sigma, %.0f-%.0f%%", kAntimatterMatterLabel[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      fSigma.SetName(TString::Format("f%sSigma_%.0f_%.0f", kAntimatterMatter[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      fSigma.GetXaxis()->SetTitle("#it{p}_{T} (GeV/#it{c})");
      fSigma.GetYaxis()->SetTitle("#sigma (GeV^2/#it{c}^{4})");
      fSigma.SetMarkerStyle(20);
      fSigma.SetMarkerSize(0.8);
      fSigma.SetOption("PE");
      fSigma.Write();

      // set mean histogram style and write to file
      fMean.SetTitle(TString::Format("%s mean, %.0f-%.0f%%", kAntimatterMatterLabel[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      fMean.SetName(TString::Format("f%sMean_%.0f_%.0f", kAntimatterMatter[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      fMean.GetXaxis()->SetTitle("#it{p}_{T} (GeV/#it{c})");
      fMean.GetYaxis()->SetTitle("#mu (GeV^2/#it{c}^{4})");
      fMean.SetMarkerStyle(20);
      fMean.SetMarkerSize(0.8);
      fMean.SetOption("PE");
      fMean.Write();

      // set sigma histogram style and write to file
      fAlphaR.SetTitle(TString::Format("%s alphaR, %.0f-%.0f%%", kAntimatterMatterLabel[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      fAlphaR.SetName(TString::Format("f%sAlphaR_%.0f_%.0f", kAntimatterMatter[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      fAlphaR.GetXaxis()->SetTitle("#it{p}_{T} (GeV/#it{c})");
      fAlphaR.GetYaxis()->SetTitle("#alpha_{R}");
      fAlphaR.SetMarkerStyle(20);
      fAlphaR.SetMarkerSize(0.8);
      fAlphaR.SetOption("PE");
      fAlphaR.Write();

      // set sigma histogram style and write to file
      fAlphaL.SetTitle(TString::Format("%s alphaL, %.0f-%.0f%%", kAntimatterMatterLabel[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      fAlphaL.SetName(TString::Format("f%sAlphaL_%.0f_%.0f", kAntimatterMatter[iMatt], kCentBinsLimitsProton[iCent][0], kCentBinsLimitsProton[iCent][1]));
      fAlphaL.GetXaxis()->SetTitle("#it{p}_{T} (GeV/#it{c})");
      fAlphaL.GetYaxis()->SetTitle("#alpha_{L}");
      fAlphaL.SetMarkerStyle(20);
      fAlphaL.SetMarkerSize(0.8);
      fAlphaL.SetOption("PE");
      fAlphaL.Write();

      //delete model, mean, sigma, alphaL, alphaR;
    } // end of loop on centrality bin
  }   // end of loop on antimatter/matter
  outFile->Close();
} // end of macro Signal.cpp