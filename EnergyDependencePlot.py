import ROOT
import numpy as np

ROOT.gStyle.SetOptStat(0)

root_s_NuclPhysA772 = [2.7, 3.32, 3.84, 4.30, 4.85, 8.76, 12.3, 17.3, 130., 200.]
root_s_PhysRevC96 = [7.7, 11.5, 19.6, 27.0, 39.0, 62.4, 200.]
root_s_PhysLettB738 = [11.5, 19.6, 27., 39., 62.4, 200.]
root_s_Nature561 = [2760.]
root_s_ThisWork = [5020.]

root_s_err_NuclPhysA772 = [0., 0., 0., 0., 0., 0., 0., 0., 0., 0.]
root_s_err_PhysRevC96 = [0., 0., 0., 0., 0., 0., 0.]
root_s_err_PhysLettB738 = [0., 0., 0., 0., 0., 0.]
root_s_err_Nature561 = [0.]
root_s_err_ThisWork = [0.]

mu_b_NuclPhysA772 = [760., 670., 615., 580., 537., 403., 298., 240., 38., 20.]
mu_b_PhysRevC96 = [398.2, 287.3, 187.9, 144.4, 103.2, 69.8, 28.4]
mu_b_PhysLettB738 = [326.7, 192.5, 140.4, 99.9, 66.4, 24.3]
mu_b_Nature561 = [0.7]
mu_b_ThisWork = [0.70]

mu_b_err_NuclPhysA772 = [20., 16., 8., 11., 10., 16., 19., 18., 11., 4.]
mu_b_err_PhysRevC96 = [16.4, 12.5, 8.6, 7.2, 7.4, 5.6, 5.8]
mu_b_err_PhysLettB738 = [25.9, 3.9, 1.4, 1.4, 0.6, 0.6]
mu_b_err_Nature561 = [3.8]
mu_b_err_ThisWork = [0.45]

param = ROOT.TF1("param", "[0]/(1.+0.288*x)", 1., 10000.)
param.SetParameter(0, 1307.5)
param.SetLineColor(ROOT.kBlack)
frame = ROOT.TH2D("frame", ";#sqrt{s_{NN}} (GeV);#mu_{B} (MeV)", 1, 2., 9000., 1, 0.2, 1200.)
g1 = ROOT.TGraphErrors(10, np.array(root_s_NuclPhysA772), np.array(mu_b_NuclPhysA772), np.array(root_s_err_NuclPhysA772), np.array(mu_b_err_NuclPhysA772))
g3 = ROOT.TGraphErrors(7, np.array(root_s_PhysRevC96), np.array(mu_b_PhysRevC96), np.array(root_s_err_PhysRevC96), np.array(mu_b_err_PhysRevC96))
g2 = ROOT.TGraphErrors(6, np.array(root_s_PhysLettB738), np.array(mu_b_PhysLettB738), np.array(root_s_err_PhysLettB738), np.array(mu_b_err_PhysLettB738))
g4 = ROOT.TGraphErrors(1, np.array(root_s_Nature561), np.array(mu_b_Nature561), np.array(root_s_err_Nature561), np.array(mu_b_err_Nature561))
g5 = ROOT.TGraphErrors(1, np.array(root_s_ThisWork), np.array(mu_b_ThisWork), np.array(root_s_err_ThisWork), np.array(mu_b_err_ThisWork))
g1.SetMarkerStyle(53)
g3.SetMarkerStyle(54)
g2.SetMarkerStyle(56)
g4.SetMarkerStyle(57)
g5.SetMarkerStyle(20)
g1.SetMarkerSize(1.)
g3.SetMarkerSize(1.)
g2.SetMarkerSize(1.)
g4.SetMarkerSize(1.)
g5.SetMarkerSize(1.)
g1.SetLineWidth(2)
g3.SetLineWidth(2)
g2.SetLineWidth(2)
g4.SetLineWidth(2)
g5.SetLineWidth(2)
g1.SetMarkerColor(ROOT.kGreen+2)
g3.SetMarkerColor(ROOT.kOrange+7)
g2.SetMarkerColor(ROOT.kAzure-3)
g4.SetMarkerColor(ROOT.kViolet+1)
g5.SetMarkerColor(ROOT.kRed)
g1.SetLineColor(ROOT.kGreen+2)
g3.SetLineColor(ROOT.kOrange+7)
g2.SetLineColor(ROOT.kAzure-3)
g4.SetLineColor(ROOT.kViolet+1)
g5.SetLineColor(ROOT.kRed)

frame2 = ROOT.TH2D("frame2", ";#sqrt{s_{NN}} (GeV);dat. - par. (MeV)", 1, 2., 9000., 1, -35., 35.)
g1_compare = ROOT.TGraphErrors(10, np.array(root_s_NuclPhysA772), np.array(mu_b_NuclPhysA772), np.array(root_s_err_NuclPhysA772), np.array(mu_b_err_NuclPhysA772))
g3_compare = ROOT.TGraphErrors(7, np.array(root_s_PhysRevC96), np.array(mu_b_PhysRevC96), np.array(root_s_err_PhysRevC96), np.array(mu_b_err_PhysRevC96))
g2_compare = ROOT.TGraphErrors(6, np.array(root_s_PhysLettB738), np.array(mu_b_PhysLettB738), np.array(root_s_err_PhysLettB738), np.array(mu_b_err_PhysLettB738))
g4_compare = ROOT.TGraphErrors(1, np.array(root_s_Nature561), np.array(mu_b_Nature561), np.array(root_s_err_Nature561), np.array(mu_b_err_Nature561))
g5_compare = ROOT.TGraphErrors(1, np.array(root_s_ThisWork), np.array(mu_b_ThisWork), np.array(root_s_err_ThisWork), np.array(mu_b_err_ThisWork))
gTot = ROOT.TGraphErrors()
for g in zip([g1_compare, g3_compare, g2_compare, g4_compare, g5_compare],[g1, g3, g2, g4, g5]):
    for i_p in range(g[0].GetN()):
        par = param.Eval(g[0].GetPointX(i_p))
        g[0].SetPointY(i_p,g[0].GetPointY(i_p)-par)
        gTot.AddPoint(g[0].GetPointX(i_p),g[0].GetPointY(i_p))
        gTot.SetPointError(gTot.GetN()-1,g[0].GetErrorX(i_p),g[0].GetErrorY(i_p))
    g[0].SetMarkerStyle(g[1].GetMarkerStyle())
    g[0].SetMarkerSize(g[1].GetMarkerSize())
    g[0].SetMarkerColor(g[1].GetMarkerColor())
    g[0].SetLineColor(g[1].GetLineColor())
    g[0].SetLineWidth(g[1].GetLineWidth())

c = ROOT.TCanvas("c", "c", 500, 450)
pad1 = ROOT.TPad("pad1", "pad1", 0.0, 0.3, 1.0, 1.0, 0)
pad1.SetFillColor(0)
pad1.SetFrameBorderMode(0)
pad1.SetBottomMargin(0.0)
pad1.SetTopMargin(0.02)
pad1.SetRightMargin(0.02)
pad1.SetLogx()
pad1.SetLogy()
pad1.Draw()
frame.GetYaxis().SetTitleFont(44)
frame.GetYaxis().SetTitleSize(14)
frame.GetYaxis().SetTitleOffset(1.5)
frame.GetXaxis().SetLabelFont(44)
frame.GetXaxis().SetLabelSize(10)
pad2 = ROOT.TPad("pad2", "pad2", 0.0, 0.0, 1.0, 0.3, 0)
pad2.SetLogx()
pad2.SetFillColor(0)
pad2.SetFrameBorderMode(0)
pad2.SetFillColor(0)
pad2.SetFrameBorderMode(0)
pad2.SetTopMargin(0.0)
pad2.SetBottomMargin(0.25)
pad2.SetRightMargin(0.02)
frame2.GetXaxis().SetTitleFont(44)
frame2.GetXaxis().SetTitleSize(17)
frame2.GetXaxis().SetTitleOffset(0.6)
frame2.GetYaxis().SetTitleFont(44)
frame2.GetYaxis().SetTitleSize(14)
frame2.GetYaxis().SetTitleOffset(1.5)
frame2.GetYaxis().CenterTitle()
frame2.GetYaxis().SetNdivisions(210)
frame2.GetYaxis().SetTickLength(0.04)
frame2.GetXaxis().SetTickLength(0.07)
frame2.GetXaxis().SetLabelFont(44)
frame2.GetXaxis().SetLabelSize(11)
frame2.GetYaxis().SetLabelFont(44)
frame2.GetYaxis().SetLabelSize(11)
pad2.SetGridy()
pad2.Draw()
pad1.cd()
l = ROOT.TLegend(0.16,0.05,0.4,0.45)
l.SetTextFont(44)
l.SetTextSize(15)
frame.Draw()
param.Draw("lsame")
g1.Draw("pesame")
g3.Draw("pesame")
g2.Draw("pesame")
g4.Draw("pesame")
g5.Draw("pesame")
l.AddEntry(g1, "Nucl. Phys. A 772, 167-199 (2006)", "pe")
l.AddEntry(g3, "Phys. Lett. B 738, 305-310 (2014)", "pe")
l.AddEntry(g2, "Phys. Rev. C 96, 044904 (2017)", "pe")
l.AddEntry(g4, "Nature 561, 321-330 (2018)", "pe")
l.AddEntry(g5, "ALICE, Pb-Pb, #sqrt{s_{NN}}=5.02 TeV", "pe")
l.AddEntry(param, "Param., Nature 561, 321-330 (2018)", "l")
l.Draw("same")
#g1.Fit(param)
pad2.cd()
frame2.Draw()
g1_compare.Draw("pesame")
g3_compare.Draw("pesame")
g2_compare.Draw("pesame")
g4_compare.Draw("pesame")
g5_compare.Draw("pesame")
#gTot.Fit("pol0")
#gTot.Draw("pesame")

c.Print("c.pdf")