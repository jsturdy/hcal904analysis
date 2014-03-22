#!/bin/bash

if (( ${#LOCALRT} < 4 ))
then
    if (( ${#HCALDAQ_SW_LOC} > 3  && ${#HCAL_CMSSW_RELEASE} > 3 ))
    then
#	pushd $HCALDAQ_SW_LOC/src/$HCAL_CMSSW_RELEASE/src >/dev/null
	pushd $HCALDAQ_SW_LOC/sw/slc4_ia32_gcc345/cms/cmssw/CMSSW_2_1_11/src >/dev/null
	eval `scramv1 runtime -sh`
	popd >/dev/null
    fi
fi

if (( ${#LOCALRT} < 4 ))
then
    echo Please setup your runtime environment!
    exit
fi

ARG1=$1
ARG2=$2

if [[ -e ./reco_setup.rc ]] 
then
    source ./reco_setup.rc
fi

if (( ${#DOCALIB} > 1 ))
then
    UNPACKCALIB=true
else
    UNPACKCALIB=false
fi

# ARG1 determines the file selection mode
#
if [[ "${ARG1}" == *[[:alpha:]]* ]]
then
    # Filename mode
    FILE=$ARG1
else
    # Run Number mode
    FILE=`printf "${FORMAT}" ${ARG1}`
fi

echo $FILE

# ARG2 determines the file selection mode
#
#if [[ "${ARG2}" == *[[:alpha:]]* ]]
if [[ ${ARG2} ]]
then
    # Filename mode
    OUTPUTFILE=$ARG2
else
    # Run Number mode
    OUTPUTFILE=`printf "${OUTFORMAT}" ${ARG1}`
fi

echo $OUTPUTFILE

if (( ${#EVENTLIMIT} == 0 )) 
then
    EVENTLIMIT="-1";
fi


#### common head part of Config File
### create the file
CFGFILE=/tmp/runKPlotter_${USER}_cfg.py
cat > ${CFGFILE}<<EOF
import FWCore.ParameterSet.Config as cms

process = cms.Process("ANAL")

EOF

### Mode-dependent part

if [[ "$MODE" == "TESTSTAND" || "${FILE}" == *"HTBqc_"* ]]
then
# STREAMS: comma-separated list of strings, like: " 'HCAL_Trigger','HCAL_DCC020'  "
    STREAMS=","${FEDS}
    STREAMS=${STREAMS/20/020} # special case for teststand files
    STREAMS=${STREAMS//,/\'\ ,\'\HCAL_DCC}
    STREAMS="'HCAL_Trigger', 'HCAL_SlowData"${STREAMS}\'

    EXTRAPREPATH="process.tbunpacker *"
cat >> ${CFGFILE}<<EOF
#       Loads the events from testbeam files
process.source = cms.Source("HcalTBSource",
    fileNames = cms.untracked.vstring( "file:${FILE}" ),
    streams   = cms.untracked.vstring( ${STREAMS} )
)

process.tbunpacker = cms.EDFilter("HcalTBObjectUnpacker",
    HcalTriggerFED       = cms.untracked.int32(1),
    HcalSlowDataFED      = cms.untracked.int32(3),
    HcalTDCFED           = cms.untracked.int32(-1),
    HcalSourcePosFED     = cms.untracked.int32(-1),
    IncludeUnmatchedHits = cms.untracked.bool(False)
)

EOF

elif [[ "$MODE" == "USC" ]] 
    then
    EXTRAPREPATH="process.tbunpacker *"
cat >> ${CFGFILE}<<EOF
    // Loads the events from testbeam files
process.source = cms.Source("HcalTBSource",
    fileNames = cms.untracked.vstring( "file:${FILE}" ),
    streams   = cms.untracked.vstring( 'HCAL_Trigger','HCAL_SlowData',
        'HCAL_DCC700','HCAL_DCC701','HCAL_DCC702','HCAL_DCC703',
        'HCAL_DCC704','HCAL_DCC705','HCAL_DCC706','HCAL_DCC707',
        'HCAL_DCC708','HCAL_DCC709','HCAL_DCC710','HCAL_DCC711',
        'HCAL_DCC712','HCAL_DCC713','HCAL_DCC714','HCAL_DCC715',
        'HCAL_DCC716','HCAL_DCC717','HCAL_DCC718','HCAL_DCC719',
        'HCAL_DCC720','HCAL_DCC721','HCAL_DCC722','HCAL_DCC723',
        'HCAL_DCC724','HCAL_DCC725','HCAL_DCC726','HCAL_DCC727',
        'HCAL_DCC728','HCAL_DCC729','HCAL_DCC730','HCAL_DCC731' 
    )
)

process.tbunpacker = cms.EDFilter("HcalTBObjectUnpacker",
    HcalTriggerFED       = cms.untracked.int32(1),
    HcalSlowDataFED      = cms.untracked.int32(3),
    HcalTDCFED           = cms.untracked.int32(-1),
    HcalSourcePosFED     = cms.untracked.int32(-1),
    IncludeUnmatchedHits = cms.untracked.bool(False)
)

EOF
else
  echo Unknown mode '$MODE'
  exit
fi    

#### common tail part of Config File
cat >> ${CFGFILE}<<EOF99


process.MessageLogger = cms.Service("MessageLogger",
    destinations = cms.untracked.vstring( "cout" ),
    categories   = cms.untracked.vstring( "DDLParser", "FwkJob", "FwkReport"),
    cout = cms.untracked.PSet(
        INFO = cms.untracked.PSet( 
            limit = cms.untracked.int32(10000) 
        ),
        threshold = cms.untracked.string("INFO"),
        DDLParser = cms.untracked.PSet(
            limit = cms.untracked.int32(0)
        ),
        FwkJob    = cms.untracked.PSet(
            limit = cms.untracked.int32(10)
        ),
        FwkReport = cms.untracked.PSet(
            limit = cms.untracked.int32(20)
        ),
        default   = cms.untracked.PSet(
            reportEvery = cms.untracked.int32(100)
        )
    )
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(${EVENTLIMIT})
)

process.kplotter = cms.EDAnalyzer("KPlotter",
    debugit = cms.untracked.bool(False),
    singleMode = cms.untracked.bool(False),
    reportOD = cms.untracked.bool(False),
    debugEvn = cms.untracked.int32(0)
    #trigtype = cms.untracked.int(0)
)


process.TFileService = cms.Service("TFileService",
    fileName = cms.string("${OUTPUTFILE}"),
    closeFileFast = cms.untracked.bool(True)
)

process.p = cms.Path( ${EXTRAPREPATH} process.kplotter )


EOF99
echo "" >> ${CFGFILE}

# run cmsRun
cmsRun ${CFGFILE}

# root -l -x makeplots.cc
