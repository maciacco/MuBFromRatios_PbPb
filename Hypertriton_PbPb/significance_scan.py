#!/usr/bin/env python3
import os
import pickle
import warnings

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from scipy.stats import norm
import ROOT
import uproot
import yaml
from helpers import significance_error, expected_signal

SPLIT = True

# avoid pandas warning
warnings.simplefilter(action='ignore', category=FutureWarning)
ROOT.gROOT.SetBatch()

##################################################################
# read configuration file
##################################################################
config = 'config.yaml'
with open(os.path.expandvars(config), 'r') as stream:
    try:
        params = yaml.full_load(stream)
    except yaml.YAMLError as exc:
        print(exc)

ANALYSIS_RESULTS_PATH = params['ANALYSIS_RESULTS_PATH']
CT_BINS = params['CT_BINS']
CENTRALITY_LIST = params['CENTRALITY_LIST']
RANDOM_STATE = params['RANDOM_STATE']
##################################################################

# split matter/antimatter
SPLIT_LIST = ['']
if SPLIT:
    SPLIT_LIST = ['antimatter', 'matter']

score_eff_arrays_dict = pickle.load(open("file_score_eff_dict", "rb"))
eff_array = np.arange(0.10, 0.91, 0.01)
presel_eff_file = uproot.open('PreselEff.root')
analysis_results_file = uproot.open(os.path.expandvars(ANALYSIS_RESULTS_PATH))

# get centrality selected histogram
cent_counts, cent_edges = analysis_results_file['Centrality_selected;1'].to_numpy()
cent_bin_centers = (cent_edges[:-1]+cent_edges[1:])/2

# cut dictionary
eff_cut_dict = dict()

for split in SPLIT_LIST:
    for i_cent_bins in range(len(CENTRALITY_LIST)):
        cent_bins = CENTRALITY_LIST[i_cent_bins]

        # get number of events
        cent_range_map = np.logical_and(cent_bin_centers > cent_bins[0], cent_bin_centers < cent_bins[1])
        counts_cent_range = cent_counts[cent_range_map]
        evts = np.sum(counts_cent_range)
        print(f'Number of events: {evts}')

        # get preselection efficiency histogram
        presel_eff_counts, presel_eff_edges = presel_eff_file[f'fPreselEff_vs_ct_{split}_{cent_bins[0]}_{cent_bins[1]};1'].to_numpy(
        )
        presel_eff_bin_centers = (presel_eff_edges[1:]+presel_eff_edges[:-1])/2

        for ct_bins in zip(CT_BINS[i_cent_bins][:-1], CT_BINS[i_cent_bins][1:]):

            bin = f'{split}_{cent_bins[0]}_{cent_bins[1]}_{ct_bins[0]}_{ct_bins[1]}'
            df_data = pd.read_parquet(f'df/{bin}')

            # plot directory
            if not os.path.isdir('plots/significance_scan'):
                os.mkdir('plots/significance_scan')

            # lists filled inside loop
            significance_list = []
            significance_err_list = []

            for eff_score in zip(eff_array, score_eff_arrays_dict[bin]):
                if (ct_bins[0] > 0) and (eff_score[0] < 0.49):
                    continue
                formatted_eff = "{:.2f}".format(eff_score[0])
                print(f'processing {bin}: eff = {eff_score[0]:.2f}, score = {eff_score[1]:.2f}...')

                # select data in the sidebands
                df_data_sel = df_data.query(f'model_output > {eff_score[1]}')

                # make histogram
                hyp_mass = 2.991
                sigma = 0.0015
                m_min = hyp_mass - 3*sigma
                m_max = hyp_mass + 3*sigma
                nBins = 32

                counts, bin_edges = np.histogram(np.array(df_data_sel['m']), nBins, range=(2.96, 3.04))
                bin_centers = (bin_edges[1:]+bin_edges[:-1])/2.
                side_map = np.logical_or(bin_centers < m_min, bin_centers > m_max)
                mass_map = np.logical_not(side_map)

                side_bins = bin_centers[side_map]
                side_counts = counts[side_map]
                side_errors = np.sqrt(side_counts)

                # polynomial fit to background
                pol = np.polynomial.Polynomial.fit(side_bins, side_counts, deg=2)
                xx_side, yy_side = pol.linspace()  # plot polynomial

                # compute background
                pol_integral = pol.integ()
                bin_size = (3.04-2.96)/nBins
                bkg = (pol_integral(m_max) - pol_integral(m_min))/bin_size

                # compute eff = presel_eff * BDT_eff
                presel_eff_map = np.logical_and(
                    presel_eff_bin_centers > ct_bins[0],
                    presel_eff_bin_centers < ct_bins[1])
                presel_eff = presel_eff_counts[presel_eff_map]
                eff = presel_eff * eff_score[0]

                # compute expected signal
                sig = expected_signal(cent_bins, ct_bins, eff, evts)[0]
                mass_bins = bin_centers[mass_map]
                mass_counts = norm.pdf(mass_bins, hyp_mass, sigma)
                mass_counts = mass_counts*sig*bin_size
                pol_offset = pol(mass_bins)
                mass_counts = mass_counts+pol_offset

                xx_mass = np.linspace(norm.ppf(0.005, hyp_mass, sigma), norm.ppf(0.995, hyp_mass, sigma), 100)
                yy_mass = norm.pdf(xx_mass, hyp_mass, sigma)*sig*bin_size
                yy_offset = pol(xx_mass)  # plot polynomial
                yy_mass = yy_mass+yy_offset

                # compute significance
                significance_list.append(sig/np.sqrt(sig+bkg))
                significance_err_list.append(significance_error(sig, bkg))

                # plot histograms
                if not os.path.isdir(f'plots/significance_scan/{bin}'):
                    os.mkdir(f'plots/significance_scan/{bin}')
                plt.errorbar(side_bins, side_counts, side_errors, fmt='o')
                plt.errorbar(mass_bins, mass_counts, np.sqrt(mass_counts), fmt='o')
                plt.plot(xx_side, yy_side)
                plt.plot(xx_mass, yy_mass)
                plt.xlabel(r'$M \ (^{3}He + \pi^{-}) \ (\mathrm{GeV}/\it{c}^{2})$')
                plt.ylabel(r'$\mathrm{Entries}/(2.5 \mathrm{MeV}/\it{c}^{2})$')
                plt.savefig(f'plots/significance_scan/{bin}/{formatted_eff}_{bin}.png')
                plt.close('all')

            eff_array_reduced = eff_array[40:]
            significance_array = np.asarray(significance_list)
            significance_err_array = np.asarray(significance_err_list)

            significance_array = significance_array*(eff_array_reduced)
            significance_err_array = significance_err_array*(eff_array_reduced)

            low_limit = significance_array - significance_err_array
            up_limit = significance_array + significance_err_array

            # cut
            cut_eff_index = significance_array == significance_array.max()
            cut_eff = eff_array_reduced[cut_eff_index]

            fig = plt.figure()
            plt.plot(eff_array_reduced, significance_array, 'b', label='Expected Significance')
            plt.fill_between(eff_array_reduced, low_limit, up_limit,
                             facecolor='deepskyblue', label=r'$ \pm 1\sigma$', alpha=0.3)
            plt.vlines(cut_eff[0], 0, 10, colors='red', linestyles='dashed', label=f'cut = {"{:.2f}".format(cut_eff[0])}')

            handles, labels = fig.gca().get_legend_handles_labels()
            order = [0,2,1]
            plt.legend([handles[idx] for idx in order],[labels[idx] for idx in order], loc='lower left')

            plt.xlabel("BDT Efficiency")
            plt.ylabel("Significance x BDT Efficiency")
            plt.xlim(0.5,0.9)
            plt.ylim(0.3, up_limit.max()+0.3)

            plt.savefig(f'plots/significance_scan/{bin}.png')
            plt.close('all')

            eff_cut_dict[bin] = cut_eff

pickle.dump(eff_cut_dict, open("file_eff_cut_dict", "wb"))
