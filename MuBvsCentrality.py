import ROOT
import numpy as np

staggering = False

def InvertPoints(g,a,b):
    buffer = []
    buffer.append(g.GetPointX(a))
    buffer.append(g.GetErrorX(a))
    buffer.append(g.GetPointY(a))
    buffer.append(g.GetErrorY(a))
    g.SetPoint(a,g.GetPointX(b),g.GetPointY(b))
    g.SetPointError(a,g.GetErrorX(b),g.GetErrorY(b))
    g.SetPoint(b,buffer[0],buffer[2])
    g.SetPointError(b,buffer[1],buffer[3])
    return

def is_pos_def(x):
    return np.all(np.linalg.eigvals(x) > 0)

def chi2(par,g,gCorr,gB,print_flag):
    val = []
    uncorr = []
    corr = []
    polarity = []
    cov_m = []
    par_vec = []
    for i_c in range(5):
        val.append(g.GetPointY(i_c))
        uncorr.append(np.abs(g.GetErrorY(i_c)))
        corr.append(np.abs(gCorr.GetErrorY(i_c)))
        polarity.append(np.abs(gB.GetErrorY(i_c)))
        par_vec.append(par)
    polarity_min = min(polarity)
    corr_min = min(corr)
    if print_flag:
        #print(corr)
        pass
    for i in range(5):
        cov_m.append([(polarity_min**2 + corr[i]**2) for _ in range(5)])
        for j in range(5):
            if j == i:
                continue
            cov_m[i][j] = corr[i]*corr[j] + polarity[i]*polarity[j]
        cov_m[i][i] = uncorr[i]**2 + corr[i]**2 + polarity[i]**2
    cov_m = np.array(cov_m)
    val = np.array(val)
    par_vec = np.array(par_vec)
    cov_m_inv = np.linalg.inv(cov_m)
    diff_val_par = val-par_vec
    diff_val_par_T = np.matrix.transpose(diff_val_par)
    tmp = np.matmul(cov_m_inv,diff_val_par)
    chisq = np.matmul(diff_val_par_T,tmp)
    if print_flag:
        print(cov_m)
        print(diff_val_par)
    return chisq

def covM(g,gCorr,gB,print_flag):
    val = []
    uncorr = []
    corr = []
    polarity = []
    cov_m = []
    par_vec = []
    for i_c in range(5):
        val.append(g.GetPointY(i_c))
        uncorr.append(np.abs(g.GetErrorY(i_c)))
        corr.append(np.abs(gCorr.GetErrorY(i_c)))
        polarity.append(np.abs(gB.GetErrorY(i_c)))
    polarity_min = min(polarity)
    corr_min = min(corr)
    if print_flag:
        #print(corr)
        pass
    for i in range(5):
        cov_m.append([(polarity_min**2 + corr[i]**2) for _ in range(5)])
        for j in range(5):
            if j == i:
                continue
            cov_m[i][j] = corr[i]*corr[j] + polarity[i]*polarity[j]
        cov_m[i][i] = uncorr[i]**2 + corr[i]**2 + polarity[i]**2

    if print_flag:
        print(cov_m)
    if is_pos_def(cov_m):
        print("positive_def")
    return cov_m

ROOT.gStyle.SetOptStat(0)
cent = [[0,5],[5,10],[10,30],[30,50],[50,90]]

f_names = ["FinalPlot3D_new_2.root","FinalPlot3D_new_2fixmuQ_nopions.root"]
file = [ROOT.TFile(f) for f in f_names]
cvs = [f.Get("cMuBCent") for f in file]
g = [c.FindObject("MuBCent") for c in cvs]
gB = [c.FindObject("gMuBCentPolarityUnc") for c in cvs]
gCorr = [c.FindObject("gMuBCentCorrUnc") for c in cvs]
gCorrPlot = [ROOT.TGraphErrors(g) for g in gCorr]
n = cvs[0].FindObject("NaturePoint")

c = ROOT.TCanvas()
c.cd()
c.SetRightMargin(0.02)
c.SetTopMargin(0.03)
frame = ROOT.TH2D("frame",";Centrality (%);#mu_{#it{B}} (MeV)",1,0,90,1,-0.1,1.3)
frame.Draw()
n.SetFillColor(ROOT.kGray+1)
#n.Draw("samee5")
#gCorr[0].Draw("samepe5")
for p in range(gCorr[1].GetN()):
    if staggering:
        gCorr[1].SetPointX(p,gCorr[1].GetPointX(p)+2)
        g[1].SetPointX(p,g[1].GetPointX(p)+2)
    for gg in g:
        gg.SetPointError(p,0.,gg.GetErrorY(p))
        gg.SetLineWidth(2)
        gg.SetMarkerSize(1.5)
    for polPlot, corrPlot in zip(gB, gCorrPlot):
        corrPlot.SetPointError(p,0.,np.sqrt(corrPlot.GetErrorY(p)**2+polPlot.GetErrorY(p)**2))
InvertPoints(g[0],2,3)
InvertPoints(g[1],2,3)
InvertPoints(gB[0],2,3)
InvertPoints(gB[1],2,3)
InvertPoints(gCorr[0],2,3)
InvertPoints(gCorr[1],2,3)
InvertPoints(gCorrPlot[0],2,3)
InvertPoints(gCorrPlot[1],2,3)
gCorrPlot[1].SetFillColor(ROOT.kBlue)
gCorrPlot[1].SetFillStyle(3345)
#gCorrPlot[1].Draw("samepe3")
gCorrPlot[0].SetFillColor(ROOT.kRed)
gCorrPlot[0].SetFillStyle(3354)
#gCorrPlot[0].Draw("samepe3")
# gB[0].SetFillColor(ROOT.kRed-10)
# gB[1].SetFillColor(ROOT.kBlue-10)
# gB[1].Draw("e3same")
# gB[0].Draw("e3same")
g[0].Draw("pesame")
#InvertPoints(gB[1],2,3)
g[1].SetLineColor(ROOT.kBlue)
g[1].SetMarkerColor(ROOT.kBlue)
g[1].Draw("samepe")

print(f"chisq = {chi2(0.8,g[1],gCorr[1],gB[1],True)}")

cc = ROOT.TCanvas()
cc.cd()
h = ROOT.TH1D("h",";#it{#mu}_{B} (MeV);#chi^{2}",1000,-5.,5.)
for i_p in range(1000):
    h.SetBinContent(i_p+1,chi2(i_p*0.01 - 5.,g[1],gCorr[1],gB[1],False))

h2 = ROOT.TH1D("h2",";#it{#mu}_{B} (MeV);#chi^{2}",1000,-5.,5.)
for i_p in range(1000):
    h2.SetBinContent(i_p+1,chi2(i_p*0.01 - 5.,g[0],gCorr[0],gB[0],False))


c.cd()
l = ROOT.TLegend(0.26817,0.175652,0.452381,0.295217)
l.SetTextFont(44)
l.SetTextSize(22)
l.AddEntry(g[0],"Thermal-FIST, #it{T}_{ch}=155 MeV, #mu_{#it{S}} constrained","pe")
l.AddEntry(g[1],"Thermal-FIST, #it{T}_{ch}=155 MeV, #mu_{#it{S}} and #mu_{#it{Q}} constrained","pe")
#l.AddEntry(n,"SHM fit, #it{Nature} #bf{561}, 321-330 (2018)")
l.Draw("same")

t = ROOT.TLatex()
t.SetTextFont(44)
t.SetTextSize(40)
t.DrawLatex(40.,1.12,"ALICE")
t.SetTextSize(35)
t.DrawLatex(40.,0.95,"Pb-Pb #sqrt{#it{s}_{NN}}=5.02 TeV")
t.SetTextSize(25)
t.DrawLatex(20.,0.25,"#pm0.15 MeV corr. sys. not shown")
#t.DrawLatex(17.,-1.,"0.5 MeV correlated uncertainty not shown")

o = ROOT.TFile("muBvsCent_1.root","recreate")
cc.cd()
h.Fit("pol2")
h2.Fit("pol2")
c.Print("MuBvsCent.pdf")

p0 = h.GetFunction("pol2").GetParameter(0)
p1 = h.GetFunction("pol2").GetParameter(1)
p2 = h.GetFunction("pol2").GetParameter(2)
val = -p1/2./p2
val_err = np.sqrt(p1**2-4*p2*(p1**2/4./p2-1))/2./p2
format_val = "{:.4f}".format(val)
format_val_err = "{:.4f}".format(val_err)
format_chi2 = "{:.2f}".format(h.GetFunction("pol2").Eval(val))
print(f"muB = {format_val} +/- {format_val_err} MeV, chi2 = {format_chi2}/4")

p0 = h2.GetFunction("pol2").GetParameter(0)
p1 = h2.GetFunction("pol2").GetParameter(1)
p2 = h2.GetFunction("pol2").GetParameter(2)
val = -p1/2./p2
val_err = np.sqrt(p1**2-4*p2*(p1**2/4./p2-1))/2./p2
format_val = "{:.4f}".format(val)
format_val_err = "{:.4f}".format(val_err)
format_chi2 = "{:.2f}".format(h2.GetFunction("pol2").Eval(val))
print(f"muB = {format_val} +/- {format_val_err} MeV, chi2 = {format_chi2}/4")

cchi2 = ROOT.TCanvas("cchi2_muB", "cchi2_muB", 500, 500)
cchi2.SetTopMargin(0.02)
cchi2.SetRightMargin(0.02)
h2.GetXaxis().SetRangeUser(val - 4 * val_err, val + 4 * val_err)
# h.GetXaxis().SetNdivisions(210)
# h.GetYaxis().SetNdivisions(210)
cchi2.cd()
h2.SetLineWidth(2)
h2.SetLineColor(ROOT.kAzure - 3)
h2.Draw("l")
h2.GetYaxis().SetRangeUser(h2.GetMinimum() - h2.GetMaximum() * 0.05, h2.GetMaximum() * 1.05)
text = ROOT.TLatex()
text.SetTextFont(44)
text.SetTextSize(25)
text.SetNDC()
text.DrawLatex(0.32, 0.9, "ALICE")
text.DrawLatex(0.32, 0.82, "Pb#minusPb #sqrt{#it{s}_{NN}} = 5.02 TeV")
format_val = "{:.2f}".format(val)
format_val_err = "{:.2f}".format(val_err)
text.DrawLatex(0.32, 0.74, "#it{#mu}_{B} = "+format_val+" #pm "+format_val_err+" MeV")
cchi2.GetPrimitive("pol2").Delete()
cchi2.Update()
lines = [ROOT.TLine() for _ in range(6)]
nsigma = -1
for ln in lines:
    ln.SetLineWidth(1)
    ln.SetLineStyle(9)
    ln.SetLineColor(ROOT.kOrange + 8)
    ln.SetX1(val + nsigma * val_err)
    ln.SetX2(val + nsigma * val_err)
    ln.SetY1(h2.GetMinimum())
    ln.SetY2(h2.GetBinContent(h2.FindBin(val + np.abs(nsigma) * val_err)))
    nsigma = nsigma * (-1)
    if nsigma < 0:
        nsigma = nsigma - 1
    ln.Draw("same")
lines_2 = [ROOT.TLine() for _ in range(3)]
nsigma = -1
text.SetNDC(False)
for ln in lines_2:
    ln.SetLineWidth(1)
    ln.SetLineStyle(9)
    ln.SetLineColor(ROOT.kOrange + 8)
    ln.SetX1(val + nsigma * val_err)
    ln.SetX2(val - nsigma * val_err)
    ln.SetY1(h2.GetBinContent(h2.FindBin(val - nsigma * val_err)))
    ln.SetY2(h2.GetBinContent(h2.FindBin(val - nsigma * val_err)))
    print("{:.0f}".format(np.abs(nsigma))+"sigma C.I. -> "+f"[{val + nsigma * val_err}, {val - nsigma * val_err}]")
    ln.Draw("same")
    text.DrawLatex(val - 0.6 * val_err, h2.GetBinContent(h2.FindBin(val - nsigma * val_err)) + h2.GetBinContent(h2.GetMaximumBin()) * 0.03, "{:.0f}".format(np.abs(nsigma))+"#sigma C.I.")
    nsigma = nsigma - 1
cchi2.Write()
cchi2.Print("muBChi2.pdf")

# plot covariance matrices
hh = [ROOT.TH2D(f"covM_{i}", f" ", 5, 0, 5, 5, 0, 5) for i in range(2)]
cc = [ROOT.TCanvas(f"c_{i}", f"c_{i}", 500, 500) for i in range(2)]
for iG in range(2):
    cov = covM(g[iG],gCorr[iG],gB[iG],False)
    lab = ['0-5%', '5-10%', '10-30%', '30-50%', '50-90%']
    ROOT.gStyle.SetPalette(ROOT.kLightTemperature)
    ROOT.gStyle.SetPaintTextFormat(".2f")
    hh[iG].SetMarkerSize(2)
    ROOT.gStyle.SetTextFont(42)
    hh[iG].GetZaxis().SetRangeUser(0.2,0.3)
    hh[iG].GetXaxis().SetLabelFont(44)
    hh[iG].GetXaxis().SetLabelSize(25)
    hh[iG].GetXaxis().SetLabelOffset(-0.35)
    hh[iG].GetYaxis().SetLabelFont(44)
    hh[iG].GetYaxis().SetLabelSize(25)
    for i in range(5):
        hh[iG].GetXaxis().SetBinLabel(i + 1, lab[i])
        hh[iG].GetYaxis().SetBinLabel(5 - i, lab[i])
        for j in range(5):
            if j > i:
                continue
            hh[iG].SetBinContent(i + 1, 5 - j, cov[i][j])
    cc[iG].cd()
    hh[iG].GetXaxis().SetNdivisions(5)
    hh[iG].GetYaxis().SetNdivisions(5)
    cc[iG].SetRightMargin(0.02)
    cc[iG].SetLeftMargin(0.13)
    cc[iG].SetTopMargin(0.08)
    cc[iG].SetBottomMargin(0.02)
    hh[iG].Draw("col text")
    cc[iG].Update()
    tt = ROOT.TLatex()
    tt.SetTextFont(44)
    tt.SetTextSize(29)
    tt.DrawLatex(0.3, 1.5, "ALICE")
    tt.SetTextSize(19)
    tt.DrawLatex(0.3, 1.1, "Pb#minusPb #sqrt{#it{s}_{NN}} = 5.02 TeV")
    tt.SetTextSize(19)
    tt.DrawLatex(0.3, .7, "Thermal-FIST, #it{#mu}_{B} covariance (MeV)")
    if iG == 0:
        tt.DrawLatex(0.3, .3, "#it{T}_{ch} = 155 #pm 2 MeV, #it{#mu}_{S} constrained")
    else:
        tt.DrawLatex(0.3, .3, "#it{T}_{ch} = 155 #pm 2 MeV, #it{#mu}_{S} and #it{#mu}_{Q} constrained")
    # palette = hh[iG].GetListOfFunctions().FindObject("palette")
    # cc[iG].Modified()
    # cc[iG].Update()
    cc[iG].Write()
    hh[iG].Write()
    cc[iG].Print(f'covM_{iG}.pdf')

h.Write()
h2.Write()
c.Write()