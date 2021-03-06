# LaunchAnalysis.sh

# parameters
cutSettings="$1"
binCountingFlag="$2"
expFlag="$3" # 1->sum of 2 exp
sigmoidFlag="$4"
spectraHistNameId="$5"
extractRatios=1

fileData="AnalysisResults"
fileMC="mc"
signalName="SignalDeuteronSys"
spectraName="SpectraDeuteronSys"
EfficiencyHe3="EfficiencyDeuteronSys"
PrimaryHe3="PrimaryDeuteronSys"

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
.L Spectra.cpp+
Spectra("$cutSettings",$argumentSignal,$sigmoidFlag,"$spectraHistNameId","$spectraName","update","AnalysisResults","$signalName","$EfficiencyHe3","$PrimaryHe3")
.q
EOF
fi
