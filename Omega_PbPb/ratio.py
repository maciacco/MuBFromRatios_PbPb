import ROOT
import os
import pickle
import warnings
import argparse
import yaml
import pandas as pd
import matplotlib.pyplot as plt
import helpers
import numpy as np
import uproot

SPLIT = True
N_TRIALS = 10000
MAX_EFF = 1
speed_of_light = 0.0299792458
centrality_colors = [ROOT.kOrange+7, ROOT.kAzure+4, ROOT.kTeal+4, ROOT.kCyan+1, ROOT.kMagenta-3]
warnings.simplefilter(action='ignore', category=FutureWarning)
ROOT.gROOT.SetBatch()

bkg_function = ['pol','expo']

##################################################################
# read configuration file
##################################################################
config = 'config.yaml'
with open(os.path.expandvars(config), 'r') as stream:
    try:
        params = yaml.full_load(stream)
    except yaml.YAMLError as exc:
        print(exc)

DATA_PATH = params['DATA_PATH']
PSEUDODATA_PATH = params['PSEUDODATA_PATH']
BKG_PATH = params['BKG_PATH']
MC_SIGNAL_PATH = params['MC_SIGNAL_PATH']
MC_SIGNAL_PATH_GEN = params['MC_SIGNAL_PATH_GEN']
CT_BINS = params['CT_BINS']
CT_BINS_APPLY = params['CT_BINS_APPLY']
CT_BINS_CENT = params['CT_BINS_CENT']
PT_BINS = params['PT_BINS']
CENTRALITY_LIST = params['CENTRALITY_LIST']
TRAINING_COLUMNS_LIST = params['TRAINING_COLUMNS']
RANDOM_STATE = params['RANDOM_STATE']
HYPERPARAMS = params['HYPERPARAMS']
HYPERPARAMS_RANGES = params['HYPERPARAMS_RANGES']
##################################################################

# split matter/antimatter
SPLIT_LIST = ['all']
if SPLIT:
    SPLIT_LIST = ['antimatter', 'matter']

raw_yields_file = ROOT.TFile('SignalExtraction-data-extend-3.root')
# score_eff_dict = pickle.load(open('second_round/file_score_eff_dict','rb'))
eff_array = np.arange(0.10, MAX_EFF, 0.01)
f = ROOT.TFile("ratio_cutCompetingMass-3.root","recreate")
cut_val_cent = [0.5,0.5,0.5,0.5,0.5]

# for i_cent_bins in range(len(CENTRALITY_LIST)):
#     h = []
#     h.append(ROOT.TH1D(f"h_antimatter",f"h_antimatter",len(CT_BINS_CENT[i_cent_bins])-1,np.asarray(CT_BINS_CENT[i_cent_bins],dtype="float")))
#     h.append(ROOT.TH1D(f"h_matter",f"h_matter",len(CT_BINS_CENT[i_cent_bins])-1,np.asarray(CT_BINS_CENT[i_cent_bins],dtype="float")))

#     for i_split, split in enumerate(SPLIT_LIST):
#         split_ineq_sign = '> -0.1'
#         if SPLIT:
#             split_ineq_sign = '> 0.5'
#             if split == 'antimatter':
#                 split_ineq_sign = '< 0.5'

#         cent_bins = CENTRALITY_LIST[i_cent_bins]
#         h_pres_eff = presel_eff_file.Get(f"fPreselEff_vs_ct_{split}_{cent_bins[0]}_{cent_bins[1]};2")
#         for ct_bins in zip(CT_BINS_CENT[i_cent_bins][:-1], CT_BINS_CENT[i_cent_bins][1:]):
#             if ct_bins[0] < 5 or ct_bins[1] > 40:
#                 continue
#             h_raw = raw_yields_file.Get(f"{split}_{cent_bins[0]}_{cent_bins[1]}_{ct_bins[0]}_{ct_bins[1]}_pol/fRawYields_pol")
#             if not h_raw:
#                 continue
#             bin = f'{split}_{cent_bins[0]}_{cent_bins[1]}_{ct_bins[0]}_{ct_bins[1]}'
#             print(bin)

#             delta_t = ct_bins[1]-ct_bins[0]
#             raw_yield = h_raw.GetBinContent(h_raw.FindBin(0.9))
#             raw_yield_error = h_raw.GetBinError(h_raw.FindBin(0.9))
#             j = 0
#             i = np.power(-1,j)*0.01*int(j/2)
#             while raw_yield < 1.e-6 and (0.9+i < 0.9) and (0.9+i > 75):
#                 i = np.power(-1,j)*0.01*int(j/2)
#                 j = j + 1
#                 raw_yield = h_raw.GetBinContent(h_raw.FindBin(0.9+i))
#                 raw_yield_error = h_raw.GetBinError(h_raw.FindBin(0.9+i))
#             h[i_split].SetBinContent(h[i_split].FindBin(0.5*(ct_bins[0]+ct_bins[1])),raw_yield/delta_t/h_pres_eff.GetBinContent(h[i_split].FindBin(0.5*(ct_bins[0]+ct_bins[1]))))
#             h[i_split].SetBinError(h[i_split].FindBin(0.5*(ct_bins[0]+ct_bins[1])),raw_yield_error/delta_t/h_pres_eff.GetBinContent(h[i_split].FindBin(0.5*(ct_bins[0]+ct_bins[1]))))

#     h_ratio = ROOT.TH1D(f"h_ratio_{cent_bins[0]}_{cent_bins[1]}",f"h_ratio_{cent_bins[0]}_{cent_bins[1]}",len(CT_BINS_CENT[i_cent_bins])-1,np.asarray(CT_BINS_CENT[i_cent_bins],dtype="float"))
#     h_ratio.Divide(h[0],h[1],1,1)
#     h_ratio.GetXaxis().SetTitle("#it{c}t (cm)")
#     h_ratio.GetYaxis().SetTitle("Ratio #bar{#Lambda}/#Lambda")
#     h_ratio.Fit("pol0")
#     h_ratio.Write()

cut_dict = pickle.load(open('file_eff_cut_dict','rb'))

for i_cent_bins in range(len(CENTRALITY_LIST)):

    h = []
    h.append(ROOT.TH1D(f"h_antimatter",f"h_antimatter",len(CT_BINS_CENT[i_cent_bins])-1,np.asarray(CT_BINS_CENT[i_cent_bins],dtype="float")))
    h.append(ROOT.TH1D(f"h_matter",f"h_matter",len(CT_BINS_CENT[i_cent_bins])-1,np.asarray(CT_BINS_CENT[i_cent_bins],dtype="float")))

    cent_bins = CENTRALITY_LIST[i_cent_bins]
    presel_eff_file = ROOT.TFile(f'PreselEff_{cent_bins[0]}_{cent_bins[1]}_fineCt.root')
    h_pres_eff = [ROOT.TH1D(),ROOT.TH1D()]
    h_pres_eff[0] = presel_eff_file.Get(f"fPreselEff_vs_ct_antimatter_{cent_bins[0]}_{cent_bins[1]};2")
    h_pres_eff[1] = presel_eff_file.Get(f"fPreselEff_vs_ct_matter_{cent_bins[0]}_{cent_bins[1]};2")
    #h_pres_eff_a = presel_eff_file.Get(f"fPreselEff_vs_ct_antimatter_{cent_bins[0]}_{cent_bins[1]};2")
    #h_pres_eff_m = presel_eff_file.Get(f"fPreselEff_vs_ct_matter_{cent_bins[0]}_{cent_bins[1]};2")
    for ct_bins in zip(CT_BINS_CENT[i_cent_bins][:-1], CT_BINS_CENT[i_cent_bins][1:]):
        if ct_bins[0] < 1 or ct_bins[1] > 40:
            continue
        h_raw = [ROOT.TH1D(),ROOT.TH1D()]
        # h_raw_a = raw_yields_file.Get(f"antimatter_{cent_bins[0]}_{cent_bins[1]}_{ct_bins[0]}_{ct_bins[1]}_pol1/fRawYields")
        # h_raw_m = raw_yields_file.Get(f"matter_{cent_bins[0]}_{cent_bins[1]}_{ct_bins[0]}_{ct_bins[1]}_pol1/fRawYields")
        h_raw[0] = raw_yields_file.Get(f"antimatter_{cent_bins[0]}_{cent_bins[1]}_{ct_bins[0]}_{ct_bins[1]}_pol1/fRawYields")
        h_raw[1] = raw_yields_file.Get(f"matter_{cent_bins[0]}_{cent_bins[1]}_{ct_bins[0]}_{ct_bins[1]}_pol1/fRawYields")
        #if not h_raw_a or not h_raw_m:
        #    continue
        if not h_raw[0] or not h_raw[1]:
            continue
        bin = f'all_{cent_bins[0]}_{cent_bins[1]}_{ct_bins[0]}_{ct_bins[1]}'
        #print(bin)

        for i_split_ in enumerate(SPLIT_LIST):
            cut_bin=f'{i_split_[1]}_{cent_bins[0]}_{cent_bins[1]}_{ct_bins[0]}_{ct_bins[1]}'
            #cut_val=cut_dict[cut_bin]
            cut_val = cut_val_cent[i_cent_bins]
            i_split = int(i_split_[0])
            delta_t = ct_bins[1]-ct_bins[0]
            raw_yield = h_raw[i_split].GetBinContent(h_raw[i_split].FindBin(cut_val))/cut_val
            raw_yield_error = h_raw[i_split].GetBinError(h_raw[i_split].FindBin(cut_val))/cut_val
            j = 0
            i = np.power(-1,j)*0.01*int(j/2)
            while (raw_yield < 1.e-6 or (raw_yield_error/raw_yield > 0.03 and i_cent_bins<4)) and (cut_val+i < (cut_val+0.05)) and (cut_val+i > (cut_val-0.05)):
                i = np.power(-1,j)*0.01*int(j/2)
                j = j + 1
                raw_yield = h_raw[i_split].GetBinContent(h_raw[i_split].FindBin(cut_val+i))/(cut_val+i)
                raw_yield_error = h_raw[i_split].GetBinError(h_raw[i_split].FindBin(cut_val+i))/(cut_val+i)
                print(f'check other eff... (i_split={i_split}, i={i})')
            if raw_yield < 1.e-6:
                continue
            h[i_split].SetBinContent(h[i_split].FindBin(0.5*(ct_bins[0]+ct_bins[1])),raw_yield/delta_t/h_pres_eff[i_split].GetBinContent(h[i_split].FindBin(0.5*(ct_bins[0]+ct_bins[1]))))
            h[i_split].SetBinError(h[i_split].FindBin(0.5*(ct_bins[0]+ct_bins[1])),np.sqrt(((h_raw[i_split].GetBinError(h_raw[i_split].FindBin(cut_val+i)))**2)/((h_raw[i_split].GetBinContent(h_raw[i_split].FindBin(cut_val+i)))**2)+2*((h_pres_eff[i_split].GetBinError(h[i_split].FindBin(0.5*(ct_bins[0]+ct_bins[1]))))**2)/((h_pres_eff[i_split].GetBinContent(h[i_split].FindBin(0.5*(ct_bins[0]+ct_bins[1]))))**2))*raw_yield/delta_t/h_pres_eff[i_split].GetBinContent(h[i_split].FindBin(0.5*(ct_bins[0]+ct_bins[1]))))

    h_ratio = ROOT.TH1D(f"h_ratio_{cent_bins[0]}_{cent_bins[1]}",f"h_ratio_{cent_bins[0]}_{cent_bins[1]}",len(CT_BINS_CENT[i_cent_bins])-1,np.asarray(CT_BINS_CENT[i_cent_bins],dtype="float"))
    h_ratio.GetXaxis().SetTitle("#it{c}t (cm)")
    h_ratio.GetYaxis().SetTitle("#bar{#Omega}^{+} / #Omega^{-}")
    h_ratio.Divide(h[0],h[1],1,1)
    h_ratio.Fit("pol0")
    f.cd()
    h_ratio.Write()

    # plot ratios
    c = ROOT.TCanvas("c", "c")
    c.SetTicks(1, 1)
    c.cd()
    ROOT.gStyle.SetOptStat(0)
    ROOT.gStyle.SetOptFit(0)
    h_ratio.GetYaxis().SetRangeUser(0.8,1.2)
    h_ratio.GetXaxis().SetRangeUser(1,10)
    h_ratio.SetLineColor(centrality_colors[i_cent_bins])
    h_ratio.SetMarkerColor(centrality_colors[i_cent_bins])
    h_ratio.Draw()
    formatted_ratio = "{:.4f}".format(h_ratio.GetFunction("pol0").GetParameter(0))
    formatted_ratio_error = "{:.4f}".format(h_ratio.GetFunction("pol0").GetParError(0))
    text_x_position = 5
    ratio_text = ROOT.TLatex(text_x_position, 1.12, f"R = {formatted_ratio} #pm {formatted_ratio_error}")
    ratio_text.SetTextFont(44)
    ratio_text.SetTextSize(28)
    ratio_text.Draw("same")
    formatted_chi2 = "{:.2f}".format(h_ratio.GetFunction("pol0").GetChisquare())
    chi2_text = ROOT.TLatex(text_x_position, 1.15, "#chi^{2}/NDF = "+formatted_chi2+"/"+str(h_ratio.GetFunction("pol0").GetNDF()))
    chi2_text.SetTextFont(44)
    chi2_text.SetTextSize(28)
    chi2_text.Draw("same")
    c.Print(f"plots/{h_ratio.GetName()}.pdf")

    # systematics
    # continue
    h_sys = ROOT.TH1D(f"h_sys",f"h_sys",400,0.95,1.05)
    i=0
    while i < N_TRIALS:
        h_tmp = []
        h_tmp.append(ROOT.TH1D("h_tmp_a","h_tmp_a",len(CT_BINS_CENT[i_cent_bins])-1,np.asarray(CT_BINS_CENT[i_cent_bins],dtype="float")))
        h_tmp.append(ROOT.TH1D("h_tmp_m","h_tmp_m",len(CT_BINS_CENT[i_cent_bins])-1,np.asarray(CT_BINS_CENT[i_cent_bins],dtype="float")))
        zero_entries=[0,0]
        for ct_bins in zip(CT_BINS_CENT[i_cent_bins][:-1], CT_BINS_CENT[i_cent_bins][1:]):
            if ct_bins[0] < 1 or ct_bins[1] > 10:
                continue
            bkg = 'pol1'
            bkg_index = int(ROOT.gRandom.Rndm())
            if bkg_index < 0.5:
                bbkg = 'expo'
            split_ineq_sign = '> -0.1'
            cut_rndm_ = int(ROOT.gRandom.Rndm()*20)*0.01
            if cent_bins[0]>9:
                cut_rndm_ = int(ROOT.gRandom.Rndm()*30)*0.01
            for i_split, split in enumerate(SPLIT_LIST):
                if SPLIT:
                    split_ineq_sign = '> 0.5'
                    if split == 'antimatter':
                        split_ineq_sign = '< 0.5'
                cut_bin = f'{split}_{cent_bins[0]}_{cent_bins[1]}_{ct_bins[0]}_{ct_bins[1]}'
                cut_val = cut_val_cent[i_cent_bins]-0.09
                if cent_bins[0]==30:
                    cut_val = cut_val_cent[i_cent_bins]-0.14
                cut_rndm = cut_rndm_ + cut_val
                cent_bins = CENTRALITY_LIST[i_cent_bins]
                h_pres_eff = presel_eff_file.Get(f"fPreselEff_vs_ct_{split}_{cent_bins[0]}_{cent_bins[1]};2")
                h_raw = raw_yields_file.Get(f"{split}_{cent_bins[0]}_{cent_bins[1]}_{ct_bins[0]}_{ct_bins[1]}_{bkg}/fRawYields")
                if not h_raw:
                    continue
                bin = f'{split}_{cent_bins[0]}_{cent_bins[1]}_{ct_bins[0]}_{ct_bins[1]}_{cut_rndm}'
                print(bin)

                h_tmp[i_split].SetBinContent(h_tmp[i_split].FindBin(0.5*(ct_bins[0]+ct_bins[1])),h_raw.GetBinContent(h_raw.FindBin(cut_rndm+0.0001))/h_pres_eff.GetBinContent(h_tmp[i_split].FindBin(0.5*(ct_bins[0]+ct_bins[1])))/h_raw.GetXaxis().GetBinLowEdge(h_raw.FindBin(cut_rndm+0.0001)))
                if h_tmp[i_split].GetBinContent(h_tmp[i_split].FindBin(0.5*(ct_bins[0]+ct_bins[1])))<1.e-6:
                    zero_entries[i_split]=zero_entries[i_split]+1
                    continue
                h_tmp[i_split].SetBinError(h_tmp[i_split].FindBin(0.5*(ct_bins[0]+ct_bins[1])),np.sqrt((h_raw.GetBinError(h_raw.FindBin(cut_rndm+0.0001)))**2/((h_raw.GetBinContent(h_raw.FindBin(cut_rndm+0.0001)))**2)+2*(h_pres_eff.GetBinError(h_tmp[i_split].FindBin(0.5*(ct_bins[0]+ct_bins[1]))))**2/((h_pres_eff.GetBinContent(h_tmp[i_split].FindBin(0.5*(ct_bins[0]+ct_bins[1]))))**2))*h_raw.GetBinContent(h_raw.FindBin(cut_rndm+0.0001))/h_raw.GetXaxis().GetBinLowEdge(h_raw.FindBin(cut_rndm+0.0001))/h_pres_eff.GetBinContent(h_tmp[i_split].FindBin(0.5*(ct_bins[0]+ct_bins[1]))))
                
        print(f'zero_entries = {zero_entries}')
        if zero_entries[0] ==11 or zero_entries[1]>0:
            continue
        h_tmp_ratio = ROOT.TH1D("h_tmp_ratio","h_tmp_ratio",len(CT_BINS_CENT[i_cent_bins])-1,np.asarray(CT_BINS_CENT[i_cent_bins],dtype="float"))
        h_tmp_ratio.Divide(h_tmp[0],h_tmp[1])
        h_tmp_ratio.Fit("pol0")
        if h_tmp_ratio.GetFunction("pol0").GetNDF()<9.5:
            continue
        if h_tmp_ratio.GetFunction("pol0").GetProb()>0.975 or h_tmp_ratio.GetFunction("pol0").GetProb()<0.025:
            continue
        i = i+1
        # if h_tmp_ratio.GetFunction("pol0").GetParameter(0) > 0.99 and (i%100)==0:
        #     h_tmp_ratio.Write()
        h_sys.Fill(h_tmp_ratio.GetFunction("pol0").GetParameter(0))

    h_sys.GetXaxis().SetTitle("p_{0}")
    h_sys.SetTitle(f"{cent_bins[0]}-{cent_bins[1]}%")
    h_sys.GetXaxis().SetRangeUser(0.8, 1.2)
    h_sys.GetYaxis().SetTitle("Entries")
    h_sys.GetXaxis().SetTitle("R (#bar{#Omega}^{+} / #Omega^{-})")
    h_sys.SetDrawOption("histo")
    h_sys.SetLineWidth(2)
    h_sys.SetFillStyle(3345)
    h_sys.SetFillColor(ROOT.kBlue)
    h_sys.Write()

    # plot systematics distribution
    c = ROOT.TCanvas("c", "c")
    ROOT.gStyle.SetOptStat(110001110)
    c.cd()
    c.SetTicks(1, 1)
    mean = h_sys.GetMean()
    std_dev = h_sys.GetRMS()
    h_sys.Rebin(2)
    h_sys.GetXaxis().SetRangeUser(mean-5*std_dev, mean+5*std_dev)
    h_sys.Draw("histo")
    c.Print(f"plots/{h_sys.GetName()}_{cent_bins[0]}_{cent_bins[1]}.pdf")

    f.cd()
    h_sys.Write()