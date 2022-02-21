# LaunchAnalysis.sh

# parameters
cutSettings=""
binCountingFlag=1
expFlag=1 # 1->sum of 2 exp, 0 -> sum of exp and pol
sigmoidFlag=1
spectraHistNameId=""
extractRatios=1

fileData="AnalysisResults"
fileMC="AnalysisResults_LHC21l5_full_largeDCA_cutChi2"
signalName="SignalPion"
spectraName="SpectraPion"
EfficiencyHe3="EfficiencyPionMC_21l5_false_"
PrimaryHe3="PrimaryPion"

# create output directories
DIR_OUT=out
DIR_PLOT=plots

if [ -d "$DIR_OUT" ]; then
    echo "Directory '$DIR_OUT' already exists"
else
    mkdir $DIR_OUT
fi

if [ -d "$DIR_PLOT" ]; then
    echo "Directory '$DIR_PLOT' already exists"
else
    mkdir $DIR_PLOT
fi

# launch analysis

echo $cutSettings
argumentSignal="$binCountingFlag,$expFlag"

if [ $extractRatios -eq 1 ]; then
    root -b -l <<EOF
.L ../utils/RooGausExp.cxx+
.L ../utils/RooDSCBShape.cxx+
.L ../utils/RooGausDExp.cxx+
.L SignalBinned.cpp+
//.L Efficiency.cpp+
.L Secondary.cpp+
.L Spectra.cpp+
.L AbsorptionError.cpp+
SignalBinned("$cutSettings",1.5,11.,8.5,13.5,$argumentSignal,"$fileData","$signalName","recreate")
Secondary("$cutSettings",0.12,"$fileData","$fileMC","$PrimaryHe3")//,true)
Spectra("$cutSettings",1.5,11.,$argumentSignal,$sigmoidFlag,"$spectraHistNameId","$spectraName","recreate","AnalysisResults","$signalName","$EfficiencyHe3","$PrimaryHe3")//,true)
//AbsorptionError("AbsErrorMCorrection","recreate","$spectraName")
.q
EOF
fi
