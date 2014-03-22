//#include <algorithm>
#include <vector>
//#include "TPostScript.h"
//#include "TPaveLabel.h"

void plotRMqa(int runno)
{

  //don't check if the run is in the range specified in the input file 
  bool ProcessAnyRun=true; 

  // Turns off statistica box
  gStyle->SetOptStat(0);
  ifstream inp;
  ostringstream name;
  std::string stringname;
  // Specify root file name here 
  //name<<"muiso_QCD.root";
  char filename[200];
  sprintf(filename, "Run_%06d.root", runno);
  //cout <<"Enter root name: ";
  //cin >>stringname;
  //name<<stringname;
  name<<filename;
  inp.open(name.str().c_str(), ifstream::in);
  inp.close();

  if(inp.fail())
    {
      cout <<"Can't find file '"<<name.str().c_str()<<"'"<<endl;
      exit();
    }

  //*****************
  // Francesco ->->->
  //*****************
  // Read the input file
  string inputList_ = "RM_runs.txt";

  vector<int> v_RUN; 
  vector<string> v_RM0;
  vector<string> v_RM1;
  vector<string> v_RM2;
  vector<string> v_RM3;

  int prevrun = -100;

  ifstream is(inputList_.c_str());
  if(is.good())
    {
      cout << "Reading list: " << inputList_ << " ......." << endl;
      while(is)
 	{
	  int RUN; string RM0; string RM1; string RM2; string RM3;
	  is >> RUN >> RM0 >> RM1 >> RM2 >> RM3;

	  if (RUN > prevrun) {
	    v_RUN.push_back(RUN);
	    v_RM0.push_back(RM0);
	    v_RM1.push_back(RM1);
	    v_RM2.push_back(RM2);
	    v_RM3.push_back(RM3);
	    prevrun = RUN;
	  }
	}
      cout << "Finished reading list: " << inputList_ << endl;
    }
  else
    {
      cout << "ERROR opening file:" << inputList_ << endl;
      exit (1);
    }
  is.close();

  //check: print file content
  bool printFile=false;
  if(printFile)
    {
      for(int run=0; run<v_RUN.size();run++)
	{
	  cout << v_RUN[run] << " "
	       << v_RM0[run] << " "
	       << v_RM1[run] << " "
	       << v_RM2[run] << " "
	       << v_RM3[run] << " "
	       << endl;
	}
    }
  
  string thisRM0="none";
  string thisRM1="none";
  string thisRM2="none";
  string thisRM3="none";
  
  for(int run=0; run<v_RUN.size(); run++)
    {

      if(runno<v_RUN[0])
	{

	  if(run==0)
	    {
	      cout << "WARNING: run" << runno 
		   << " is not included in the range of runs specified in the file " 
		   << inputList_ << endl;
	    }	  

	  if(ProcessAnyRun==false)
	    {
	      cout << "end" << endl;
	      exit();
	    }
	}

      if(run<v_RUN.size()-1)
 	{
 	  if(runno>=v_RUN[run] && runno<v_RUN[run+1])
 	    {
	      thisRM0=v_RM0[run];
	      thisRM1=v_RM1[run];
	      thisRM2=v_RM2[run];
	      thisRM3=v_RM3[run];
 	    }
 	}
      else if(run==v_RUN.size()-1 && runno>=v_RUN[run])
 	{
	  thisRM0=v_RM0[run];
	  thisRM1=v_RM1[run];
	  thisRM2=v_RM2[run];
	  thisRM3=v_RM3[run];
 	}
      
    }

  cout << endl;
  cout << "run number: " << runno << endl;
  cout << "RM0: " << thisRM0 << endl;
  cout << "RM1: " << thisRM1 << endl;
  cout << "RM2: " << thisRM2 << endl;
  cout << "RM3: " << thisRM3 << endl;

  //*****************
  // <-<-<- Francesco
  //*****************




  TFile *myfile = TFile::Open(name.str().c_str());
  TH1F* ledchg0 = (TH1F*)myfile->Get("kplotter/FED_700/Spigot 00/LED Plots/QIEsSumLED_S00");
  TH1F* ledtime0 = (TH1F*)myfile->Get("kplotter/FED_700/Spigot 00/LED Plots/LED timing mean of spigot 00");
  TH1F* pedavg0 = (TH1F*)myfile->Get("kplotter/FED_700/Spigot 00/Pedestal Plots/Pedestal mean of spigot 00");
  TH1F* pedrms0 = (TH1F*)myfile->Get("kplotter/FED_700/Spigot 00/Pedestal Plots/Pedestal RMS of spigot 00");

  TH1F* ledchg1 = (TH1F*)myfile->Get("kplotter/FED_700/Spigot 01/LED Plots/QIEsSumLED_S01");
  TH1F* ledtime1 = (TH1F*)myfile->Get("kplotter/FED_700/Spigot 01/LED Plots/LED timing mean of spigot 01");
  TH1F* pedavg1 = (TH1F*)myfile->Get("kplotter/FED_700/Spigot 01/Pedestal Plots/Pedestal mean of spigot 01");
  TH1F* pedrms1 = (TH1F*)myfile->Get("kplotter/FED_700/Spigot 01/Pedestal Plots/Pedestal RMS of spigot 01");

  TH1F* ledchg2 = (TH1F*)myfile->Get("kplotter/FED_700/Spigot 02/LED Plots/QIEsSumLED_S02");
  TH1F* ledtime2 = (TH1F*)myfile->Get("kplotter/FED_700/Spigot 02/LED Plots/LED timing mean of spigot 02");
  TH1F* pedavg2 = (TH1F*)myfile->Get("kplotter/FED_700/Spigot 02/Pedestal Plots/Pedestal mean of spigot 02");
  TH1F* pedrms2 = (TH1F*)myfile->Get("kplotter/FED_700/Spigot 02/Pedestal Plots/Pedestal RMS of spigot 02");

  TH1F* ledchg3 = (TH1F*)myfile->Get("kplotter/FED_700/Spigot 03/LED Plots/QIEsSumLED_S03");
  TH1F* ledtime3 = (TH1F*)myfile->Get("kplotter/FED_700/Spigot 03/LED Plots/LED timing mean of spigot 03");
  TH1F* pedavg3 = (TH1F*)myfile->Get("kplotter/FED_700/Spigot 03/Pedestal Plots/Pedestal mean of spigot 03");
  TH1F* pedrms3 = (TH1F*)myfile->Get("kplotter/FED_700/Spigot 03/Pedestal Plots/Pedestal RMS of spigot 03");


  //bgmet->SetMarkerStyle(20);
  //bgMET->GetXaxis()->SetTitle("QIEsSumLED_S00");
  //bgMET->GetYaxis()->SetTitle("MET (GeV)");

  TCanvas *c0 = new TCanvas("c0","c0",900,900);
  TCanvas *c1 = new TCanvas("c1","c1",900,900);
  TCanvas *c2 = new TCanvas("c2","c2",900,900);
  TCanvas *c3 = new TCanvas("c3","c3",900,900);

  //*****************
  // Francesco ->->->
  //*****************
  char plottitle0[200];
  char plottitle1[200];
  char plottitle2[200];
  char plottitle3[200];
  sprintf(plottitle0, "%s run %d", thisRM0.c_str(), runno);
  sprintf(plottitle1, "%s run %d", thisRM1.c_str(), runno);
  sprintf(plottitle2, "%s run %d", thisRM2.c_str(), runno);
  sprintf(plottitle3, "%s run %d", thisRM3.c_str(), runno);
  TPaveLabel *title0 = new TPaveLabel(0.199115,0.280488,0.79941,0.841463,plottitle0);
  TPaveLabel *title1 = new TPaveLabel(0.199115,0.280488,0.79941,0.841463,plottitle1);
  TPaveLabel *title2 = new TPaveLabel(0.199115,0.280488,0.79941,0.841463,plottitle2);
  TPaveLabel *title3 = new TPaveLabel(0.199115,0.280488,0.79941,0.841463,plottitle3);
 
  char psname[200];
  sprintf(psname, "RMQC_%s_%06d.ps", thisRM0.c_str(), runno);
  TPostScript *ps = new TPostScript(psname);

  ps->NewPage();
  c0->cd();
  c0->SetTitle(thisRM0.c_str());
  TPad *pa0 = new TPad("pa0","pa0",0,0.9,1,1); pa0->Draw();
  TPad *pb0 = new TPad("pb0","pb0",0,0.45,0.5,0.9); pb0->Draw();
  TPad *pc0 = new TPad("pc0","pc0",0.5,0.45,1,0.9); pc0->Draw();
  TPad *pd0 = new TPad("pd0","pd0",0,0,0.5,0.45); pd0->Draw();
  TPad *pe0 = new TPad("pe0","pe0",0.5,0,1,0.45); pe0->Draw();
  pa0->cd();
  title0->Draw();
  pb0->cd();
  //gPad->SetLogy();
  ledchg0->Draw();
  pc0->cd();
  ledtime0->Draw();
  pd0->cd();
  pedavg0->Draw();
  pe0->cd();
  pedrms0->Draw();
  c0->Update();
  ps->Close();

  sprintf(psname, "RMQC_%s_%06d.ps", thisRM1.c_str(), runno);
  TPostScript *ps = new TPostScript(psname);
  ps->NewPage();
  c1->cd();
  c1->SetTitle(thisRM1.c_str());
  TPad *pa1 = new TPad("pa1","pa1",0,0.9,1,1); pa1->Draw();
  TPad *pb1 = new TPad("pb1","pb1",0,0.45,0.5,0.9); pb1->Draw();
  TPad *pc1 = new TPad("pc1","pc1",0.5,0.45,1,0.9); pc1->Draw();
  TPad *pd1 = new TPad("pd1","pd1",0,0,0.5,0.45); pd1->Draw();
  TPad *pe1 = new TPad("pe1","pe1",0.5,0,1,0.45); pe1->Draw();
  pa1->cd();
  title1->Draw();
  pb1->cd();
  //gPad->SetLogy();
  ledchg1->Draw();
  pc1->cd();
  ledtime1->Draw();
  pd1->cd();
  pedavg1->Draw();
  pe1->cd();
  pedrms1->Draw();
  c1->Update();
  ps->Close();

  sprintf(psname, "RMQC_%s_%06d.ps", thisRM2.c_str(), runno);
  TPostScript *ps = new TPostScript(psname);

  ps->NewPage();
  c2->cd();
  c2->SetTitle(thisRM2.c_str());
  TPad *pa2 = new TPad("pa2","pa2",0,0.9,1,1); pa2->Draw();
  TPad *pb2 = new TPad("pb2","pb2",0,0.45,0.5,0.9); pb2->Draw();
  TPad *pc2 = new TPad("pc2","pc2",0.5,0.45,1,0.9); pc2->Draw();
  TPad *pd2 = new TPad("pd2","pd2",0,0,0.5,0.45); pd2->Draw();
  TPad *pe2 = new TPad("pe2","pe2",0.5,0,1,0.45); pe2->Draw();
  pa2->cd();
  title2->Draw();
  pb2->cd();
  //gPad->SetLogy();
  ledchg2->Draw();
  pc2->cd();
  ledtime2->Draw();
  pd2->cd();
  pedavg2->Draw();
  pe2->cd();
  pedrms2->Draw();
  c2->Update();
  ps->Close();

  sprintf(psname, "RMQC_%s_%06d.ps", thisRM3.c_str(), runno);
  TPostScript *ps = new TPostScript(psname);


  ps->NewPage();
  c3->cd();
  c3->SetTitle(thisRM3.c_str());
  TPad *pa3 = new TPad("pa3","pa3",0,0.9,1,1); pa3->Draw();
  TPad *pb3 = new TPad("pb3","pb3",0,0.45,0.5,0.9); pb3->Draw();
  TPad *pc3 = new TPad("pc3","pc3",0.5,0.45,1,0.9); pc3->Draw();
  TPad *pd3 = new TPad("pd3","pd3",0,0,0.5,0.45); pd3->Draw();
  TPad *pe3 = new TPad("pe3","pe3",0.5,0,1,0.45); pe3->Draw();
  pa3->cd();
  title3->Draw();
  pb3->cd();
  //gPad->SetLogy();
  ledchg3->Draw();
  pc3->cd();
  ledtime3->Draw();
  pd3->cd();
  pedavg3->Draw();
  pe3->cd();
  pedrms3->Draw();
  c3->Update();
  ps->Close();

  //*****************
  // <-<-<- Francesco
  //*****************


  
  cout << "end" << endl;
  //exit();

}
