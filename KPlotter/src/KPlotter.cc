
// -*- C++ -*-
//
// Package:    KPlotter
// Class:      Kplotter
// 
/**\class KPlotter KPlotter.cc subsystem/KPlotter/src/KPlotter.cc

Description: <Modification of the plotALL904rgk.C script on cmsmoe4>

Implementation:
<Notes on implementation>
*/
//
// Original Author:  Drew Baden
//         Created:  Tue Aug 12 14:53:11 CEST 2008
// $Id$
//
//


// system include files
#include <memory>
#include <cmath>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DataFormats/Common/interface/Handle.h"
#include <DataFormats/FEDRawData/interface/FEDRawDataCollection.h>
#include <DataFormats/FEDRawData/interface/FEDHeader.h>
#include <DataFormats/FEDRawData/interface/FEDTrailer.h>
#include <DataFormats/FEDRawData/interface/FEDNumbering.h>
#include <DataFormats/FEDRawData/interface/FEDRawData.h>

#include "PhysicsTools/UtilAlgos/interface/TFileService.h"

#include <TBDataFormats/HcalTBObjects/interface/HcalTBTriggerData.h>
#include <RecoTBCalo/HcalTBObjectUnpacker/interface/HcalTBTriggerDataUnpacker.h>


#include "TH1F.h"
#include "TH2F.h"
#include "TGraphErrors.h"
#include "TCanvas.h"
#include "TFrame.h"

#include <iostream>
#include <iomanip>

using namespace edm;
using namespace std;
using namespace hcaltb;




//
// class decleration
//
//bool debugit = true;
int qie2fC[4][32]={
  { -1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,15,  //0-15
    17,19,21,23,25,27,29,32,35,38,42,46,50,54,59,64 }, //16-31
  { 59,64,69,74,79,84,89,94,99,104,109,114,119,124,129,137,  //32-47
    147,157,167,177,187,197,209,224,239,254,272,292,312,334,359,384 }, //48-63
  { 359,384,409,434,459,484,509,534,559,584,609,634,659,684,709,747, //64-79
    797,847,897,947,997,1047,1109,1184,1259,1334,1422,1522,1622,1734,1859,1984 }, //80-95
  { 1859,1984,2109,2234,2359,2484,2609,2734,2859,2984,3109,3234,3359,3484,3609,3797, //96-101
    4047,4297,4547,4797,5047,5297,5609,5984,6359,6734,7172,7672,8172,8734,9359,9984 } }; //102-127

//bool first = true;
bool first[33] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

const uint32_t* payload;
size_t size;
int hspigot[18];
int wspigot[18];
int ospigot[18];
int spigots = 0;
uint32_t* hdata;  //here is the 16-bit HTR payload data
uint32_t  n16;     //number of 16-bit words for this HTR payload
uint32_t* tpgs;
uint32_t  nqies;
uint32_t* tpinfos;
uint32_t* qies;
uint32_t* extra;

//data word information
int qie,mant,range,capid,dv,er,addr,fib;
//tp word information
int tptag,tpz,tpsoi,tpfeature,tpval,tpfull;
//extra word information
int arrivebcn,empty,fullbcn,latcnt; //normal mode extra information
int ch12to1[12],ch24to13[12],tpthresh,tpsup,futuse,digibuffstat,daqthreshch1;
int tpbuffstat,daqthreshch24,fiblatcnt[8],bcnofrxbc0; //unspuressed mode extra information

const uint32_t ntptype[16] = {0,24,16,16,16,16,16,16,2,24,16,16,16,16,16,16};

void qies_unpack(int which);
void extra_unpack(int which);
void tpinfos_unpack(int what);
bool unpack_FED_data(int fed, const FEDRawData& data);
void unpack_FED_spigots();
void unpack_spigot(int spigot);
void unpack_spigot_header();
void unpack_spigot_tps();
void unpack_spigot_qies();
bool unpack_spigot_extra();
void print_spigot_qies();
void print_spigot_tps();

int sprinkle = 0;
int nwords,htrerr,lrberr,ee,ep,eb,ev,et;

bool debugit = true;
bool debugme = true;

int evn1, evn, htrn, ow, bz, hee, rl, le, lw, od, ck, be, tmb, hm, ct, one;
int bcn, ntps, fw, htype, tpsamps, qiesamps, ndccw, nwc, ttcready, dll, evn2;
int nqiech, ntpch;
int cmbit, usbit;
bool singleMode = false;
bool singleModeCancel = false;
bool reportOD = true;
int debugEvn = 0;
int firstFED = -1;

int channel;
int counterLED[13][24][20];
int counterLaser[13][24][20];
int trigtype;
bool containsTPs;

int evnprev = -1;

const int NCHANS = 24;

int qie_counter[33][13][NCHANS][3];//first for Pedestal triggers, second for LED, third for Laser
int timing_counterled[33][13][NCHANS];
int timing_counterlaser[33][13][NCHANS];
int sum_qie[33][13][NCHANS][3];//first for Pedestal triggers, second for LED, third for Laser
int sum2_qie[33][13][NCHANS][3];
double qie_mean[33][13][NCHANS][3];
double qie_sd[33][13][NCHANS][3];

double timing_meanled[33][13][NCHANS];
double timing2_meanled[33][13][NCHANS];
double timing_sdled[33][13][NCHANS];

double timing_meanlaser[33][13][NCHANS];
double timing2_meanlaser[33][13][NCHANS];
double timing_sdlaser[33][13][NCHANS];

int numspigs[33];

bool checkingonprint = true;
uint32_t trigger_word;

TH2F* hinvalid[33][50];

TH1F* htrigtype[33];
TH1F* hcapiderr[33][13];

// Histograms for LED type runs
TH1F* htpsAllLED[33];
TH1F* hqieAllLED[33];
TH1F* hqieNoSLED[33];
//plots per spigot
TH1F* hqiemeanLED[33][13];
TH1F* htimingmeanled[33][13];
TH1F* htimingdevled[33][13];
TH1F* htpmeanLED[33][13];
TH1F* htprmsLED[33][13];
TH1F* hbcnidleLED[33][13];
//plots summarized over channels
TH1F* htpSummaryShapeLED[33][13];
TH1F* htpSummarySOIShapeLED[33][13];
TH1F* htpSummaryFeatureShapeLED[33][13];
//plots per channel
TH1F* hqieShapeLED[33][13][NCHANS];
TH1F* hqieSumLED[33][13][NCHANS];
TH1F* htpShapeLED[33][13][NCHANS];
TH1F* htpFeatureShapeLED[33][13][NCHANS];
TH1F* htpSOIShapeLED[33][13][NCHANS];
TH1F* htpMaxLED[33][13][NCHANS];
//plots per fibre
TH1F* hbcnIdleLED[33][13][NCHANS/3-2];
TH1F* hmeanTSminBCNIdleLED[33][13][NCHANS/3-2];
TH1F* hweighttimingmeanled[33][13][NCHANS/3-2];
//2D plots
TH2F* htpnonzeroLED[33];
TH2F* htpsoiLED[33];
TH2F* htpfeatureLED[33];
TH2F* htpvalLED[33];
TH2F* hbcnidleLED2D[33];
TH2F* hmeanTSminBCNIdleLED2D[33];

// Histograms for Laser type runs
TH1F* htpsAllLaser[33];
TH1F* hqieAllLaser[33];
TH1F* hqieNoSLaser[33];
//plots per spigot
TH1F* hqiemeanLaser[33][13];
TH1F* htimingmeanlaser[33][13];
TH1F* htimingdevlaser[33][13];
TH1F* htpmeanLaser[33][13];
TH1F* htprmsLaser[33][13];
TH1F* hbcnidleLaser[33][13];
//plots summarized over channels
TH1F* htpSummaryShapeLaser[33][13];
TH1F* htpSummarySOIShapeLaser[33][13];
TH1F* htpSummaryFeatureShapeLaser[33][13];
//plots per channel
TH1F* hqieShapeLaser[33][13][NCHANS];
TH1F* hqieSumLaser[33][13][NCHANS];
TH1F* htpShapeLaser[33][13][NCHANS];
TH1F* htpFeatureShapeLaser[33][13][NCHANS];
TH1F* htpSOIShapeLaser[33][13][NCHANS];
TH1F* htpMaxLaser[33][13][NCHANS];
//plots per fibre
TH1F* hbcnIdleLaser[33][13][NCHANS/3-2];
TH1F* hmeanTSminBCNIdleLaser[33][13][NCHANS/3-2];
TH1F* hweighttimingmeanlaser[33][13][NCHANS/3-2];
//2D plots
TH2F* htpnonzeroLaser[33];
TH2F* htpsoiLaser[33];
TH2F* htpfeatureLaser[33];
TH2F* htpvalLaser[33];
TH2F* hbcnidleLaser2D[33];
TH2F* hmeanTSminBCNIdleLaser2D[33];

//Histograms for Pedestal type runs
TH1F* htpsAllPedestal[33];
TH1F* hqieShapePedestal[33][13][NCHANS];
TH1F* hqieSumPedestal[33][13][NCHANS];
TH1F* hqieAllPedestal[33];
TH1F* hqieNoSPedestal[33];
TH1F* hqieMAXPedestal[33];
TH1F* hqieZSPedestal[33];
TH1F* hqiemeanPedestal[33][13];
TH1F* hpedestal[33][13];
TH1F* hpedestalrms[33][13];

class KPlotter : public edm::EDAnalyzer {
public:

  edm::Service<TFileService> fs;
  explicit KPlotter(const edm::ParameterSet&);
  ~KPlotter();
    
    
private:
  virtual void beginJob(const edm::EventSetup&) ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;
    
  bool debugit_;
  bool singleMode_;
  bool reportOD_;
  int debugEvn_;
  int bcnidle_;
  // ----------member data ---------------------------
};

bool unpack_FED_data(int fed, const FEDRawData& data) {
  size=data.size();
  FEDHeader header(data.data());
  FEDTrailer trailer(data.data()+size-8);
  payload=(uint32_t*)(data.data());
  if (size == 0) return false;
  //	cout << "unpack_FED_data: FED# " << fed << " has " << size << " bytes " << std::endl;
  //
  // very first 4 words...
  //
  int fedn = (payload[0] >> 8) & 0xFFF;  // 12 bits
  int bcn =  (payload[0] >> 20) & 0xFFF;  // 12 bits
  int evn =  payload[1] & 0xFFFFFF; // 24 bits
  
  for (unsigned int i = 0; i < size; ++i) 
    if (debugme) std::cout<<"payload["<<i<<"]: 0x"<<hex<<payload[i]<<dec<<std::endl;
  //        int evtype = (payload[1]>>25) & 0xF;
  //        cout << "Event type: "<<evtype<<endl;
  int orn1 = (payload[2] >> 4) & 0xFFFFFFFF;
  int orn2 = (payload[3] & 0xF);
  int orn = orn1 + (orn2 *65536*65536);
  if (debugme) {
    cout << "   Fed " << fedn << "  BcN " << bcn << "  EvN " << evn;
    cout << "  OrN " << orn;
    cout << " L1Id: " << setw(8) << header.lvl1ID();
    cout << " BXId: " << setw(4) << header.bxID() << std::endl;
  }
  //
  // check if the fed numbers agree.  if now we are cooked
  //
  if (fedn != fed) {
    cout << "ERROR!!!  Fed number " << fed << " does not match " <<
      fedn << " in DCC Header" << std::endl;
    return false;
  }
  //
  // next comes the dcc header/status info
  //
  int formver = payload[4] & 0xFF;
  int TTS = (payload[4] >> 8) & 0xF;
  int HTRstatus = (payload[4] >> 14) & 0x3FF;
  int DCCstatus = payload[5] & 0x3F;
  int DCCrev = (payload[5] >> 16) & 0xFF;
  if (debugme) {
    cout << "   HTR status " << hex << HTRstatus << " TTS " << TTS
	 << dec << " FormatVersion " << formver
	 << hex << " DCCrev: " << DCCrev << " DCC status " << DCCstatus << dec << std::endl;
  }
  return true;
}

void unpack_FED_spigots() {
  if (debugme) {
    std::cout << "unpack_FED_spigots:" << std::endl;
    std::cout << "  Spigot  #Words  HTRerr  LRBerr  E-P-B-V-T " << std::endl;
  }
  spigots = 0;
  for (int i=0; i<18; i++) {
    //
    // dig these out, 32 bits at a time...
    uint32_t s = payload[i+6] & 0xFFFFFFFF;
    nwords = (s & 0x3FF);
    htrerr = (s>>24) & 0xFF;
    lrberr = (s>>16) & 0xFF;
    ee = (s>>15) & 0x1;
    ep = (s>>14) & 0x1;
    eb = (s>>13) & 0x1;
    ev = (s>>12) & 0x1;
    et = (s>>11) & 0x1;
    if (nwords > 0) {
      hspigot[spigots] = s;
      wspigot[spigots] = nwords;
      if (debugme) {
	printf("payload:   %4d    %5d    0x%2.2x    0x%2.2x   %d-%d-%d-%d-%d\n",
	       spigots,nwords,htrerr,lrberr,ee,ep,eb,ev,et);
      }
      spigots++;
    }
  }
  if (debugme) cout << "   There were " << spigots << " spigots found" << std::endl;
  //
  // now it's easy to calculate the offset to the beginning of each "spigot"
  // (aka HTR) payload using the above hspigot and wspigot variables
  //
  ospigot[0] = 24;
  for (int i=1; i<spigots; i++) ospigot[i] = ospigot[i-1] + wspigot[i-1];
  //	if (debugme) for (int i=0; i<spigots; i++) 
  //	    	cout << dec << "    Spigot " << i << " offset " << ospigot[i] << std::endl;
    
}

void unpack_spigot(int i) {

  int iptr = ospigot[i];
  int nwords = wspigot[i];
  n16 = 2*nwords;
  hdata = new uint32_t [n16];
  for (int j=0; j<nwords; j++) {
    hdata[2*j] = payload[iptr+j] & 0xFFFF;
    //	    printf("%d %d 0x%x\n",j,2*j,hdata[2*j]);
    hdata[2*j+1] = (payload[iptr+j] >> 16) & 0xFFFF;
    //	    printf("%d %d 0x%x\n",j,2*j+1,hdata[2*j+1]);
  }

}

void unpack_spigot_header() {

  evn1 = (hdata[0] & 0xFF);
  evn = evn1 + (hdata[1] * 256);
  htrn = hdata[3] & 0x7FF;
  ow = (hdata[2] & 0x1);          //overflow warning
  bz = (hdata[2]>>1) & 0x1;       //internal buffers busy, fast report
  hee = (hdata[2]>>2) & 0x1;      //empty event
  rl = (hdata[2]>>3) & 0x1;       //rejected previous L1A
  le = (hdata[2]>>4) & 0x1;       //latency error
  lw = (hdata[2]>>5) & 0x1;       //latency warning
  od = (hdata[2]>>6) & 0x1;       //optical data error
  ck = (hdata[2]>>7) & 0x1;       //clocking problems
  be = (hdata[2]>>8) & 0x1;       //bunch error
  tmb = (hdata[2]>>12) & 0x1;     //test mode (1=patterns)
  hm = (hdata[2]>>13) & 0x1;      //histo mode
  ct = (hdata[2]>>14) & 0x1;      //calibration trigger
  one = (hdata[2]>>15) & 0x1;     //should be 1
  bcn = hdata[4] & 0xFFF;
  ntps = (hdata[5]>>8);           //number of TP words
  fw = hdata[6] & 0xFF;
  cmbit = (hdata[6]>>15)&0x1;     //compact mode indicator
  usbit = (hdata[6]>>16)&0x1;     //unsuppressed mode indicator
  htype = (hdata[7]>>8);
  if (htype == 0) tpsamps = 0;
  else tpsamps = ntps/ntptype[htype];
  qiesamps = hdata[n16-4]>>11;
  ndccw = hdata[n16-2];
  nwc = hdata[n16-3];
  ttcready = hdata[5] & 0x1;
  dll = (hdata[5]>>1) & 0x3;
  evn2 = hdata[n16-1] >> 8;
  sprinkle = (hdata[6] >> 15) & 0x1;
  if(debugme){
    std::cout<< "hdata "  << hex << hdata << dec << std::endl;
    std::cout<< "sprinkle " << sprinkle << "hdata[6]= " << hex << hdata[6] << dec << std::endl;
    printf("htype  ntptype  ntps  tpsamps\n");
    printf("%15i  %7i  %4i  %7i\n",htype,ntptype[htype],ntps,tpsamps);
  }
  if(debugme) {
    std::cout<<
      "evn1 "<<             evn1<< "  "<<
      "evn "<<              evn<< "  "<<
      "htrn "<<             htrn<< "  "<<
      "ow "<<               ow<< "  "<<
      "bz "<<               bz<< "  "<<
      "hee "<<              hee<< "  "<<
      "rl "<<               rl<< "  "<<
      "le "<<               le<< "  "<<
      "lw "<<               lw<< "  "<<
      "od "<<               od<< "  "<<
      "ck "<<               ck<< "  "<<
      "be "<<               be<< "  "<<
      "tmb "<<              tmb<< "  "<<
      "hm "<<               hm<< "  "<<
      "ct "<<               ct<< "  "<<
      "one "<<              one<< "  "<<
      "bcn "<<              bcn<< "  "<<
      "ntps "<<             ntps<< "  "<<
      "fw "<<               fw<< "  "<<
      "cmbit "<<            cmbit<< "  "<<
      "usbit "<<            usbit<< "  "<<
      "htype "<<            htype<< "  "<<
      "tpsamps "<<          tpsamps<< "  "<<
      "qiesamps "<<         qiesamps<< "  "<<
      "ndccw "<<            ndccw<< "  "<<
      "nwc "<<              nwc<< "  "<<
      "ttcready "<<         ttcready<< "  "<<
      "dll "<<              dll<< "  "<<
      "evn2 "<<             evn2<< "  "<<
      "sprinkle "<<sprinkle<<std::endl;
  }
}


void unpack_spigot_tps() {
  tpinfos = new uint32_t [ntps];
  ntpch = ntps/tpsamps;
  for (int j=0; j<(int) ntps; j++) tpinfos[j] = hdata[j+8];
    
  //	  if (debugme) {
  //	    for (int j=0; j<(int) ntps; j++) cout << j << " " << hex << "0x" <<
  //	        tpinfos[j] << dec << std::endl;
  //	  }
    
}
void unpack_spigot_qies() {
  nqies = n16 - ntps - 20;
  qies = new uint32_t [nqies];
  nqiech = nqies/qiesamps;
  for (int j=0; j<(int) nqies; j++) qies[j] = hdata[j+ntps+8];
    
  //	  if (debugme) {
  //	    for (int j=0; j<(int) nqies; j++) cout << j << " " << hex << "0x" <<
  //	        qies[j] << dec << std::endl;
  //	  }
    
}

bool unpack_spigot_extra() {
  if (cmbit==1) return false; //compact mode, no extra words present in the data
  int offset = ntps + nqies + 8 + (nqies%2);
  extra = new uint32_t [8];
  for (int j=0; j<8; j++) extra[j] = hdata[j+offset];
  //after headers, tp data, qie data and parity word
  return true;
  //	  if (debugme) {
  //	    for (int j=0; j<8; j++) cout << j << " " << hex << "0x" <<
  //	        extra[j] << dec << std::endl;
  //	  }

}

void tpinfos_unpack(int what) {
  //
  // "what" goes from 0 to number of TP words from header word 6
  //

  tpval = tpinfos[what] & 0xFF;         //value of the TP
  tpfeature = (tpinfos[what]>>8) & 0x1; //is the feature bit set 
  tpfull = tpinfos[what] & 0x1FF;       //full TP including feature bit
  tpsoi = (tpinfos[what]>>9) & 0x1;     //sample of interest
  tpz = (tpinfos[what]>>10) & 0x1;      //tp Z, value of TP sent to RCT was zero
  tptag = (tpinfos[what]>>11) & 0x1F;   //tp tag
}

void qies_unpack(int which) {
  //
  // "which" goes from 0 to number of DAQ data words from trailer word -4
  //
  //	cout << "inside qies_unpack, qie " << which << " qies[]= " << hex <<
  //	   " 0x" << qies[which] << dec << std::endl;
  mant = qies[which] & 0x1F; //cout << mant << " mant";
  range = (qies[which]>>5) & 0x3; //cout << range << " exp";
  qie = qies[which] & 0x7F;
  capid = (qies[which]>>7) & 0x3; //cout << capid << " capid";
  dv = (qies[which]>>9) & 0x1; //cout << dv << " dv";
  er = (qies[which]>>10) & 0x1; //cout << er << " er";
  addr = (qies[which]>>11) & 0x3; //cout << addr << " addr";
  fib = (qies[which]>>13) & 0x7; //cout << fib << " fib" << std::endl;
  channel = ((3*fib)+1)+addr;
}

//almost always part of the extra words
void extra_unpack(int who) {
  //
  // "who" goes from 0 to number of extra info words from trailer word -4
  //
  //	cout << "inside extra_unpack, extra word " << who << " extra[]= " << hex <<
  //	   " 0x" << extra[who] << dec << std::endl;
  latcnt=(extra[who]>>12)&0x2;
  fullbcn=(extra[who]>>14)&0x1;
  empty=(extra[who]>>15)&0x1;
}

//special case extra word information

void extra_norm_unpack(int who) {
  arrivebcn=extra[who]&0xFFF;
}

void extra_unsup_unpack() {
  for(int chan=0;chan<12;chan++) {
    ch12to1[chan]=(extra[0]>>chan)&0x1;
    ch24to13[chan]=(extra[1]>>chan)&0x1;
  }

  tpthresh=extra[2]&0xFF;
  tpsup=(extra[2]>>8)&0x1;
  futuse=(extra[2]>>9)&0x7;
  daqthreshch1=extra[3]&0xFF;
  digibuffstat=(extra[3]>>8)&0xF;
  daqthreshch24=extra[4]&0xFF;
  tpbuffstat=(extra[4]>>8)&0xF;

  for(int fibcnt = 0; fibcnt<4;fibcnt++){
    fiblatcnt[fibcnt]=(extra[5]>>(2*fibcnt))&0x2;
    fiblatcnt[fibcnt+4]=(extra[6]>>(2*fibcnt))&0x2;
  }

  bcnofrxbc0=extra[7]&0xFFF;
}

void print_spigot_qies() {
  //
  // qiesamps is the variable that you use to know how many time samples
  //
  if(debugme) {
    std::cout << "    There are " << nqiech << " QIE channels present, " << 
      qiesamps  << " time samples per channel " << std::endl;
  }
  int ipt = 0;
  for (int i=0; i<nqiech; i++) {
    if (debugme) printf("i/samples: %3d/%3d \n",i+1,qiesamps);
    cout << "    Mant Exp  Cap  DV ER Fib Addr: " << std::endl;
    for (int j=0; j<qiesamps; j++) {
      qies_unpack(ipt);
        if(debugme){
	  printf("    %5d  %1d   %1d    %1d  %1d  %2d %3d \n",
	     mant,range,capid,dv,er,fib,addr);
	}
      ipt++;
    }
    if (debugme) printf("\n");
  }	

}

//
// constructors and destructor
//
KPlotter::KPlotter(const edm::ParameterSet& iConfig)
{
  //edm::Service<TFileService> fs;
  bcnidle_ = iConfig.getUntrackedParameter<int>("bcnIdle",97);
  trigtype = iConfig.getUntrackedParameter<int>("trigType",3);
  containsTPs = iConfig.getUntrackedParameter<bool>("containsTP",true);
  debugit_ = iConfig.getUntrackedParameter<bool>("debugit",false);
  debugit = debugit_;
  singleMode_ = iConfig.getUntrackedParameter<bool>("singleMode",false);
  singleMode = singleMode_;
  reportOD_ = iConfig.getUntrackedParameter<bool>("reportOD",false);
  reportOD = reportOD_;
  debugEvn_ = iConfig.getUntrackedParameter<int>("debugEvn",false);
  debugEvn = debugEvn_;
  if(debugme){
    cout << "KPlotter CONSTRUCTOR: Debug flag is " << debugit << std::endl;
    cout << "                   singleMode is " << singleMode << std::endl;
    cout << "                   reportOD is " << reportOD << std::endl;
    cout << "                   debugEvn is " << debugEvn << std::endl;
  }
  //now do what ever initialization is needed
}


KPlotter::~KPlotter()
{

  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
  //exit(0);
}


//
// member functions
//

// ------------ method called to for each event  ------------
void KPlotter::analyze(const edm::Event& e, const edm::EventSetup& c) {


  bool checker = true;
  //   while(checker) {

  int this_evn = e.id().event();

  char fname[128];
  char ftitle[128];

  debugme = debugit || (this_evn = debugEvn);
  Handle<FEDRawDataCollection> rawdata;
  e.getByType(rawdata);

  Handle<HcalTBTriggerData> trigd;
  e.getByType(trigd);

  bool wasPEDTrig = 0;
  bool wasLEDTrig = 0;
  bool wasLaserTrig = 0;
  bool wasBeamTrig = 0;
  string triggerT = "";

  if( trigd->wasInSpillPedestalTrigger()   ||
      trigd->wasOutSpillPedestalTrigger()  ||
      trigd->wasSpillIgnorantPedestalTrigger() ) 
    wasPEDTrig = 1;
  if( trigd->wasLEDTrigger())   wasLEDTrig   = 1;
  if( trigd->wasLaserTrigger()) wasLaserTrig = 1;
  if( trigd->wasBeamTrigger() ) wasBeamTrig  = 1;
  if (debugme) printf("Pedestal: %d - LED: %d - Laser: %d - Beam: %d\n",wasPEDTrig,wasLEDTrig,wasLaserTrig,wasBeamTrig);

  //   for (int i = 0; i<FEDNumbering::lastFEDId(); i++) {
  for (int i = 700; i<733; i++) {

    const FEDRawData& data = rawdata->FEDData(i);
    //	cout << "looking for FED " << i << std::endl;
    if (!unpack_FED_data(i,data)) continue;
    //	cout << "found FED # "<<i<<"!" << std::endl;
    if (firstFED < 0) firstFED = i;
    //
    // ok, nonzero FED means there's something here
    //
    //

    // next comes the header words for the "spigot"s, 32 bits each.  There is
    // room in the FED data here for 18 spigots, although for the HCAL we only
    // really need 12 or 13
    //
    // I'll store these numbers in arrays so that it's easier later to dig out
    // the data from the individual spigot payloads
    //
    if (debugme) std::cout<<"Unpacking FED spigots"<<std::endl;
    unpack_FED_spigots();

    //In order to conserve resources, I will only attempt to create the necessary histograms
    if (first[i-700]) {
      //          cout<<"Now booking histograms for FED "<<i<<"... ";
      numspigs[i-700] = spigots;

      first[i-700] = false;
      //cout<<"Creating pedestal and timing histograms for FED # "<<i<<std::endl;

      sprintf(fname,"FED_%.2i",i);
      TFileDirectory feddir        = fs->mkdir(fname);
      TFileDirectory genplots      = feddir.mkdir("General");
      TFileDirectory genpedplots   = genplots.mkdir("Pedestal Plots");
      TFileDirectory genledplots   = genplots.mkdir("LED Plots");
      TFileDirectory genlaserplots = genplots.mkdir("Laser Plots");

      //General plots
      sprintf(fname,"Trigger type for event");
      htrigtype[i-700] = genplots.make<TH1F>(fname,fname,3,0,3);
      htrigtype[i-700]->SetXTitle("Trigger Type (0 for Pedestal, 1 for LED, 2 for Laser)");

      // General Pedestal plots
      htpsAllPedestal[i-700] = genpedplots.make<TH1F>("TPsAllPedestal","TPs All Pedestal",250,0,250);
      hqieAllPedestal[i-700] = genpedplots.make<TH1F>("QIEsAllPedestal","QIEs All Pedestal",15,0,15);
      hqieNoSPedestal[i-700] = genpedplots.make<TH1F>("QIEsNoSprinklePedestal","QIEs No Sprinkle Pedestal",25,0,25);
      hqieMAXPedestal[i-700] = genpedplots.make<TH1F>("QIEsMAXPedestal","QIEs Max Pedestal",25,0,25);
      hqieZSPedestal[i-700]  = genpedplots.make<TH1F>("QIEzsPedestal","QIEzs Pedestal",25,0,25);

      // General LED plots
      htpsAllLED[i-700]    = genledplots.make<TH1F>("TPsAllLED","TPs All LED",250,0,250);
      hqieAllLED[i-700]    = genledplots.make<TH1F>("QIEsAllLED","QIEs All LED",2000,0,2000);
      hqieNoSLED[i-700]    = genledplots.make<TH1F>("QIEsNoSprinkleLED","QIEs No Sprinkle LED",10000,0,10000);
      htpnonzeroLED[i-700] = genledplots.make<TH2F>("NonZeroTPs","Non-Zero TPs vs spigot and channel",24,0,23,14,0,13);
      htpsoiLED[i-700]     = genledplots.make<TH2F>("TPsSOI","TP_SOI vs spigot and channel",96,0,23,14,0,13);
      htpfeatureLED[i-700] = genledplots.make<TH2F>("TPsFeatureBit","TP_Feature vs spigot and channel",96,0,23,14,0,13);
      htpvalLED[i-700]     = genledplots.make<TH2F>("TPs","TPs vs spigot and channel",96,0,23,14,0,13);

      sprintf(fname,"BCN_Idle");
      hbcnidleLED2D[i-700] = genledplots.make<TH2F>(fname,fname,8,0.5,8.5,4,-0.5,3.5);
      hbcnidleLED2D[i-700]->SetXTitle("Fibre Number");
      hbcnidleLED2D[i-700]->SetYTitle("Spigot Number");
      
      // General Laser plots
      htpsAllLaser[i-700]    = genlaserplots.make<TH1F>("TPsAllLaser","TPs All Laser",250,0,250);
      hqieAllLaser[i-700]    = genlaserplots.make<TH1F>("QIEsAllLaser","QIEs All Laser",2000,0,2000);
      hqieNoSLaser[i-700]    = genlaserplots.make<TH1F>("QIEsNoSprinkleLaser","QIEs No Sprinkle Laser",10000,0,10000);
      htpnonzeroLaser[i-700] = genlaserplots.make<TH2F>("NonZeroTPs","Non-Zero TPs vs spigot and channel",24,0,23,14,0,13);
      htpsoiLaser[i-700]     = genlaserplots.make<TH2F>("TPsSOI","TP_SOI vs spigot and channel",96,0,23,14,0,13);
      htpfeatureLaser[i-700] = genlaserplots.make<TH2F>("TPsFeatureBit","TP_Feature vs spigot and channel",96,0,23,14,0,13);
      htpvalLaser[i-700]     = genlaserplots.make<TH2F>("TPs","TPs vs spigot and channel",96,0,23,14,0,13);

      sprintf(fname,"BCN_Idle");
      hbcnidleLaser2D[i-700] = genlaserplots.make<TH2F>(fname,fname,8,0.5,8.5,4,-0.5,3.5);
      hbcnidleLaser2D[i-700]->SetXTitle("Fibre Number");
      hbcnidleLaser2D[i-700]->SetYTitle("Spigot Number");

      for ( int jj = 0; jj < numspigs[i-700]; jj++ ) {
	sprintf(fname,"Spigot %.2i",jj);
	TFileDirectory spigplots       = feddir.mkdir(fname);
	TFileDirectory spigledplots    = spigplots.mkdir("LED Plots");
	TFileDirectory spiglaserplots  = spigplots.mkdir("Laser Plots");
	TFileDirectory spigpedplots    = spigplots.mkdir("Pedestal Plots");
	TFileDirectory ledshapeplots   = spigledplots.mkdir("Shape");
	TFileDirectory ledsumplots     = spigledplots.mkdir("Sum");
	TFileDirectory ledbcnplots     = spigledplots.mkdir("BCN");

	TFileDirectory lasershapeplots   = spiglaserplots.mkdir("Shape");
	TFileDirectory lasersumplots     = spiglaserplots.mkdir("Sum");
	TFileDirectory laserbcnplots     = spiglaserplots.mkdir("BCN");

	TFileDirectory pedshapeplots = spigpedplots.mkdir("Shape");
	TFileDirectory pedsumplots   = spigpedplots.mkdir("Sum");

	//Spigot plots
	sprintf(fname,"CapIDErrsOnSpig_%.2i",jj);
	sprintf(ftitle,"Cap ID Errors on Spigot %.2i",jj);
	hcapiderr[i-700][jj] = spigplots.make<TH1F>(fname,ftitle,26,0,25);
	hcapiderr[i-700][jj]->SetXTitle("Channel Number");

	//Spigot Pedestal plots
	sprintf(fname,"Pedestal mean of spigot %.2i",jj);
	hpedestal[i-700][jj] = spigpedplots.make<TH1F>(fname,fname,26,0,25);
	hpedestal[i-700][jj]->SetXTitle("Channel Number");

	sprintf(fname,"Pedestal RMS of spigot %.2i",jj);
	hpedestalrms[i-700][jj] = spigpedplots.make<TH1F>(fname,fname,26,0,25);
	hpedestalrms[i-700][jj]->SetXTitle("Channel Number");

	sprintf(fname,"QIEsSumPedestal_S%.2i",jj);
	sprintf(ftitle,"QIEs Sum Pedestal Spigot %.2i",jj);
	hqiemeanPedestal[i-700][jj] = spigpedplots.make<TH1F>(fname,ftitle,26,0,25);
	hqiemeanPedestal[i-700][jj]->SetXTitle("Channel Number");

	//Spigot LED plots
	sprintf(fname,"TP Shape spigot %.2i",jj);
	htpSummaryShapeLED[i-700][jj] = spigledplots.make<TH1F>(fname,fname,20,-0.5,19.5);
	htpSummaryShapeLED[i-700][jj]->SetXTitle("Sample Number");
	sprintf(fname,"SOI Shape spigot %.2i",jj);
	htpSummarySOIShapeLED[i-700][jj] = spigledplots.make<TH1F>(fname,fname,20,-0.5,19.5);
	htpSummarySOIShapeLED[i-700][jj]->SetXTitle("Sample Number");
	sprintf(fname,"Feature Bit Shape spigot %.2i",jj);
	htpSummaryFeatureShapeLED[i-700][jj] = spigledplots.make<TH1F>(fname,fname,20,-0.5,19.5);
	htpSummaryFeatureShapeLED[i-700][jj]->SetXTitle("Sample Number");

	sprintf(fname,"LED timing mean of spigot %.2i",jj);
	htimingmeanled[i-700][jj]   = spigledplots.make<TH1F>(fname,fname,26,0,25);
	htimingmeanled[i-700][jj]->SetXTitle("Channel Number");
	sprintf(fname,"LED timing RMS of spigot %.2i",jj);
	htimingdevled[i-700][jj]   = spigledplots.make<TH1F>(fname,fname,26,0,25);
	htimingdevled[i-700][jj]->SetXTitle("Channel Number");

	sprintf(fname,"QIEsSumLED_S%.2i",jj);
	sprintf(ftitle,"QIEs Sum LED Spigot %.2i",jj);
	hqiemeanLED[i-700][jj] = spigledplots.make<TH1F>(fname,ftitle,26,0,25);
	hqiemeanLED[i-700][jj]->SetXTitle("Channel Number");

	sprintf(fname,"TPsMaxMeanLED_S%.2i",jj);
	sprintf(ftitle,"Max TP mean LED Spigot %.2i",jj);
	htpmeanLED[i-700][jj] = spigledplots.make<TH1F>(fname,ftitle,26,0,25);
	htpmeanLED[i-700][jj]->SetXTitle("Channel Number");

	sprintf(fname,"TPsMaxRMSLED_S%.2i",jj);
	sprintf(ftitle,"Max TP rms LED Spigot %.2i",jj);
	htprmsLED[i-700][jj] = spigledplots.make<TH1F>(fname,ftitle,26,0,25);
	htprmsLED[i-700][jj]->SetXTitle("Channel Number");

	//Spigot Laser plots
	sprintf(fname,"TP Shape spigot %.2i",jj);
	htpSummaryShapeLaser[i-700][jj] = spiglaserplots.make<TH1F>(fname,fname,20,-0.5,19.5);
	htpSummaryShapeLaser[i-700][jj]->SetXTitle("Sample Number");
	sprintf(fname,"SOI Shape spigot %.2i",jj);
	htpSummarySOIShapeLaser[i-700][jj] = spiglaserplots.make<TH1F>(fname,fname,20,-0.5,19.5);
	htpSummarySOIShapeLaser[i-700][jj]->SetXTitle("Sample Number");
	sprintf(fname,"Feature Bit Shape spigot %.2i",jj);
	htpSummaryFeatureShapeLaser[i-700][jj] = spiglaserplots.make<TH1F>(fname,fname,20,-0.5,19.5);
	htpSummaryFeatureShapeLaser[i-700][jj]->SetXTitle("Sample Number");

	sprintf(fname,"Laser timing mean of spigot %.2i",jj);
	htimingmeanlaser[i-700][jj]   = spiglaserplots.make<TH1F>(fname,fname,26,0,25);
	htimingmeanlaser[i-700][jj]->SetXTitle("Channel Number");
	sprintf(fname,"Laser timing RMS of spigot %.2i",jj);
	htimingdevlaser[i-700][jj]   = spiglaserplots.make<TH1F>(fname,fname,26,0,25);
	htimingdevlaser[i-700][jj]->SetXTitle("Channel Number");

	sprintf(fname,"QIEsSumLaser_S%.2i",jj);
	sprintf(ftitle,"QIEs Sum Laser Spigot %.2i",jj);
	hqiemeanLaser[i-700][jj] = spiglaserplots.make<TH1F>(fname,ftitle,26,0,25);
	hqiemeanLaser[i-700][jj]->SetXTitle("Channel Number");

	sprintf(fname,"TPsMaxMeanLaser_S%.2i",jj);
	sprintf(ftitle,"Max TP mean Laser Spigot %.2i",jj);
	htpmeanLaser[i-700][jj] = spiglaserplots.make<TH1F>(fname,ftitle,26,0,25);
	htpmeanLaser[i-700][jj]->SetXTitle("Channel Number");

	sprintf(fname,"TPsMaxRMSLaser_S%.2i",jj);
	sprintf(ftitle,"Max TP rms Laser Spigot %.2i",jj);
	htprmsLaser[i-700][jj] = spiglaserplots.make<TH1F>(fname,ftitle,26,0,25);
	htprmsLaser[i-700][jj]->SetXTitle("Channel Number");

	for (int yy = 0; yy < NCHANS; yy++) {
	  //Pedestal Shape plots
	  sprintf(fname,"QIEShapePedestal_S%.2i_Ch%.2i",jj,yy+1);
	  sprintf(ftitle,"QIE Shape Pedestal Spigot %.2i Channel %.2i",jj,yy+1);
	  hqieShapePedestal[i-700][jj][yy] = pedshapeplots.make<TH1F>(fname,ftitle,20,0,20);
	  hqieShapePedestal[i-700][jj][yy]->SetXTitle("Sample Number");
	  //Pedestal Sum plots
	  sprintf(fname,"QIEsSumPedestal_S%.2i_Ch%.2i",jj,yy+1);
	  sprintf(ftitle,"QIEs Sum Pedestal Spigot %.2i Channel %.2i",jj,yy+1);
	  hqieSumPedestal[i-700][jj][yy] = pedsumplots.make<TH1F>(fname,ftitle,100,0,150);


	  //LED Shape plots
	  sprintf(fname,"QIEShapeLED_S%.2i_Ch%.2i",jj,yy+1);
	  sprintf(ftitle,"QIE Shape LED Spigot %.2i Channel %.2i",jj,yy+1);
	  hqieShapeLED[i-700][jj][yy] = ledshapeplots.make<TH1F>(fname,ftitle,20,-0.0,20.0);
	  hqieShapeLED[i-700][jj][yy]->SetXTitle("Sample Number");

	  sprintf(fname,"TPShapeLED_S%.2i_Ch%.2i",jj,yy+1);
	  sprintf(ftitle,"TP Shape LED Spigot %.2i Channel %.2i",jj,yy+1);
	  htpShapeLED[i-700][jj][yy] = ledshapeplots.make<TH1F>(fname,ftitle,20,0.0,20.0);
	  htpShapeLED[i-700][jj][yy]->SetXTitle("Sample Number");

	  sprintf(fname,"SOIShapeLED_S%.2i_Ch%.2i",jj,yy+1);
	  sprintf(ftitle,"SOI Shape LED Spigot %.2i Channel %.2i",jj,yy+1);
	  htpSOIShapeLED[i-700][jj][yy] = ledshapeplots.make<TH1F>(fname,ftitle,20,-0.5,19.5);
	  htpSOIShapeLED[i-700][jj][yy]->SetXTitle("Sample Number");

	  sprintf(fname,"FeatureShapeLED_S%.2i_Ch%.2i",jj,yy+1);
	  sprintf(ftitle,"Feature bit Shape LED Spigot %.2i Channel %.2i",jj,yy+1);
	  htpFeatureShapeLED[i-700][jj][yy] = ledshapeplots.make<TH1F>(fname,ftitle,20,-0.5,19.5);
	  htpFeatureShapeLED[i-700][jj][yy]->SetXTitle("Sample Number");

	  //LED Sum plots
	  sprintf(fname,"QIEsSumLED_S%.2i_Ch%.2i",jj,yy+1);
	  sprintf(ftitle,"QIEs Sum LED Spigot %.2i Channel %.2i",jj,yy+1);
	  hqieSumLED[i-700][jj][yy] = ledsumplots.make<TH1F>(fname,ftitle,3000,0,3000);

	  sprintf(fname,"TPsMaxLED_S%.2i_Ch%.2i",jj,yy+1);
	  sprintf(ftitle,"TPs Max LED Spigot %.2i Channel %.2i",jj,yy+1);
	  htpMaxLED[i-700][jj][yy] = ledsumplots.make<TH1F>(fname,ftitle,250,0,250);


	  //Laser Shape plots
	  sprintf(fname,"QIEShapeLaser_S%.2i_Ch%.2i",jj,yy+1);
	  sprintf(ftitle,"QIE Shape Laser Spigot %.2i Channel %.2i",jj,yy+1);
	  hqieShapeLaser[i-700][jj][yy] = lasershapeplots.make<TH1F>(fname,ftitle,20,-0.0,20.0);
	  hqieShapeLaser[i-700][jj][yy]->SetXTitle("Sample Number");

	  sprintf(fname,"TPShapeLaser_S%.2i_Ch%.2i",jj,yy+1);
	  sprintf(ftitle,"TP Shape Laser Spigot %.2i Channel %.2i",jj,yy+1);
	  htpShapeLaser[i-700][jj][yy] = lasershapeplots.make<TH1F>(fname,ftitle,20,0.0,20.0);
	  htpShapeLaser[i-700][jj][yy]->SetXTitle("Sample Number");

	  sprintf(fname,"SOIShapeLaser_S%.2i_Ch%.2i",jj,yy+1);
	  sprintf(ftitle,"SOI Shape Laser Spigot %.2i Channel %.2i",jj,yy+1);
	  htpSOIShapeLaser[i-700][jj][yy] = lasershapeplots.make<TH1F>(fname,ftitle,20,-0.5,19.5);
	  htpSOIShapeLaser[i-700][jj][yy]->SetXTitle("Sample Number");

	  sprintf(fname,"FeatureShapeLaser_S%.2i_Ch%.2i",jj,yy+1);
	  sprintf(ftitle,"Feature bit Shape Laser Spigot %.2i Channel %.2i",jj,yy+1);
	  htpFeatureShapeLaser[i-700][jj][yy] = lasershapeplots.make<TH1F>(fname,ftitle,20,-0.5,19.5);
	  htpFeatureShapeLaser[i-700][jj][yy]->SetXTitle("Sample Number");

	  //Laser Sum plots
	  sprintf(fname,"QIEsSumLaser_S%.2i_Ch%.2i",jj,yy+1);
	  sprintf(ftitle,"QIEs Sum Laser Spigot %.2i Channel %.2i",jj,yy+1);
	  hqieSumLaser[i-700][jj][yy] = lasersumplots.make<TH1F>(fname,ftitle,3000,0,3000);

	  sprintf(fname,"TPsMaxLaser_S%.2i_Ch%.2i",jj,yy+1);
	  sprintf(ftitle,"TPs Max Laser Spigot %.2i Channel %.2i",jj,yy+1);
	  htpMaxLaser[i-700][jj][yy] = lasersumplots.make<TH1F>(fname,ftitle,250,0,250);
	}
	
	//LED BCN plots
	sprintf(fname,"BCN_Idle_S%.2i",jj);
	sprintf(ftitle,"BCN Idle for Spigot %.2i",jj);
	hbcnidleLED[i-700][jj] = ledbcnplots.make<TH1F>(fname,ftitle,8,0.5,8.5);
	hbcnidleLED[i-700][jj]->SetXTitle("Fibre Number");
	//Laser BCN plots
	sprintf(fname,"BCN_Idle_S%.2i",jj);
	sprintf(ftitle,"BCN Idle for Spigot %.2i",jj);
	hbcnidleLaser[i-700][jj] = laserbcnplots.make<TH1F>(fname,ftitle,8,0.5,8.5);
	hbcnidleLaser[i-700][jj]->SetXTitle("Fibre Number");
	for (int zz = 1; zz < 7; zz++) {
	  sprintf(fname,"TSmean_BCN_Idle_S%.2i_Fib_%.2i",jj,zz+1);
	  sprintf(ftitle,"TS mean minus BCN Idle for Spigot %.2i and Fibre %.2i",jj,zz+1);
	  hmeanTSminBCNIdleLED[i-700][jj][zz] = ledbcnplots.make<TH1F>(fname,ftitle,200,-(bcnidle_+10),-(bcnidle_-10));
	  sprintf(fname,"BCN_Idle_fib_%i",zz);
	  hbcnIdleLED[i-700][jj][zz] = ledbcnplots.make<TH1F>(fname,fname,100,bcnidle_-50.5,bcnidle_+49.5);
	  hbcnIdleLED[i-700][jj][zz]->SetXTitle("BCN Idle");
	  sprintf(fname,"TS_Mean_Fib_%i",zz+1);
	  sprintf(ftitle,"TS charge weighted mean for Fibre %i",zz+1);
	  hweighttimingmeanled[i-700][jj][zz] = ledbcnplots.make<TH1F>(fname,ftitle,100,-0.1,19.9);
	  hweighttimingmeanled[i-700][jj][zz]->SetXTitle("TS mean");

	  sprintf(fname,"TSmean_BCN_Idle_S%.2i_Fib_%.2i",jj,zz+1);
	  sprintf(ftitle,"TS mean minus BCN Idle for Spigot %.2i and Fibre %.2i",jj,zz+1);
	  hmeanTSminBCNIdleLaser[i-700][jj][zz] = laserbcnplots.make<TH1F>(fname,ftitle,200,-(bcnidle_+10),-(bcnidle_-10));
	  sprintf(fname,"BCN_Idle_fib_%i",zz);
	  hbcnIdleLaser[i-700][jj][zz] = laserbcnplots.make<TH1F>(fname,fname,100,bcnidle_-50.5,bcnidle_+49.5);
	  hbcnIdleLaser[i-700][jj][zz]->SetXTitle("BCN Idle");
	  sprintf(fname,"TS_Mean_Fib_%i",zz+1);
	  sprintf(ftitle,"TS charge weighted mean for Fibre %i",zz+1);
	  hweighttimingmeanlaser[i-700][jj][zz] = laserbcnplots.make<TH1F>(fname,ftitle,100,-0.1,19.9);
	  hweighttimingmeanlaser[i-700][jj][zz]->SetXTitle("TS mean");
	}
      }

      qie_counter   [i-700][13][NCHANS][3]    = 0;
      timing_counterled[i-700][13][NCHANS]    = 0;
      timing_counterlaser[i-700][13][NCHANS]  = 0;
      sum_qie       [i-700][13][NCHANS][3]    = 0;
      sum2_qie      [i-700][13][NCHANS][3]    = 0;
      qie_mean      [i-700][13][NCHANS][3]    = 0;
      qie_sd        [i-700][13][NCHANS][3]    = 0;
      timing_meanled   [i-700][13][NCHANS]    = 0;
      timing2_meanled  [i-700][13][NCHANS]    = 0;
      timing_sdled     [i-700][13][NCHANS]    = 0;
      timing_meanlaser   [i-700][13][NCHANS]  = 0;
      timing2_meanlaser  [i-700][13][NCHANS]  = 0;
      timing_sdlaser     [i-700][13][NCHANS]  = 0;
      counterLED         [13][24][20]         = 0;
      counterLaser       [13][24][20]         = 0;

      //
      // make 1 2D plot per FED of the number of times a QIE channel per spigot had invalid data
      // somewhere in the "n" time samples
      //
      char hname[128];
      sprintf(hname,"Invalid Ch v Spigot FED %d",i);
      hinvalid[i][i-firstFED] = genplots.make<TH2F>(hname,hname,24,0,24,15,0,15);
    }

    if      ( wasPEDTrig )   htrigtype[i-700]->Fill(0.5,1);
    else if ( wasLEDTrig )   htrigtype[i-700]->Fill(1.5,1);
    else if ( wasLaserTrig ) htrigtype[i-700]->Fill(2.5,1);
    //
    // now that we have the number of spigots, we can loop over them and unpack
    // all of the data, and maybe even analyze it
    //
    for (int j=0; j<spigots; j++) {
      //
      // gosh, the DCC data is 32 bit words, but the HTR data is 16 bit words
      // when in Rome...go to 16 bit words. so we can follow the documentation
      //

      unpack_spigot(j);
      //
      // done.  now "hdata" has the data and it will stay that way until it's
      // destroyed (and don't forget to do that!)
      //
      // now you can unpack the HTR header, the QIEs, TPS, "extra", pretrailer,
      // and trailer
      //
      // first, fill values from the header
      //
      if (debugme) printf("spigot %.2i ",j);
      unpack_spigot_header();
      //          printf("spigot = %i, half htr number = %i\n",j,htrn);
      //	  for (int q=0; q<8; q++) cout << hex << hdata[q] << dec << std::endl;
      //
      // now print stuff out
      //
      if (debugme) {
	//
	// the following are all errors if == 1 except for the last one
	//
	printf("                                      ");
	printf("             Samples             OBERLLOCKTHC1\n");
	printf(" Spigot    EvN     BcN   HTR   fw   #TP");
	printf("     Htype  TP  QIE DCCw   HTRw WZELEWDKEMMT? TTC DLL\n");
	printf("   %2d    %4d   %5d  %4d  0x%2.2X   %2d     0x%2.2x   %2d   %2d  %3d    %3d",
	       j,evn,bcn,htrn,fw,ntps,htype,tpsamps,qiesamps,ndccw,nwc);
	printf(" %d%d%d%d%d%d%d%d%d%d%d%d%d  %2d %2d",
	       ow,bz,hee,rl,le,lw,od,ck,be,tmb,hm,ct,one,ttcready,dll);
	if (evn2 == evn1) cout << std::endl;
	else 
	  if (debugme) cout << " ***" << std::endl;
      }
      //
      // now we can look at the QIE data
      // collect all of the qies into arrays
      //
      if (ntps) {
	if (debugme) printf("unpacking spigot tps\n");
	unpack_spigot_tps();
	//printf("TP information for FED_%.2i, spigot %.2i: tpsamples %2i ntps %2i\n",i,j,tpsamps,ntps);
	//            for (int hh = 0; hh < ntps; hh++) {
	//              printf("TP word %i: %x\n",hh,tpinfos[hh]);
	//            }
	//
	// printout the TP data?
	//
	//            print_spigot_tps();
	//
	//get the TP information
	//
	int zpt = 0;
	for( int z = 0; z < ntpch; z++ ) {
	  int tp_max = 0;
	  int sample_max = -1;
	  int nzcount = 0;
	  if (debugme) printf("chan: %.2i : tp word, tptag, tpz, tpsoi, tpfeature, tpval, tp_max, max samp, non-zero, tp word number\n",z );
	  tpinfos_unpack( zpt++ );

	  if ( wasPEDTrig ) {
	    htpsAllPedestal[i-700]->Fill(tpval);
	  }
	  else if ( wasLEDTrig ) {
	    if (tpval>0) nzcount++;
	    htpsAllLED[i-700]->Fill(tpval);

	    htpShapeLED[i-700][j][z]->Fill(0.0,tpval);
	    htpSOIShapeLED[i-700][j][z]->Fill(0.0,tpsoi);
	    htpFeatureShapeLED[i-700][j][z]->Fill(0.0,tpfeature);
	    htpsoiLED[i-700]->Fill(z+.125,j+.5,tpsoi);
	    htpfeatureLED[i-700]->Fill(z+.125,j+.5,tpfeature);
	    htpvalLED[i-700]->Fill(z+.125,j+.5,tpval);

	    htpSummaryShapeLED[i-700][j]->Fill(0.0,tpval);
	    htpSummarySOIShapeLED[i-700][j]->Fill(0.0,tpsoi);
	    htpSummaryFeatureShapeLED[i-700][j]->Fill(0.0,tpfeature);
	    //
	    //                find max TP and put into histogram per channel, check
	    if ( tpval > tp_max ) {
	      tp_max = tpval;
	      sample_max = 0;
	    }
	    //                plot the TS spectrum of the max TP? how
	    //                display the number of non-zero TPs, check
	    //                figure out how to display the SOI in some plot
	    //
	  }
	  else if ( wasLaserTrig ) {
	    if (tpval>0) nzcount++;
	    htpsAllLaser[i-700]->Fill(tpval);

	    htpShapeLaser[i-700][j][z]->Fill(0.0,tpval);
	    htpSOIShapeLaser[i-700][j][z]->Fill(0.0,tpsoi);
	    htpFeatureShapeLaser[i-700][j][z]->Fill(0.0,tpfeature);
	    htpsoiLaser[i-700]->Fill(z+.125,j+.5,tpsoi);
	    htpfeatureLaser[i-700]->Fill(z+.125,j+.5,tpfeature);
	    htpvalLaser[i-700]->Fill(z+.125,j+.5,tpval);

	    htpSummaryShapeLaser[i-700][j]->Fill(0.0,tpval);
	    htpSummarySOIShapeLaser[i-700][j]->Fill(0.0,tpsoi);
	    htpSummaryFeatureShapeLaser[i-700][j]->Fill(0.0,tpfeature);
	    //
	    //                find max TP and put into histogram per channel, check
	    if ( tpval > tp_max ) {
	      tp_max = tpval;
	      sample_max = 0;
	    }
	    //                plot the TS spectrum of the max TP? how
	    //                display the number of non-zero TPs, check
	    //                figure out how to display the SOI in some plot
	    //
	  }
	  if (debugme) printf("samp %2i :   0x%1x   0x%.2x  0x%1x    0x%1x        0x%1x   0x%.2x %7i %9i %9i %15i \n",0,tpinfos[zpt],tptag,tpz,tpsoi,tpfeature,tpval,tp_max,sample_max,nzcount,zpt-1);
	  for ( int p = 1; p < tpsamps; p++ ) {
	    tpinfos_unpack( zpt );
	    if ( wasPEDTrig ) {
	      htpsAllPedestal[i-700]->Fill(tpval);
	    }
	    else if ( wasLEDTrig ) {
	      if (tpval>0) nzcount++;
	      htpsAllLED[i-700]->Fill(tpval);

	      htpShapeLED[i-700][j][z]->Fill(p,tpval);
	      htpSOIShapeLED[i-700][j][z]->Fill(p,tpsoi);
	      htpFeatureShapeLED[i-700][j][z]->Fill(p,tpfeature);
	      htpsoiLED[i-700]->Fill(z+.125+(p/4),j+.5,tpsoi);
	      htpfeatureLED[i-700]->Fill(z+.125+(p/4),j+.5,tpfeature);
	      htpvalLED[i-700]->Fill(z+.125+(p/4),j+.5,tpval);

	      htpSummaryShapeLED[i-700][j]->Fill(p,tpval);
	      htpSummarySOIShapeLED[i-700][j]->Fill(p,tpsoi);
	      htpSummaryFeatureShapeLED[i-700][j]->Fill(p,tpfeature);
	      //
	      //                  find max TP and put into histogram per channel, check
	      if ( tpval > tp_max ) {
		tp_max = tpval;
		sample_max = p;
	      }
	      //                  plot the TS spectrum of the max TP? how
	      //                  display the number of non-zero TPs, check
	      //                  figure out how to display the SOI in some plot
	      //
	    }
	    else if ( wasLaserTrig ) {
	      if (tpval>0) nzcount++;
	      htpsAllLaser[i-700]->Fill(tpval);

	      htpShapeLaser[i-700][j][z]->Fill(p,tpval);
	      htpSOIShapeLaser[i-700][j][z]->Fill(p,tpsoi);
	      htpFeatureShapeLaser[i-700][j][z]->Fill(p,tpfeature);
	      htpsoiLaser[i-700]->Fill(z+.125+(p/4),j+.5,tpsoi);
	      htpfeatureLaser[i-700]->Fill(z+.125+(p/4),j+.5,tpfeature);
	      htpvalLaser[i-700]->Fill(z+.125+(p/4),j+.5,tpval);

	      htpSummaryShapeLaser[i-700][j]->Fill(p,tpval);
	      htpSummarySOIShapeLaser[i-700][j]->Fill(p,tpsoi);
	      htpSummaryFeatureShapeLaser[i-700][j]->Fill(p,tpfeature);
	      //
	      //                  find max TP and put into histogram per channel, check
	      if ( tpval > tp_max ) {
		tp_max = tpval;
		sample_max = p;
	      }
	      //                  plot the TS spectrum of the max TP? how
	      //                  display the number of non-zero TPs, check
	      //                  figure out how to display the SOI in some plot
	      //
	    }
	    if (debugme) printf("samp %2i :   0x%1x   0x%.2x  0x%1x    0x%1x        0x%1x   0x%.2x %7i %9i %9i %15i \n",p,tpinfos[zpt],tptag,tpz,tpsoi,tpfeature,tpval,tp_max,sample_max,nzcount,zpt);
	    zpt++;
	  }
	  htpMaxLED[i-700][j][z]->Fill(tp_max);
	  htpnonzeroLED[i-700]->Fill(z+0.5,j+0.5,nzcount);
	}
      }
	
      //unpack the qie data words
      unpack_spigot_qies();
      if (debugme) printf("unpacking spigot qies\n");
      //
      // printout the QIE data?
      //
      //	  print_spigot_qies();
	
      //
      // look for evidence of bad capids, start with the first and see if the others are ok
      //
      int ipt = 0;
      int ndead = 0;
      int idead[24];
      
      double timingvalBCN[8] = {0,0,0,0,0,0,0,0};
      double integratedchargeBCN[8] = {0,0,0,0,0,0,0,0};

      for (int k=0; k<nqiech; k++) {
	//
	// loop over number of time samples
	//
	//	    cout << " QIE channel " << k+1 << " pointer is " << ipt << std::endl;
	qies_unpack(ipt++);
	//	    cout << " capid(0) " << capid << " mant/exp " << mant << "/" << range << std::endl;
	int cap_old = capid;
	bool capid_error = false;
	int all = 0;
	int qiemax[2];
	qiemax[0] = qiemax[1]  = -1;
	int qie2max[2];
	qie2max[0] = qie2max[1] = -1;
	int qiesum = 0;
	bool iambad = false;
	//
	// to emulate the ZS we have to sum over 2.  which means we have to drop the 1st QIE sum over the previous one
	//
	int qie_prev[2];
	qie_prev[0] = qie_prev[1] = 0;
	int qie2[2];
	  
	double timingvalled = 0;
	double integratedchargeled = 0;
	double timingvallaser = 0;
	double integratedchargelaser = 0;

  
	//
	//i think that i need to do a sum here for the first qie sample
	//
	if ( (dv==1) & (er==0) ) {
	  //
	  // only make plots of QIE if the channel has valid data
	  //
	  if (wasPEDTrig) {
	    qiesum += qie2fC[range][mant];
	    hqieShapePedestal[i-700][j][channel-1]->Fill(0.5,qie2fC[range][mant]);
	    hqieAllPedestal[i-700]->Fill(qie2fC[range][mant]+.5);

	    if (!sprinkle) {
	      hqieNoSPedestal[i-700]->Fill(qie2fC[range][mant]+.5);
	    }
	    sum_qie[i-700][j][channel-1][0] += qie2fC[range][mant];
	    sum2_qie[i-700][j][channel-1][0] += (qie2fC[range][mant]*qie2fC[range][mant]);
	    qie_counter[i-700][j][channel-1][0]+=1;
	      
	    //
	    // now do the emulation.  only odd values of "l" are used to fill the histogram
	    //
	      
	    if (qie > qiemax[0]) qiemax[0] = qie;
	      
	  }
	  else if (wasLEDTrig) {
	    counterLED[j][channel-1][0]++;
	    qiesum += qie2fC[range][mant];
	    //                hqieShapeLED[i-700][j][channel-1]->Fill(0.5,qie2fC[range][mant]);
	    hqieShapeLED[i-700][j][channel-1]->Fill(0.0,qie2fC[range][mant]);
	    hqieAllLED[i-700]->Fill(qie2fC[range][mant]+.5);
	    if (!sprinkle) {
	      hqieNoSLED[i-700]->Fill(qie2fC[range][mant]+.5);
	    }
	      
	    if (qie > qiemax[1]) qiemax[1] = qie;
	      
	    sum_qie[i-700][j][channel-1][1] += qie2fC[range][mant];
	    sum2_qie[i-700][j][channel-1][1] += (qie2fC[range][mant]*qie2fC[range][mant]);
	    qie_counter[i-700][j][channel-1][1]+=1;
	    timingvalled += 0*(qie2fC[range][mant]);
	    integratedchargeled += qie2fC[range][mant]; 

	    if (debugme) cout<<"fib: "<<fib<<"  range: "<<range<<"  mant: "<<mant<<"  qie2fC: "<<qie2fC[range][mant];
	    timingvalBCN[fib] += 0*(qie2fC[range][mant]);
	    integratedchargeBCN[fib] += qie2fC[range][mant]; 
	    if (debugme) cout<<"  weighted TS: "<<timingvalBCN[fib]<<"  charge summed: "<<integratedchargeBCN[fib]<<std::endl;
	  }

	  else if (wasLaserTrig) {
	    counterLaser[j][channel-1][0]++;
	    qiesum += qie2fC[range][mant];
	    //                hqieShapeLaser[i-700][j][channel-1]->Fill(0.5,qie2fC[range][mant]);
	    hqieShapeLaser[i-700][j][channel-1]->Fill(0.0,qie2fC[range][mant]);
	    hqieAllLaser[i-700]->Fill(qie2fC[range][mant]+.5);
	    if (!sprinkle) {
	      hqieNoSLaser[i-700]->Fill(qie2fC[range][mant]+.5);
	    }
	      
	    if (qie > qiemax[1]) qiemax[1] = qie;
	      
	    sum_qie[i-700][j][channel-1][2] += qie2fC[range][mant];
	    sum2_qie[i-700][j][channel-1][2] += (qie2fC[range][mant]*qie2fC[range][mant]);
	    qie_counter[i-700][j][channel-1][2]+=1;
	    timingvallaser += 0*(qie2fC[range][mant]);
	    integratedchargelaser += qie2fC[range][mant]; 

	    if (debugme) cout<<"fib: "<<fib<<"  range: "<<range<<"  mant: "<<mant<<"  qie2fC: "<<qie2fC[range][mant];
	    timingvalBCN[fib] += 0*(qie2fC[range][mant]);
	    integratedchargeBCN[fib] += qie2fC[range][mant]; 
	    if (debugme) cout<<"  weighted TS: "<<timingvalBCN[fib]<<"  charge summed: "<<integratedchargeBCN[fib]<<std::endl;
	  }
	    
	}
	else {
	  //
	  // flag this channel as being bad and count it in the 2d "hinvalid" histograms
	  //
	  iambad = true;
	}
	  
	for (int l=1; l<qiesamps; l++) {
	  qies_unpack(ipt);
	  if (debugme) cout << " capid(" << l << ") " << capid;
	  int cap_new = cap_old + 1;
	  if (cap_new == 4) cap_new = 0;
	  if (debugme) {
	    cout << " cap_old is " << cap_old << " and cap_new is " << cap_new;
	    cout << " mant/exp " << mant << "/" << range << std::endl;
	  }
	  //
	  // check if data is valid and error flag is zero
	  //
	  if ( (dv==1) & (er==0) ) {
	    //
	    // only make plots of QIE if the channel has valid data
	    //
	    if (wasPEDTrig) {
	      qiesum += qie2fC[range][mant];
	      hqieShapePedestal[i-700][j][channel-1]->Fill(l+0.5,qie2fC[range][mant]);
	      hqieAllPedestal[i-700]->Fill(qie2fC[range][mant]+.5);

	      if (!sprinkle) {
		hqieNoSPedestal[i-700]->Fill(qie2fC[range][mant]+.5);
	      }
	      sum_qie[i-700][j][channel-1][0] += qie2fC[range][mant];
	      sum2_qie[i-700][j][channel-1][0] += (qie2fC[range][mant]*qie2fC[range][mant]);
	      qie_counter[i-700][j][channel-1][0]+=1;
		
	      //
	      // now do the emulation.  only odd values of "l" are used to fill the histogram
	      //
		
	      if (qie > qiemax[0]) qiemax[0] = qie;
	      qie2[0] = qie + qie_prev[0];
	      qie_prev[0] = qie;
		
	      if (l > 0) {
		if (qie2[0] > qie2max[0]) qie2max[0] = qie2[0];
	      }
	    }
	    else if (wasLEDTrig) {
	      counterLED[j][channel-1][l]++;
	      qiesum += qie2fC[range][mant];
	      //                  hqieShapeLED[i-700][j][channel-1]->SetBinContent(l+1,qie2fC[range][mant]);
	      //                  hqieShapeLED[i-700][j][channel-1]->Fill(l+0.5,qie2fC[range][mant]);
	      hqieShapeLED[i-700][j][channel-1]->Fill(l+0.0,qie2fC[range][mant]);
	      hqieAllLED[i-700]->Fill(qie2fC[range][mant]+.5);
	      if (!sprinkle) {
		hqieNoSLED[i-700]->Fill(qie2fC[range][mant]+.5);
	      }
		
	      if (qie > qiemax[1]) qiemax[1] = qie;
	      qie2[1] = qie + qie_prev[1];
	      qie_prev[1] = qie;
	      if (l > 0) {
		if (qie2[1] > qie2max[1]) qie2max[1] = qie2[1];
	      }
		
	      sum_qie[i-700][j][channel-1][1] += qie2fC[range][mant];
	      sum2_qie[i-700][j][channel-1][1] += (qie2fC[range][mant]*qie2fC[range][mant]);
	      qie_counter[i-700][j][channel-1][1]+=1;
	      timingvalled += (l*qie2fC[range][mant]);
	      integratedchargeled += qie2fC[range][mant]; 

	      if (debugme) cout<<"fib: "<<fib<<"  range: "<<range<<"  mant: "<<mant<<"  qie2fC: "<<qie2fC[range][mant];
	      timingvalBCN[fib] += (l*qie2fC[range][mant]);
	      integratedchargeBCN[fib] += qie2fC[range][mant]; 
	      if (debugme) cout<<"  weighted TS: "<<timingvalBCN[fib]<<"  charge summed: "<<integratedchargeBCN[fib]<<std::endl;
	    }
	      
	    else if (wasLaserTrig) {
	      counterLaser[j][channel-1][l]++;
	      qiesum += qie2fC[range][mant];
	      //                  hqieShapeLaser[i-700][j][channel-1]->SetBinContent(l+1,qie2fC[range][mant]);
	      //                  hqieShapeLaser[i-700][j][channel-1]->Fill(l+0.5,qie2fC[range][mant]);
	      hqieShapeLaser[i-700][j][channel-1]->Fill(l+0.0,qie2fC[range][mant]);
	      hqieAllLaser[i-700]->Fill(qie2fC[range][mant]+.5);
	      if (!sprinkle) {
		hqieNoSLaser[i-700]->Fill(qie2fC[range][mant]+.5);
	      }
		
	      if (qie > qiemax[1]) qiemax[1] = qie;
	      qie2[1] = qie + qie_prev[1];
	      qie_prev[1] = qie;
	      if (l > 0) {
		if (qie2[1] > qie2max[1]) qie2max[1] = qie2[1];
	      }
		
	      sum_qie[i-700][j][channel-1][2] += qie2fC[range][mant];
	      sum2_qie[i-700][j][channel-1][2] += (qie2fC[range][mant]*qie2fC[range][mant]);
	      qie_counter[i-700][j][channel-1][2]+=1;
	      timingvallaser += (l*qie2fC[range][mant]);
	      integratedchargelaser += qie2fC[range][mant]; 

	      if (debugme) cout<<"fib: "<<fib<<"  range: "<<range<<"  mant: "<<mant<<"  qie2fC: "<<qie2fC[range][mant];
	      timingvalBCN[fib] += (l*qie2fC[range][mant]);
	      integratedchargeBCN[fib] += qie2fC[range][mant]; 
	      if (debugme) cout<<"  weighted TS: "<<timingvalBCN[fib]<<"  charge summed: "<<integratedchargeBCN[fib]<<std::endl;
	    }
	      
	    /*
	      if (l > 0) {
	      if (qie2 > qie2max) qie2max = qie2;
	      }
	    */
	  }
	  else {
	    //
	    // flag this channel as being bad and count it in the 2d "hinvalid" histograms
	    //
	    iambad = true;
	  }
	  //
	  //	      cout<<"check that capid's are rotating nicely"<<std::endl;
	  //
	  if (cap_new != capid) {
	    capid_error = true;
	    hcapiderr[i-700][j]->Fill(channel,1);
	    if (capid == cap_old) all++;
	    if (debugme) cout << " Fed " << i << " Spigot " << j << " QIE " << k+1 << " TS " << l << " CAPID error!  previous capid " << cap_old << " new capid " << capid << std::endl;
	  }
	  cap_old = capid;
	  ipt++;
	}
	  
	if (wasPEDTrig) {
	  if (qiemax[0] > 0) hqieMAXPedestal[i-700]->Fill(qiemax[0]+.5);
	  if (qie2max[0] > 0) hqieZSPedestal[i-700]->Fill(qie2max[0]+.5);
	  if (qiesum > 0) hqieSumPedestal[i-700][j][channel-1]->Fill(qiesum);
	}
	if (wasLEDTrig) {
	  if (integratedchargeled>0) {
	    timing_meanled[i-700][j][channel-1] += (timingvalled/integratedchargeled);
	    timing2_meanled[i-700][j][channel-1] += ((timingvalled/integratedchargeled)*(timingvalled/integratedchargeled));
	    timing_counterled[i-700][j][channel-1]++;
	    if (qiesum > 0) hqieSumLED[i-700][j][channel-1]->Fill(qiesum);
	  }
	}

	if (wasLaserTrig) {
	  if (integratedchargelaser>0) {
	    timing_meanlaser[i-700][j][channel-1] += (timingvallaser/integratedchargelaser);
	    timing2_meanlaser[i-700][j][channel-1] += ((timingvallaser/integratedchargelaser)*(timingvallaser/integratedchargelaser));
	    timing_counterlaser[i-700][j][channel-1]++;
	    if (qiesum > 0) hqieSumLaser[i-700][j][channel-1]->Fill(qiesum);
	  }
	}
	  
//	fs->cd();
//	sprintf(fname,"FED_%.2i/General",i);
//	fs->cd(fname);
	  
	if (iambad) hinvalid[i][i-firstFED]->Fill(k+.5,j+.5);
	if (capid_error&&debugme) {
	  all++;
	  if (all == qiesamps) {
	    idead[ndead++] = fib+1;
	  }
	  else printf("   \033[1m CAPID errors!!!\033[m FED %d spigot %d QIE %d\n",i,j,k);
	  //	      cout << "all " << all << std::endl;
	}  
	else if (debugme) printf("   \033[1m NO CAPID errors!!!\033[m\n");
      }//unpack_spigot_qies

      //now unpack the extra information
      if (wasLEDTrig) {
	if (unpack_spigot_extra()) {
	  for (int nfibs=1; nfibs<7;nfibs++){

	    double QTSMean = 0;
	    extra_unpack(nfibs);
	    if (usbit==0) {
	      extra_norm_unpack(nfibs);
	      if (evn!=evnprev) {
		hbcnidleLED[i-700][j]->Fill(nfibs+1,arrivebcn);
		hbcnIdleLED[i-700][j][nfibs]->Fill(arrivebcn);
		hbcnidleLED2D[i-700]->Fill(nfibs+1,j,arrivebcn);
	      }
	      if (integratedchargeBCN[nfibs]!=0) QTSMean = timingvalBCN[nfibs]/integratedchargeBCN[nfibs];
	      hweighttimingmeanled[i-700][j][nfibs]->Fill(QTSMean);
	      
	      hmeanTSminBCNIdleLED[i-700][j][nfibs]->Fill(QTSMean-arrivebcn);
	      
	      //	    if((QTSMean-arrivebcn)<-100||(QTSMean-arrivebcn)>-90 ) {
	      if (debugme) printf("fibre: %d  Weighted TS sum %d, integrated charge %d, TS mean %2.2f (%2.2f), bcn_idle %d, TS mean - bcn_idle %2.2f\n",nfibs,(int)timingvalBCN[nfibs],(int)integratedchargeBCN[nfibs],timingvalBCN[nfibs]/integratedchargeBCN[nfibs],QTSMean,arrivebcn,(QTSMean-arrivebcn));
	      //	    }
	    }
	  }
	  if (usbit==1) {
	    extra_unsup_unpack();
	    // no histograms yet
	  }
	}
	else printf("Spigot %.2i tagged event %i compact, EE tag %i\n",j,evn,ee);//event compact, no information stored
      }

      if (wasLaserTrig) {
	if (unpack_spigot_extra()) {
	  for (int nfibs=1; nfibs<7;nfibs++){

	    double QTSMean = 0;
	    extra_unpack(nfibs);
	    if (usbit==0) {
	      extra_norm_unpack(nfibs);
	      if (evn!=evnprev) {
		hbcnidleLaser[i-700][j]->Fill(nfibs+1,arrivebcn);
		hbcnIdleLaser[i-700][j][nfibs]->Fill(arrivebcn);
		hbcnidleLaser2D[i-700]->Fill(nfibs+1,j,arrivebcn);
	      }
	      if (integratedchargeBCN[nfibs]!=0) QTSMean = timingvalBCN[nfibs]/integratedchargeBCN[nfibs];
	      hweighttimingmeanlaser[i-700][j][nfibs]->Fill(QTSMean);
	      
	      hmeanTSminBCNIdleLaser[i-700][j][nfibs]->Fill(QTSMean-arrivebcn);
	      
	      //	    if((QTSMean-arrivebcn)<-100||(QTSMean-arrivebcn)>-90 ) {
	      if (debugme) printf("fibre: %d  Weighted TS sum %d, integrated charge %d, TS mean %2.2f (%2.2f), bcn_idle %d, TS mean - bcn_idle %2.2f\n",nfibs,(int)timingvalBCN[nfibs],(int)integratedchargeBCN[nfibs],timingvalBCN[nfibs]/integratedchargeBCN[nfibs],QTSMean,arrivebcn,(QTSMean-arrivebcn));
	      //	    }
	    }
	  }
	  if (usbit==1) {
	    extra_unsup_unpack();
	    // no histograms yet
	  }
	}
	else printf("Spigot %.2i tagged event %i compact, EE tag %i\n",j,evn,ee);//event compact, no information stored
      }
      //unpack_spigot_extra

      //
      // end of loop over fibers in this spigot
      //
      if (ndead > 0&&debugme) {
	printf("   \033[1m CAPID errors\033[m FED %d spigot %d, all capids are the same on %d fibers:  ",i,j,ndead/3);
	for (int f=0; f<ndead; f=f+3) cout << idead[f] << " ";
	cout << std::endl;
      }
      //
      // get rid of what you booked before....
      //
      delete [] qies;
      delete [] hdata;
      //
      // want to go to the next FED?
      //
      //	  if (debugme) exit(1);

    }//done with spigot loop
    evnprev=evn;
  }   
  if (singleMode) {
    int temp;
    if (!singleModeCancel) {
      cout << "enter any number to continue, -1 to free run: ";
      cin >> temp;
      if (temp == -1) singleModeCancel = true;
      cout << std::endl;
    }
  }
  checker = false;
  //   }//end while loop
}


// ------------ method called once each job just before starting event loop  ------------
void 
KPlotter::beginJob(const edm::EventSetup&)
{
  cout<<"Begin KPlotter job...   "<<std::endl;
}
// ------------ method called once each job just after ending the event loop  ------------
void 
KPlotter::endJob() {
  for (int ll = 700; ll < 733; ll++) {
    if(!first[ll-700]) {
      for (int nn = 0; nn < numspigs[ll-700]; nn++) {
	for (int mm = 0; mm < NCHANS; mm++) {
	  for (int tt = 0; tt < 20; tt++) {
	    double norm = hqieShapePedestal[ll-700][nn][mm]->GetEntries()/qiesamps;
	    hqieShapePedestal[ll-700][nn][mm]->SetMarkerColor(kBlue);
	    hqieShapePedestal[ll-700][nn][mm]->SetMarkerSize(1);
	    hqieShapePedestal[ll-700][nn][mm]->SetBinContent(tt+1,hqieShapePedestal[ll-700][nn][mm]->GetBinContent(tt+1)/norm);
	    //          hqieShapePedestal[ll-700][nn][mm]->SetBinError(tt+1,0.0001);
	  }
	  for (int tt = 0; tt < 20; tt++) {
	    double norm = hqieShapeLED[ll-700][nn][mm]->GetEntries()/qiesamps;
	    hqieShapeLED[ll-700][nn][mm]->SetMarkerColor(kBlue);
	    hqieShapeLED[ll-700][nn][mm]->SetMarkerSize(1);
	    hqieShapeLED[ll-700][nn][mm]->SetBinContent(tt+1,hqieShapeLED[ll-700][nn][mm]->GetBinContent(tt+1)/norm);
	    //            hqieShapeLED[ll-700][nn][mm]->SetBinError(tt+1,0.0001);
	  }
	  for (int tt = 0; tt < 20; tt++) {
	    double norm = hqieShapeLaser[ll-700][nn][mm]->GetEntries()/qiesamps;
	    hqieShapeLaser[ll-700][nn][mm]->SetMarkerColor(kBlue);
	    hqieShapeLaser[ll-700][nn][mm]->SetMarkerSize(1);
	    hqieShapeLaser[ll-700][nn][mm]->SetBinContent(tt+1,hqieShapeLaser[ll-700][nn][mm]->GetBinContent(tt+1)/norm);
	    //            hqieShapeLaser[ll-700][nn][mm]->SetBinError(tt+1,0.0001);
	  }

	  if (qie_counter[ll-700][nn][mm][0]>0) {
	    double meanqieped = 1.0*sum_qie[ll-700][nn][mm][0]/qie_counter[ll-700][nn][mm][0];
	    double sdqieped = sqrt(((1.0*sum2_qie[ll-700][nn][mm][0])/qie_counter[ll-700][nn][mm][0]) - (meanqieped*meanqieped));
	    //            hpedestalrms[ll-700][nn]->Draw("p");
	    hpedestalrms[ll-700][nn]->SetMarkerStyle(8);
	    hpedestalrms[ll-700][nn]->SetMarkerColor(kBlue);
	    hpedestalrms[ll-700][nn]->SetMarkerSize(1);
	    hpedestalrms[ll-700][nn]->SetXTitle("Channel Number");
	    hpedestalrms[ll-700][nn]->SetBinContent(mm+2,sdqieped);
	    hpedestalrms[ll-700][nn]->SetBinError(mm+2,0.001);

	    //            hpedestal[ll-700][nn]->Draw("p");
	    hpedestal[ll-700][nn]->SetMarkerStyle(8);
	    hpedestal[ll-700][nn]->SetMarkerColor(kBlue);
	    hpedestal[ll-700][nn]->SetMarkerSize(1);
	    hpedestal[ll-700][nn]->SetXTitle("Channel Number");
	    hpedestal[ll-700][nn]->SetBinContent(mm+2,meanqieped);
	    hpedestal[ll-700][nn]->SetBinError(mm+2,sdqieped);

	  }
	  //          hqiemeanPedestal[ll-700][nn]->Draw("p");
	  hqiemeanPedestal[ll-700][nn]->SetMarkerStyle(8);
	  hqiemeanPedestal[ll-700][nn]->SetMarkerColor(kBlue);
	  hqiemeanPedestal[ll-700][nn]->SetMarkerSize(1);
	  hqiemeanPedestal[ll-700][nn]->SetXTitle("Channel Number");
	  hqiemeanPedestal[ll-700][nn]->SetBinContent(mm+2,hqieSumPedestal[ll-700][nn][mm]->GetMean());
	  hqiemeanPedestal[ll-700][nn]->SetBinError(mm+2,hqieSumPedestal[ll-700][nn][mm]->GetRMS());


	  if (timing_counterled[ll-700][nn][mm]>0) {
	    double meantime = timing_meanled[ll-700][nn][mm]/timing_counterled[ll-700][nn][mm];
	    double sdtime = sqrt((timing2_meanled[ll-700][nn][mm]/timing_counterled[ll-700][nn][mm]) - (meantime*meantime));

	    //            htimingdev[ll-700][nn]->Draw("p");
	    htimingdevled[ll-700][nn]->SetMarkerStyle(8);
	    htimingdevled[ll-700][nn]->SetMarkerColor(kRed);
	    htimingdevled[ll-700][nn]->SetMarkerSize(1);
	    htimingdevled[ll-700][nn]->SetXTitle("Channel Number");
	    htimingdevled[ll-700][nn]->SetBinContent(mm+2,sdtime);
	    htimingdevled[ll-700][nn]->SetBinError(mm+2,0.001);
	    //htimingdevled[ll-700][nn]->SetBinContent(mm+2,hqiemeanLED[ll-700][nn][mm]->GetRMS());
	    //htimingdevled[ll-700][nn]->SetBinError(mm+2,0.001);

	    //            htimingmean[ll-700][nn]->Draw("p");
	    htimingmeanled[ll-700][nn]->SetMarkerStyle(8);
	    htimingmeanled[ll-700][nn]->SetMarkerColor(kBlue);
	    htimingmeanled[ll-700][nn]->SetMarkerSize(1);
	    htimingmeanled[ll-700][nn]->SetXTitle("Channel Number");
	    htimingmeanled[ll-700][nn]->SetBinContent(mm+2,meantime);
	    htimingmeanled[ll-700][nn]->SetBinError(mm+2,sdtime);
	    //            htimingmeanled[ll-700][nn]->SetBinContent(mm+2,hqiemeanLED[ll-700][nn][mm]->GetMean());
	    //            htimingmeanled[ll-700][nn]->SetBinError(mm+2,sdtime);
	  }
	  //          hqiemeanLED[ll-700][nn]->Draw("p");
	  hqiemeanLED[ll-700][nn]->SetMarkerStyle(8);
	  hqiemeanLED[ll-700][nn]->SetMarkerColor(kBlue);
	  hqiemeanLED[ll-700][nn]->SetMarkerSize(1);
	  hqiemeanLED[ll-700][nn]->SetBinContent(mm+2,hqieSumLED[ll-700][nn][mm]->GetMean());
	  hqiemeanLED[ll-700][nn]->SetBinError(mm+2,hqieSumLED[ll-700][nn][mm]->GetRMS());

	  htpmeanLED[ll-700][nn]->SetMarkerStyle(8);
	  htpmeanLED[ll-700][nn]->SetMarkerColor(kBlue);
	  htpmeanLED[ll-700][nn]->SetMarkerSize(1);
	  htpmeanLED[ll-700][nn]->SetBinContent(mm+2,htpMaxLED[ll-700][nn][mm]->GetMean());
	  htpmeanLED[ll-700][nn]->SetBinError(mm+2,htpMaxLED[ll-700][nn][mm]->GetRMS());

	  htprmsLED[ll-700][nn]->SetMarkerStyle(8);
	  htprmsLED[ll-700][nn]->SetMarkerColor(kBlue);
	  htprmsLED[ll-700][nn]->SetMarkerSize(1);
	  htprmsLED[ll-700][nn]->SetBinContent(mm+2,htpMaxLED[ll-700][nn][mm]->GetRMS());
	  htprmsLED[ll-700][nn]->SetBinError(mm+2,0.00001);

	  if (timing_counterlaser[ll-700][nn][mm]>0) {
	    double meantime = timing_meanlaser[ll-700][nn][mm]/timing_counterlaser[ll-700][nn][mm];
	    double sdtime = sqrt((timing2_meanlaser[ll-700][nn][mm]/timing_counterlaser[ll-700][nn][mm]) - (meantime*meantime));

	    //            htimingdev[ll-700][nn]->Draw("p");
	    htimingdevlaser[ll-700][nn]->SetMarkerStyle(8);
	    htimingdevlaser[ll-700][nn]->SetMarkerColor(kRed);
	    htimingdevlaser[ll-700][nn]->SetMarkerSize(1);
	    htimingdevlaser[ll-700][nn]->SetXTitle("Channel Number");
	    htimingdevlaser[ll-700][nn]->SetBinContent(mm+2,sdtime);
	    htimingdevlaser[ll-700][nn]->SetBinError(mm+2,0.001);
	    //htimingdevlaser[ll-700][nn]->SetBinContent(mm+2,hqiemeanLaser[ll-700][nn][mm]->GetRMS());
	    //htimingdevlaser[ll-700][nn]->SetBinError(mm+2,0.001);

	    //            htimingmean[ll-700][nn]->Draw("p");
	    htimingmeanlaser[ll-700][nn]->SetMarkerStyle(8);
	    htimingmeanlaser[ll-700][nn]->SetMarkerColor(kBlue);
	    htimingmeanlaser[ll-700][nn]->SetMarkerSize(1);
	    htimingmeanlaser[ll-700][nn]->SetXTitle("Channel Number");
	    htimingmeanlaser[ll-700][nn]->SetBinContent(mm+2,meantime);
	    htimingmeanlaser[ll-700][nn]->SetBinError(mm+2,sdtime);
	    //            htimingmeanlaser[ll-700][nn]->SetBinContent(mm+2,hqiemeanLaser[ll-700][nn][mm]->GetMean());
	    //            htimingmeanlaser[ll-700][nn]->SetBinError(mm+2,sdtime);
	  }
	  //          hqiemeanLaser[ll-700][nn]->Draw("p");
	  hqiemeanLaser[ll-700][nn]->SetMarkerStyle(8);
	  hqiemeanLaser[ll-700][nn]->SetMarkerColor(kBlue);
	  hqiemeanLaser[ll-700][nn]->SetMarkerSize(1);
	  hqiemeanLaser[ll-700][nn]->SetBinContent(mm+2,hqieSumLaser[ll-700][nn][mm]->GetMean());
	  hqiemeanLaser[ll-700][nn]->SetBinError(mm+2,hqieSumLaser[ll-700][nn][mm]->GetRMS());

	  htpmeanLaser[ll-700][nn]->SetMarkerStyle(8);
	  htpmeanLaser[ll-700][nn]->SetMarkerColor(kBlue);
	  htpmeanLaser[ll-700][nn]->SetMarkerSize(1);
	  htpmeanLaser[ll-700][nn]->SetBinContent(mm+2,htpMaxLaser[ll-700][nn][mm]->GetMean());
	  htpmeanLaser[ll-700][nn]->SetBinError(mm+2,htpMaxLaser[ll-700][nn][mm]->GetRMS());

	  htprmsLaser[ll-700][nn]->SetMarkerStyle(8);
	  htprmsLaser[ll-700][nn]->SetMarkerColor(kBlue);
	  htprmsLaser[ll-700][nn]->SetMarkerSize(1);
	  htprmsLaser[ll-700][nn]->SetBinContent(mm+2,htpMaxLaser[ll-700][nn][mm]->GetRMS());
	  htprmsLaser[ll-700][nn]->SetBinError(mm+2,0.00001);
	}
      }
    }
  }
//  fs->Write();
}

//define this as a plug-in
DEFINE_FWK_MODULE(KPlotter);
