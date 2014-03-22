
#include <vector>

void rmsplotter(string filenam) {

  int runno;
  int sp[4];
  
  std::vector<int> runnos_;
  std::vector<int> spigos_;
  std::vector<int> values_;
  
  gStyle->SetOptStat(111111);
  
  char fname[128];
  string readfile;
  
  readfile=filenam + ".txt";  
  ifstream input(readfile.c_str());
  if(input.good()) {
    while(input) {
      input >> runno >> sp[0] >> sp[1] >> sp[2] >> sp[3];
      runnos_.push_back(runno);
      values_.push_back(runno);
      for (int jj = 0; jj < 4; jj++) {
	spigos_.push_back(sp[jj]);
	values_.push_back(sp[jj]);}}}
  input.close();
  
  TFile* fs;
  
  TDirectory* workingdir;
  TDirectory* workingfeddir;
  TDirectory* workingspigdir;
  TDirectory* workingpeddir;
  
  TCanvas* pedqierms;
  TH1F*    hpedqierms;
  
  string psname;
  psname=filenam + ".ps";  
  TPostScript *ps = new TPostScript(psname.c_str());
  
  hpedqierms = new TH1F(fname,fname,100,0.0,2.0);
  hpedqierms->SetXTitle("Pedestal RMS");
  hpedqierms->SetYTitle("Number of Channels / 0.02");
  
  sprintf(fname,"%s",filenam.c_str());
  cout<<fname<<endl;
  pedqierms = new TCanvas(fname,fname);
  pedqierms->cd();
  
  int counter = 1;
  int fileexists;
  for (std::vector<int>::iterator itr = values_.begin(); itr!=values_.end(); ++itr) {
    if (counter%5==1) {
      sprintf(fname,"./Run_%6.6d.root",static_cast<int>(*itr));
      
      fs = new TFile(fname,"read");
      if(!(fs->IsZombie())) fileexists = 1;
      else fileexists = 0;}

    else if (static_cast<int>(*itr)) {
      sprintf(fname,"kplotter/FED_7%.2i/Spigot %.2i/Pedestal Plots/Pedestal RMS of spigot %.2i",0,(counter-2)%5,(counter-2)%5);
      if (!(fs->Get(fname))) sprintf(fname,"FED_7%.2i/Spigot %.2i/Pedestal Plots/Pedestal RMS of spigot %.2i",0,(counter-2)%5,(counter-2)%5);
      if (fileexists) {
	my1dhist = (TH1F*)fs->Get(fname);
	for (int kk = 4; kk < 22; kk++) {//channels 1-3 and 22-24 are not used
	  hpedqierms->Fill(my1dhist->GetBinContent(kk+1));}}}
    counter++;}
  
  hpedqierms->Draw();
  pedqierms->Update();
  ps->Close();
}
