{
  gStyle->SetOptStat(111111111);

  char fname[128];
  char ftitle[128];

  int runno;
  int fednum;
  int spigin;

  cout<<"Please enter the run number: ";
  cin>>runno;
/*
  cout<<"To run only plots for a specific FED/Spigot combination enter them below, for all enter -1:"<<endl;
  cout<<"FED (700-732): ";
  cin>>fednum;
  cout<<"Spigot (0-17): ";
  cin>>spigin;

  if ( fednum < 0 ) {
    const int STARTFED = 0;
    const int MAXFED   = 1;
    const int NUMFEDS  = 1; //maximum number of feds
  }
  else {
    const int STARTFED = fednum - 700;
    const int MAXFED   = fednum - 700 + 1;
    const int NUMFEDS  = 1;
  }
  if ( spigin < 0 ) {
    const int STARTSPIG = 0;
    const int MAXSPIG   = 6;
    const int NUMSPIGS  = 6; //maximum number of spigots
  }
  else {
    const int STARTSPIG = spigin;
    const int MAXSPIG   = spigin + 1;
    const int NUMSPIGS  = 1;
  }
*/

  const int STARTFED = 0;
  const int MAXFED   = 1;
  const int NUMFEDS  = 1;

  const int STARTSPIG = 0;
  const int MAXSPIG   = 6;
  const int NUMSPIGS  = 6;

  TFile* fs;

  TDirectory* workingdir;
  TDirectory* workingfeddir;
  TDirectory* workinggendir;
  TDirectory* workingspigdir;
  TDirectory* workingleddir;
  TDirectory* workingpeddir;
  TDirectory* workingshapedir;
  TDirectory* workingsumdir;

  TCanvas* ledshapes1[NUMFEDS][NUMSPIGS];
  TCanvas* ledshapes2[NUMFEDS][NUMSPIGS];

  TCanvas* ledsum1[NUMFEDS][NUMSPIGS];
  TCanvas* ledsum2[NUMFEDS][NUMSPIGS];

  int cans;
  (NUMSPIGS%6==0) ? cans = NUMSPIGS/6 : cans = NUMSPIGS/6 + 1;

  TCanvas* pedqiemean[NUMFEDS][cans];
  TCanvas* pedqierms[NUMFEDS][cans];
  TCanvas* ledtimemean[NUMFEDS][cans];
  TCanvas* ledtimerms[NUMFEDS][cans];
  TCanvas* lasertimemean[NUMFEDS][cans];
  TCanvas* lasertimerms[NUMFEDS][cans];
  TCanvas* invalidcaps[NUMFEDS][cans];
  TCanvas* invalidcapsall[NUMFEDS];

  sprintf(fname,"/scratch/home/daq/sturdy/CMSSW_3_3_0/src/tools/KPlotter/test/Run_%6.6d.root",runno);
  fs = new TFile(fname,"read");

  sprintf(fname,"kplotter");
  if( fs->cd(fname) ) workingdir = (TDirectory*)fs->GetDirectory(fname);
  else continue; 

  for (int jj = STARTFED; jj < MAXFED; jj++) {

    sprintf(fname,"kplotter/FED_7%.2i",jj);
    if ( fs->cd(fname) ) {
      workingfeddir = (TDirectory*)fs->GetDirectory(fname);
      sprintf(fname,"kplotter/FED_7%.2i/General",jj);
      workinggendir = (TDirectory*)fs->GetDirectory(fname);
      fs->cd();

      TH1F* hpedqiemean;
      TH1F* hpedqierms;
      TH1F* hledtimemean;
      TH1F* hledtimerms;
      TH1F* hlasertimemean;
      TH1F* hlasertimerms;
      TH1F* hspigcapiderrs;
      TH2F* hfedcapiderrs;

      for (int hh = STARTSPIG; hh < MAXSPIG; hh++) {
        sprintf(fname,"kplotter/FED_7%.2i/Spigot %.2i",jj,hh);
        if( fs->cd(fname) ) workingspigdir = (TDirectory*)fs->GetDirectory(fname);
        else continue; 

        sprintf(fname,"kplotter/FED_7%.2i/Spigot %.2i/Pedestal Plots",jj,hh);
        if( fs->cd(fname) ) workingpeddir = (TDirectory*)fs->GetDirectory(fname);

        sprintf(fname,"kplotter/FED_7%.2i/Spigot %.2i/LED Plots",jj,hh);
        if( fs->cd(fname) ) workingleddir = (TDirectory*)fs->GetDirectory(fname);

        sprintf(fname,"kplotter/FED_7%.2i/Spigot %.2i/Laser Plots",jj,hh);
        if( fs->cd(fname) ) workinglaserdir = (TDirectory*)fs->GetDirectory(fname);

        workingspigdir->cd();

//LED type canvases

        sprintf(fname,"kplotter/FED_7%.2i/Spigot %.2i/LED Plots/Shape",jj,hh);
        if( fs->cd(fname) ) workingshapedir = (TDirectory*)fs->GetDirectory(fname);

        sprintf(fname,"f7%.2ispig%.2ip1ledshape",jj,hh);
        sprintf(ftitle,"LED Pulse shapes FED 7%.2i Spigot %.2i - Channels 1 - 12",jj,hh);
        ledshapes1[jj][hh] = new TCanvas(fname,ftitle);
        ledshapes1[jj][hh]->Divide(4,3);

        sprintf(fname,"f7%.2ispig%.2ip2ledshape",jj,hh);
        sprintf(ftitle,"LED Pulse shapes FED 7%.2i Spigot %.2i - Channels 13 - 24",jj,hh);
        ledshapes2[jj][hh] = new TCanvas(fname,ftitle);
        ledshapes2[jj][hh]->Divide(4,3);

        sprintf(fname,"kplotter/FED_7%.2i/Spigot %.2i/LED Plots/Sum",jj,hh);
        if( fs->cd(fname) ) workingsumdir = (TDirectory*)fs->GetDirectory(fname);

        sprintf(fname,"f7%.2ispig%.2ip1ledsum",jj,hh);
        sprintf(ftitle,"LED Pulse QIE sum FED 7%.2i Spigot %.2i - Channels 1 - 12",jj,hh);
        ledsum1[jj][hh] = new TCanvas(fname,ftitle);
        ledsum1[jj][hh]->Divide(4,3);

        sprintf(fname,"f7%.2ispig%.2ip2ledsum",jj,hh);
        sprintf(ftitle,"LED Pulse QIE sum FED 7%.2i Spigot %.2i - Channels 13 - 24",jj,hh);
        ledsum2[jj][hh] = new TCanvas(fname,ftitle);
        ledsum2[jj][hh]->Divide(4,3);
      }

      for (int kk = 0; kk < cans; kk++) {
        sprintf(fname,"f7%.2ip%ipedmean",jj,kk+1);
        sprintf(ftitle,"Pedestal mean FED 7%.2i Spigots %.2i - %.2i",jj,(kk)*6,kk*6+5);
        pedqiemean[jj][kk] = new TCanvas(fname,ftitle);
        pedqiemean[jj][kk]->Divide(3,2);

        sprintf(fname,"f7%.2ip%ipedrms",jj,kk+1);
        sprintf(ftitle,"Pedestal RMS FED 7%.2i Spigots %.2i - %.2i",jj,(kk)*6,kk*6+5);
        pedqierms[jj][kk] = new TCanvas(fname,ftitle);
        pedqierms[jj][kk]->Divide(3,2);

        sprintf(fname,"f7%.2ip%iledtimemean",jj,kk+1);
        sprintf(ftitle,"LED timing mean FED 7%.2i Spigots %.2i - %.2i",jj,(kk)*6,kk*6+5);
        ledtimemean[jj][kk] = new TCanvas(fname,ftitle);
        ledtimemean[jj][kk]->Divide(3,2);

        sprintf(fname,"f7%.2ip%iledtimerms",jj,kk+1);
        sprintf(ftitle,"LED timing RMS FED 7%.2i Spigots %.2i - %.2i",jj,(kk)*6,kk*6+5);
        ledtimerms[jj][kk] = new TCanvas(fname,ftitle);
        ledtimerms[jj][kk]->Divide(3,2);

        sprintf(fname,"f7%.2ip%iinvalidcaps",jj,kk+1);
        sprintf(ftitle,"Invalid CapIDs FED 7%.2i Spigots %.2i - %.2i",jj,(kk)*6,kk*6+5);
        invalidcaps[jj][kk] = new TCanvas(fname,ftitle);
        invalidcaps[jj][kk]->Divide(3,2);

      }

      for (int ll = STARTSPIG; ll < MAXSPIG; ll++) {
        TH1F* hledsum;
        TH1F* hledshape;

        sprintf(fname,"kplotter/FED_7%.2i/Spigot %.2i",jj,ll);
        if( fs->cd(fname) ) workingspigdir = (TDirectory*)fs->GetDirectory(fname);
        else continue; 

        sprintf(fname,"kplotter/FED_7%.2i/Spigot %.2i/Pedestal Plots",jj,ll);
        if( fs->cd(fname) ) workingpeddir = (TDirectory*)fs->GetDirectory(fname);

        sprintf(fname,"kplotter/FED_7%.2i/Spigot %.2i/LED Plots",jj,ll);
        if( fs->cd(fname) ) workingleddir = (TDirectory*)fs->GetDirectory(fname);

        sprintf(fname,"kplotter/FED_7%.2i/Spigot %.2i/LED Plots/Shape",jj,ll);
        if( fs->cd(fname) ) workingshapedir = (TDirectory*)fs->GetDirectory(fname);

        sprintf(fname,"kplotter/FED_7%.2i/Spigot %.2i/LED Plots/Sum",jj,ll);
        if( fs->cd(fname) ) workingsumdir = (TDirectory*)fs->GetDirectory(fname);


        workingpeddir->cd();
        sprintf(fname,"Pedestal mean of spigot %.2i",ll);
        hpedqiemean = (TH1F*)gDirectory->Get(fname);
        pedqiemean[jj][ll/6]->cd((ll%6)+1);
        hpedqiemean->Draw();

        sprintf(fname,"Pedestal RMS of spigot %.2i",ll);
        hpedqierms = (TH1F*)gDirectory->Get(fname);
        pedqierms[jj][ll/6]->cd((ll%6)+1);
        hpedqierms->Draw();
        
        workingleddir->cd();
        sprintf(fname,"LED timing mean of spigot %.2i",ll);
        hledtimemean = (TH1F*)gDirectory->Get(fname);
        ledtimemean[jj][ll/6]->cd((ll%6)+1);
        hledtimemean->Draw();
        
        sprintf(fname,"LED timing RMS of spigot %.2i",ll);
        hledtimerms = (TH1F*)gDirectory->Get(fname);
        ledtimerms[jj][ll/6]->cd((ll%6)+1);
        hledtimerms->Draw();
       
        workingspigdir->cd();
        sprintf(fname,"CapIDErrsOnSpig_%.2i",ll);
        hspigcapiderrs = (TH1F*)gDirectory->Get(fname);
        invalidcaps[jj][ll/6]->cd((ll%6)+1);
        hspigcapiderrs->Draw();
        
  
        for (int tt = 0; tt < 12; tt++) {
          workingshapedir->cd();
          ledshapes1[jj][ll]->cd(tt+1);
          sprintf(ftitle,"QIEShapeLED_S%.2i_Ch%.2i",ll,tt+1);
          hledshape = (TH1F*)gDirectory->Get(ftitle);
          hledshape->Draw();
  
          ledshapes2[jj][ll]->cd(tt+1);
          sprintf(ftitle,"QIEShapeLED_S%.2i_Ch%.2i",ll,tt+1+12);
          hledshape = (TH1F*)gDirectory->Get(ftitle);
          hledshape->Draw();
  
          workingsumdir->cd();
          ledsum1[jj][ll]->cd(tt+1);
          sprintf(ftitle,"QIEsSumLED_S%.2i_Ch%.2i",ll,tt+1);
          hledsum = (TH1F*)gDirectory->Get(ftitle);
          hledsum->Draw();
  
          ledsum2[jj][ll]->cd(tt+1);
          sprintf(ftitle,"QIEsSumLED_S%.2i_Ch%.2i",ll,tt+1+12);
          hledsum = (TH1F*)gDirectory->Get(ftitle);
          hledsum->Draw();
        }
/*
        ledsum1->Print();
        ledsum2->Print();
        ledshapes1->Print();
        ledshapes2->Print();

        pedsum1->Print();
        pedsum2->Print();
        pedshapes1->Print();
        pedshapes2->Print();
*/
      }
/*
      pedqiemean->Print();
      pedqierms->Print();
      ledtimemean->Print();
      ledtimerms->Print();
*/
    workinggendir->cd();
    sprintf(fname,"Invalid Channels for FED 7%.2i",jj);
    invalidcapsall[jj] = new TCanvas(fname,fname);
    sprintf(fname,"Invalid Ch v Spigot FED 7%.2i",jj);
    hfedcapiderrs = (TH2F*)gDirectory->Get(fname);
    invalidcapsall[jj]->cd();
    hfedcapiderrs->Draw();
    }
    else continue;
  }
//  fs->Write();
//  fs-Close();
}
