# LaunchAnalysis.sh

# parameters
cutSettings=""
binCountingFlag=0
expFlag=1 # 1->sum of 2 exp, 0 -> sum of exp and pol
sigmoidFlag=0
spectraHistNameId="_1_1_1"
extractRatios=1

fileData="AnalysisResults_2015"
fileMC="mc_old_G4"
signalName="SignalDeuteron_2015G4_AntideuteronsAsPrimaries"
spectraName="SpectraDeuteron_2015G4_AntideuteronsAsPrimaries"
EfficiencyHe3="EfficiencyDeuteron_2015G4_AntideuteronsAsPrimaries"
PrimaryHe3="PrimaryDeuteron_2015G4_AntideuteronsAsPrimaries"

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
.L Efficiency.cpp+
.L Secondary.cpp+
.L Spectra.cpp+
SignalBinned("$cutSettings",$argumentSignal,"$fileData","$signalName","recreate")
Efficiency("$cutSettings","$fileMC","$EfficiencyHe3")
Secondary("$cutSettings","$fileData","$fileMC","$PrimaryHe3")
Spectra("$cutSettings",$argumentSignal,$sigmoidFlag,"$spectraHistNameId","$spectraName","recreate","AnalysisResults","$signalName","$EfficiencyHe3","$PrimaryHe3")
.q
EOF
fi
