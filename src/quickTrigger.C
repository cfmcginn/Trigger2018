//cpp dependencies
#include <iostream>
#include <string>
#include <vector>

//ROOT dependencies
#include "TFile.h"
#include "TTree.h"
#include "TDatime.h"
#include "TMath.h"
#include "TH1D.h"
#include "TGraphAsymmErrors.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TLine.h"
#include "TLatex.h"

//local dependencies
#include "include/checkMakeDir.h"
#include "include/histDefUtility.h"
#include "include/kirchnerPalette.h"
#include "include/L1AnalysisEventDataFormat.h"
#include "include/L1AnalysisL1UpgradeDataFormat.h"
#include "include/plotUtilities.h"
#include "include/returnRootFileContentsList.h"
#include "include/runLumiEventKey.h"
#include "include/stringUtil.h"

int quickTrigger(const std::string inOffFileName, const std::string jetOrEGTree, const std::string inHLTFileName, const std::string commaSeparatedTrigList, const std::string addedTag)
{
  if(!checkFile(inOffFileName) || inOffFileName.find(".root") == std::string::npos){
    std::cout << "Given inOffFileName \'" << inOffFileName << "\' is invalid. return 1" << std::endl;
    return 1;
  }

  if(!checkFile(inHLTFileName) || inHLTFileName.find(".root") == std::string::npos){
    std::cout << "Given inHLTFileName \'" << inHLTFileName << "\' is invalid. return 1" << std::endl;
    return 1;
  }

  kirchnerPalette kp;

  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;
  
  TFile* inOffFile_p = new TFile(inOffFileName.c_str(), "READ");
  std::vector<std::string> listOfTTree = returnRootFileContentsList(inOffFile_p, "TTree");
  inOffFile_p->Close();
  delete inOffFile_p;

  bool hasJetOrEGTree = false;
  std::string hltTreeName = "";
  std::string hiTreeName = "";
  for(auto const & tree : listOfTTree){
    if(tree.find("hlt") != std::string::npos && tree.find("HltTree") != std::string::npos) hltTreeName = tree;
    if(tree.find("hiEvt") != std::string::npos && tree.find("HiTree") != std::string::npos) hiTreeName = tree;
    else if(isStrSame(tree, jetOrEGTree)) hasJetOrEGTree = true;
  }

  if(!hasJetOrEGTree || (hltTreeName.size() == 0 && hiTreeName.size() == 0)){
    std::cout << "Given inOffFileName \'" << inOffFileName << "\' doesn't contain jet or hlt/hi tree" << std::endl;
    std::cout << " Available trees: ";
    for(auto const & tree : listOfTTree){
      std::cout << "\'" << tree << "\',";
    }
    std::cout << std::endl;
    std::cout << "Return 1." << std::endl;
    return 1;
  }

  std::string commaSeparatedTrigListCopy = commaSeparatedTrigList;
  if(commaSeparatedTrigListCopy.size() == 0){
    std::cout << "commaSeparatedTrigList \'" << commaSeparatedTrigList << "\' must be non-zero. return 1" << std::endl;
  }
  if(commaSeparatedTrigListCopy.substr(commaSeparatedTrigListCopy.size()-1, 1).find(",") == std::string::npos) commaSeparatedTrigListCopy = commaSeparatedTrigListCopy + ",";
  
  std::vector<std::string> trigList;
  std::vector<bool> trigListGood;

  while(commaSeparatedTrigListCopy.find(",") != std::string::npos){
    std::string temp = commaSeparatedTrigListCopy.substr(0, commaSeparatedTrigListCopy.find(","));
    trigList.push_back(temp);
    trigListGood.push_back(false);

    commaSeparatedTrigListCopy.replace(0, commaSeparatedTrigListCopy.find(",")+1, "");
  }

  TFile* inHLTFile_p = new TFile(inHLTFileName.c_str(), "READ");
  std::vector<std::string> listOfTTree2 = returnRootFileContentsList(inHLTFile_p, "TTree");
  std::string hltTreeName2 = "";
  std::string L1UpgradeTreeName = "";
  std::string L1EventTreeName = "";
  for(auto const & tree : listOfTTree2){
    if(tree.find("hlt") != std::string::npos && tree.find("HltTree") != std::string::npos) hltTreeName2 = tree;
    if(tree.find("l1EventTree") != std::string::npos) L1EventTreeName = tree;
    if(tree.find("l1UpgradeTree") != std::string::npos) L1UpgradeTreeName = tree;
  }

  if(hltTreeName2.size() != 0){
    TTree* hltTree2_p = (TTree*)inHLTFile_p->Get(hltTreeName2.c_str());
    TObjArray* listOfBranches = (TObjArray*)hltTree2_p->GetListOfBranches();

    for(Int_t bI = 0; bI < listOfBranches->GetEntries(); ++bI){
      for(unsigned int tI = 0; tI < trigList.size(); ++tI){
	if(trigListGood.at(tI)) continue;
	if(isStrSame(trigList.at(tI), listOfBranches->At(bI)->GetName())) trigListGood.at(tI) = true;
      }
    }
  }
      
  
  inHLTFile_p->Close();
  delete inHLTFile_p;

  if(hltTreeName2.size() == 0 && (L1UpgradeTreeName.size() == 0 || L1EventTreeName.size() == 0)){
    std::cout << "Give inHLTFileName \'" << inHLTFileName << "\' contains no valid hlt tree or l1 ntyple tree. return 1" << std::endl;
    return 1;
  }


  if(hltTreeName2.size() != 0){
    unsigned int pos = 0;
    while(pos < trigListGood.size()){
      if(trigListGood.at(pos)) ++pos;
      else{
	std::cout << "Erasing requested trigger \'" << trigList.at(pos) << "\'. not present" << std::endl;
	trigListGood.erase(trigListGood.begin()+pos);
	trigList.erase(trigList.begin()+pos);
      }
    }
  }

  if(trigList.size() == 0){
    std::cout << "Give inHLTFileName \'" << inHLTFileName << "\' contains none of the requested triggers. return 1" << std::endl;
    return 1;
  }

  
  std::map<unsigned long long, int> mapOfHLTRunLumiEvtToEntry;
  TTree* hltTree2_p = NULL;
  TTree* l1EventTree_p = NULL;
  TTree* l1UpgradeTree_p = NULL;

  if(!isStrSame(inHLTFileName, inOffFileName)){
    inHLTFile_p = new TFile(inHLTFileName.c_str(), "READ");

    L1Analysis::L1AnalysisEventDataFormat* Event = new L1Analysis::L1AnalysisEventDataFormat();
    Int_t runHLT2_, lumiHLT2_;
    ULong64_t eventHLT2_;

    if(hltTreeName2.size() != 0){
      hltTree2_p = (TTree*)inHLTFile_p->Get(hltTreeName2.c_str());
      hltTree2_p->SetBranchStatus("*", 0);
      hltTree2_p->SetBranchStatus("Run", 1);
      hltTree2_p->SetBranchStatus("LumiBlock", 1);
      hltTree2_p->SetBranchStatus("Event", 1);
      
      hltTree2_p->SetBranchAddress("Run", &runHLT2_);
      hltTree2_p->SetBranchAddress("LumiBlock", &lumiHLT2_);
      hltTree2_p->SetBranchAddress("Event", &eventHLT2_);
    }
    else{
      l1EventTree_p = (TTree*)inHLTFile_p->Get(L1EventTreeName.c_str());
      l1EventTree_p->SetBranchStatus("*", 0);
      l1EventTree_p->SetBranchStatus("Event", 1);
      l1EventTree_p->SetBranchStatus("run", 1);
      l1EventTree_p->SetBranchStatus("lumi", 1);
      l1EventTree_p->SetBranchStatus("event", 1);

      l1EventTree_p->SetBranchAddress("Event", &Event);
    }

    Int_t nEntriesTemp = -1;
    if(hltTreeName2.size() != 0) nEntriesTemp = hltTree2_p->GetEntries();
    else nEntriesTemp = l1EventTree_p->GetEntries();
    const Int_t nEntriesHLT = nEntriesTemp;

    std::cout << "Building map..." << std::endl;
    for(Int_t entry = 0; entry < nEntriesHLT; ++entry){
      if(hltTreeName2.size() != 0){
	hltTree2_p->GetEntry(entry);
	unsigned long long key = keyFromRunLumiEvent((UInt_t)runHLT2_, (UInt_t)lumiHLT2_, eventHLT2_);   
	mapOfHLTRunLumiEvtToEntry[key] = entry;
      }
      else{
	l1EventTree_p->GetEntry(entry);
	unsigned long long key = keyFromRunLumiEvent(Event->run, Event->lumi, Event->event);   
	mapOfHLTRunLumiEvtToEntry[key] = entry;
      }      
    }
  }

  
  std::cout << "Map entries: " << mapOfHLTRunLumiEvtToEntry.size() << std::endl;
  std::string denomName = "denomHist_" + jetOrEGTree + "_h";
  while(denomName.find("/") != std::string::npos){denomName.replace(denomName.find("/"), 1, "_");}

  Double_t absEtaMax = 2.0;
  bool isOfflineJet = jetOrEGTree.find("ggHiNtuplizer") == std::string::npos;

  if(jetOrEGTree.find("akCs4PF") != std::string::npos) absEtaMax = 1.3;
  else if(!isOfflineJet) absEtaMax = 1.44;

  const Int_t nBinsJet = 26;
  const Double_t binJetLow = 15;
  const Double_t binJetHi = 155;

  const Int_t nBinsPhoton = 18;
  const Double_t binPhotonLow = 10;
  const Double_t binPhotonHi = 100;
 
  Int_t nBinsTemp = nBinsJet;
  Double_t binTempLow = binJetLow;
  Double_t binTempHi = binJetHi;
  std::string xLabelStr = "Jet p_{T}";

  if(!isOfflineJet){
    nBinsTemp = nBinsPhoton;
    binTempLow = binPhotonLow;
    binTempHi = binPhotonHi;    

    xLabelStr = "Photon p_{T}";
  }

  const Int_t nBins = nBinsTemp;
  const Double_t binLow = binTempLow;
  const Double_t binHi = binTempHi;

  TH1D* dummyHist_p = new TH1D("dummyHist_p", (";" + jetOrEGTree + " " + xLabelStr + ";Efficiency").c_str(), nBins, binLow, binHi);
  dummyHist_p->SetMaximum(1.2);
  dummyHist_p->SetMaximum(0.0);

  TH1D* denomHist_p = new TH1D(denomName.c_str(), (";" + jetOrEGTree + " " + xLabelStr + ";Counts").c_str(), nBins, binLow, binHi);

  L1Analysis::L1AnalysisL1UpgradeDataFormat* Ana = new L1Analysis::L1AnalysisL1UpgradeDataFormat();

  const Int_t nTrig = trigList.size();
  TH1D* trigHist_p[nTrig];
  std::vector<TH1*> tempHistVect = {dummyHist_p, denomHist_p};

  const bool isTrigJet = trigList.at(0).find("Jet") != std::string::npos;
  Int_t trigVal[nTrig];
  bool goodThresh[nTrig];
  for(Int_t i = 0; i < nTrig; ++i){goodThresh[i] = false;}
  int thresh[nTrig];
  unsigned int pos=0;
  for(auto const& trig : trigList){
    std::string tempTrig = trig;
    
    if(tempTrig.find("L1_") != std::string::npos) tempTrig.replace(tempTrig.find("L1_"), 3, "");
    while(tempTrig.size() > 1 && !isStrInt(tempTrig.substr(0,1))){
      tempTrig.replace(0,1, "");
    }
    
    while(tempTrig.size() > 1 && !isStrInt(tempTrig.substr(tempTrig.size()-1,1))){
      tempTrig.replace(tempTrig.size()-1,1, "");
    }
    
    if(!isStrInt(tempTrig)) thresh[pos] = -1;
    else{
      thresh[pos] = std::stoi(tempTrig);
      goodThresh[pos] = true;
    }
    ++pos;
  }


  
  inOffFile_p = new TFile(inOffFileName.c_str(), "READ");

  if(hltTreeName2.size() != 0){
    if(isStrSame(inHLTFileName, inOffFileName)) hltTree2_p = (TTree*)inOffFile_p->Get(hltTreeName2.c_str());
    hltTree2_p->SetBranchStatus("*", 0);
    for(Int_t tI = 0; tI < nTrig; ++tI){
      hltTree2_p->SetBranchStatus(trigList.at(tI).c_str(), 1);
      
      hltTree2_p->SetBranchAddress(trigList.at(tI).c_str(), &(trigVal[tI]));
    }
  }
  else{
    l1UpgradeTree_p = (TTree*)inHLTFile_p->Get(L1UpgradeTreeName.c_str());

    l1UpgradeTree_p->SetBranchStatus("*", 0);
    l1UpgradeTree_p->SetBranchStatus("L1Upgrade", 1);
    l1UpgradeTree_p->SetBranchStatus("nJets", 1);
    l1UpgradeTree_p->SetBranchStatus("jetEt", 1);
    l1UpgradeTree_p->SetBranchStatus("nEGs", 1);
    l1UpgradeTree_p->SetBranchStatus("egEt", 1);

    l1UpgradeTree_p->SetBranchAddress("L1Upgrade", &Ana);
  }

  for(Int_t tI = 0; tI < nTrig; ++tI){
    trigHist_p[tI] = new TH1D(("histNum_" + std::to_string(tI) + "_h").c_str(), (";" + jetOrEGTree + " " + xLabelStr + ";Counts && " + trigList.at(tI)).c_str(), nBins, binLow, binHi);
    tempHistVect.push_back(trigHist_p[tI]);
  }
  
  setSumW2(tempHistVect);
  centerTitles(tempHistVect);

  Int_t runHLT_, lumiHLT_;
  ULong64_t eventHLT_;

  UInt_t runHI_, lumiHI_;
  ULong64_t eventHI_;

  TTree* jetOrEGTree_p = (TTree*)inOffFile_p->Get(jetOrEGTree.c_str());
  TTree* hltTree_p = NULL;
  TTree* hiTree_p = NULL;

  if(!isStrSame(inHLTFileName, inOffFileName)){
    if(hltTreeName.size() != 0){
      hltTree_p = (TTree*)inOffFile_p->Get(hltTreeName.c_str());
      
      hltTree_p->SetBranchStatus("*", 0);
      hltTree_p->SetBranchStatus("Run", 1);
      hltTree_p->SetBranchStatus("LumiBlock", 1);
      hltTree_p->SetBranchStatus("Event", 1);
      
      hltTree_p->SetBranchAddress("Run", &runHLT_);
      hltTree_p->SetBranchAddress("LumiBlock", &lumiHLT_);
      hltTree_p->SetBranchAddress("Event", &eventHLT_);
    }
    else{
      hiTree_p = (TTree*)inOffFile_p->Get(hiTreeName.c_str());
      
      hiTree_p->SetBranchStatus("*", 0);
      hiTree_p->SetBranchStatus("run", 1);
      hiTree_p->SetBranchStatus("lumi", 1);
      hiTree_p->SetBranchStatus("evt", 1);
      
      hiTree_p->SetBranchAddress("run", &runHI_);
      hiTree_p->SetBranchAddress("lumi", &lumiHI_);
      hiTree_p->SetBranchAddress("evt", &eventHI_);
    }
  }

  const Int_t nMaxJet = 500;
  Int_t nref_;
  Float_t jtpt_[nMaxJet];
  Float_t rawpt_[nMaxJet];
  Float_t jtphi_[nMaxJet];
  Float_t jteta_[nMaxJet];
  Float_t jtPfMUF_[nMaxJet];
  Float_t jtPfCHF_[nMaxJet];

  std::vector<float>* phoEt_p = NULL;
  std::vector<float>* phoEta_p = NULL;
  std::vector<float> *pho_swissCrx_p = NULL;
  std::vector<float> *pho_seedTime_p = NULL;
  std::vector<float> *phoSigmaIEtaIEta_2012_p = NULL;
  std::vector<float> *phoHoverE_p = NULL;
  std::vector<float> *pho_ecalClusterIsoR4_p = NULL;
  std::vector<float> *pho_hcalRechitIsoR4_p = NULL;
  std::vector<float> *pho_trackIsoR4PtCut20_p = NULL;


  jetOrEGTree_p->SetBranchStatus("*", 0);

  if(isOfflineJet){
    jetOrEGTree_p->SetBranchStatus("nref", 1);
    jetOrEGTree_p->SetBranchStatus("jtpt", 1);
    jetOrEGTree_p->SetBranchStatus("rawpt", 1);
    jetOrEGTree_p->SetBranchStatus("jtphi", 1);
    jetOrEGTree_p->SetBranchStatus("jteta", 1);
    jetOrEGTree_p->SetBranchStatus("jtPfMUF", 1);
    jetOrEGTree_p->SetBranchStatus("jtPfCHF", 1);
    
    jetOrEGTree_p->SetBranchAddress("nref", &nref_);
    jetOrEGTree_p->SetBranchAddress("jtpt", jtpt_);
    jetOrEGTree_p->SetBranchAddress("rawpt", rawpt_);
    jetOrEGTree_p->SetBranchAddress("jtphi", jtphi_);
    jetOrEGTree_p->SetBranchAddress("jteta", jteta_);
    jetOrEGTree_p->SetBranchAddress("jtPfMUF", jtPfMUF_);
    jetOrEGTree_p->SetBranchAddress("jtPfCHF", jtPfCHF_);
  }
  else{
    jetOrEGTree_p->SetBranchStatus("phoEt", 1);
    jetOrEGTree_p->SetBranchStatus("phoEta", 1);
    jetOrEGTree_p->SetBranchStatus("pho_swissCrx", 1);
    jetOrEGTree_p->SetBranchStatus("pho_seedTime", 1);
    jetOrEGTree_p->SetBranchStatus("phoSigmaIEtaIEta_2012", 1);
    jetOrEGTree_p->SetBranchStatus("phoHoverE", 1);
    jetOrEGTree_p->SetBranchStatus("pho_ecalClusterIsoR4", 1);
    jetOrEGTree_p->SetBranchStatus("pho_hcalRechitIsoR4", 1);
    jetOrEGTree_p->SetBranchStatus("pho_trackIsoR4PtCut20", 1);

    jetOrEGTree_p->SetBranchAddress("phoEt", &(phoEt_p));
    jetOrEGTree_p->SetBranchAddress("phoEta", &(phoEta_p));
    jetOrEGTree_p->SetBranchAddress("pho_swissCrx", &(pho_swissCrx_p));
    jetOrEGTree_p->SetBranchAddress("pho_seedTime", &(pho_seedTime_p));
    jetOrEGTree_p->SetBranchAddress("phoSigmaIEtaIEta_2012", &(phoSigmaIEtaIEta_2012_p));
    jetOrEGTree_p->SetBranchAddress("phoHoverE", &(phoHoverE_p));
    jetOrEGTree_p->SetBranchAddress("pho_ecalClusterIsoR4", &(pho_ecalClusterIsoR4_p));
    jetOrEGTree_p->SetBranchAddress("pho_hcalRechitIsoR4", &(pho_hcalRechitIsoR4_p));
    jetOrEGTree_p->SetBranchAddress("pho_trackIsoR4PtCut20", &(pho_trackIsoR4PtCut20_p));
  }

  Int_t matchCount = 0;
  std::map<unsigned int, unsigned long long> mapOfEventsInRun;
  std::map<unsigned int, unsigned long long> mapOfEventsInRunMatched;

  const Int_t nEntries = jetOrEGTree_p->GetEntries();
  std::cout << "Processing " << jetOrEGTree << ", " << nEntries << "..." << std::endl;
  const Int_t nDiv = TMath::Max(1, (Int_t)(nEntries/20));

  for(Int_t entry = 0; entry < nEntries; ++entry){
    if(entry%nDiv == 0) std::cout << " Entry " << entry << "/" << nEntries << std::endl;
    jetOrEGTree_p->GetEntry(entry);
        
    int outEntry = -1;

    UInt_t runTemp_ = 0;
    UInt_t lumiTemp_ = 0;
    ULong64_t eventTemp_ = 0;

    if(isStrSame(inHLTFileName, inOffFileName)) outEntry = entry;
    else if(hltTreeName.size() != 0){          
      hltTree_p->GetEntry(entry);
      runTemp_ = runHLT_;
      lumiTemp_ = lumiHLT_;
      eventTemp_ = eventHLT_;
    }
    else{
      hiTree_p->GetEntry(entry);
      runTemp_ = runHI_;
      lumiTemp_ = lumiHI_;
      eventTemp_ = eventHI_;
    }

    unsigned long long key = keyFromRunLumiEvent(runTemp_, lumiTemp_, eventTemp_);
    if(outEntry < 0 && mapOfHLTRunLumiEvtToEntry.count(key) != 0) outEntry = mapOfHLTRunLumiEvtToEntry[key];

    if(mapOfEventsInRun.count(runTemp_) == 0){
      mapOfEventsInRun[runTemp_] = 1;
      mapOfEventsInRunMatched[runTemp_] = 0;
    }
    else ++mapOfEventsInRun[runTemp_];
        
    if(outEntry == -1) continue;
    ++matchCount;
    
    ++mapOfEventsInRunMatched[runTemp_];

    if(hltTreeName2.size() != 0) hltTree2_p->GetEntry(outEntry);
    else l1UpgradeTree_p->GetEntry(outEntry);
   
    Float_t tempLeadingPt_ = -1;
    Float_t tempLeadingPhi_ = -1;
    Float_t tempLeadingEta_ = -1;

    if(isOfflineJet){
      for(Int_t jI = 0; jI < nref_; ++jI){
	if(TMath::Abs(jteta_[jI]) > absEtaMax) continue;
	
	if(jetOrEGTree.find("akCs4PF") != std::string::npos){
	  if(jtPfMUF_[jI] > 0.6) continue;
	  if(jtPfCHF_[jI] > 0.9) continue;
	}
	
	
	if(jteta_[jI] >= -3.0 && jteta_[jI] <= -1.5){
	  if(jtphi_[jI] >= -1.5 && jtphi_[jI] <= -1.0) continue;
	}
	
	if(jtpt_[jI] > tempLeadingPt_){
	  tempLeadingPt_ = jtpt_[jI];
	  tempLeadingPhi_ = jtphi_[jI];
	  tempLeadingEta_ = jteta_[jI];
	}
      }
    }
    else{
      for(unsigned int pI = 0; pI < phoEt_p->size(); ++pI){
	if(TMath::Abs(phoEta_p->at(pI)) > absEtaMax) continue;
	if(TMath::Abs(pho_seedTime_p->at(pI)) >= 3) continue;
	if(pho_swissCrx_p->at(pI) >= 0.9) continue;
	if(phoSigmaIEtaIEta_2012_p->at(pI) <= 0.002) continue;
	if(phoHoverE_p->at(pI) >= 0.15) continue;
	if(pho_ecalClusterIsoR4_p->at(pI) + pho_hcalRechitIsoR4_p->at(pI) + pho_trackIsoR4PtCut20_p->at(pI) >= 20) continue;
	if(phoSigmaIEtaIEta_2012_p->at(pI) >= 0.02) continue;

	if(tempLeadingPt_ < phoEt_p->at(pI)) tempLeadingPt_ = phoEt_p->at(pI);
      }
    }


    if(tempLeadingPt_ >= binLow && tempLeadingPt_ < binHi){
      denomHist_p->Fill(tempLeadingPt_);

      if(hltTreeName2.size() == 0){
	Double_t leadingL1Object = -1;

	if(isTrigJet){
	  for(unsigned int i = 0; i < Ana->jetEt.size(); ++i){
	    if(Ana->jetEt.at(i) > leadingL1Object) leadingL1Object = Ana->jetEt.at(i);
	  }
	}
	else{
	  for(unsigned int i = 0; i < Ana->egEt.size(); ++i){
	    if(Ana->egEt.at(i) > leadingL1Object) leadingL1Object = Ana->egEt.at(i);
	  }
	}

	for(Int_t i = 0; i < nTrig; ++i){
	  if(goodThresh[i] && leadingL1Object >= thresh[i]){
	    trigVal[i] = 1;
	  }
	  else trigVal[i] = 0;
	}
      }
     
      for(Int_t tI = 0; tI < nTrig; ++tI){
	if(trigVal[tI]){
	  trigHist_p[tI]->Fill(tempLeadingPt_);
	  
	  if(isOfflineJet && tempLeadingPt_ > 80 && trigList.at(tI).find("56") != std::string::npos && jetOrEGTree.find("akCs4PF") != std::string::npos){

	    if(!isStrSame(inHLTFileName, inOffFileName)){
	      std::cout << "RLEGOOD: " << runTemp_ << ":" << lumiTemp_ << ":" << eventTemp_ << std::endl;
	    }
	  }
	}
	else if(isOfflineJet && tempLeadingPt_ > 80 && trigList.at(tI).find("56") != std::string::npos){
	  if(!isStrSame(inHLTFileName, inOffFileName)){
	    std::cout << "RLEBAD: " << runTemp_ << ":" << lumiTemp_ << ":" << eventTemp_ << std::endl;
	  }
	  std::cout << "entry, leadingpt, phi, eta: " << entry << ", " << tempLeadingPt_ << ", " << tempLeadingPhi_ << ", " << tempLeadingEta_ << std::endl;
	}
      }
    }    
  }

  
  TGraphAsymmErrors* aPt_p[nTrig];
  TGraphAsymmErrors* aPt_Inv_p[nTrig];

  TCanvas* canv_p = new TCanvas("canv_p", "", 450, 450);
  canv_p->SetTopMargin(0.01);
  canv_p->SetRightMargin(0.01);
  canv_p->SetLeftMargin(0.12);
  canv_p->SetBottomMargin(0.12);

  canv_p->cd();

  dummyHist_p->SetMaximum(1.1);
  dummyHist_p->SetMinimum(0.0);
  dummyHist_p->DrawCopy();
  gStyle->SetOptStat(0);

  const Int_t nColors = 4;
  const Int_t nStyles = 5;
  const Int_t styles[nStyles] = {24, 25, 27, 28, 46};

  TLegend* leg_p = new TLegend(0.6, 0.2, 0.9, 0.5);
  leg_p->SetTextFont(43);
  leg_p->SetTextSize(12);
  leg_p->SetBorderSize(0);
  leg_p->SetFillStyle(0);
  leg_p->SetFillColor(0);

  TLegend* legInv_p = new TLegend(0.6, 0.6, 0.9, 0.9);
  legInv_p->SetTextFont(43);
  legInv_p->SetTextSize(12);
  legInv_p->SetBorderSize(0);
  legInv_p->SetFillStyle(0);
  legInv_p->SetFillColor(0);

  Double_t minNotZero = 100;
  bool below1Perc[nTrig];
  Double_t xVal1Perc[nTrig];
  for(Int_t tI = 0; tI < nTrig; ++tI){
    below1Perc[tI] = false;
    xVal1Perc[tI] = -1;
  }

  for(Int_t tI = 0; tI < nTrig; ++tI){
    aPt_p[tI] = new TGraphAsymmErrors();
    aPt_Inv_p[tI] = new TGraphAsymmErrors();
    aPt_p[tI]->BayesDivide(trigHist_p[tI], denomHist_p);

    Double_t xVal, yVal;
    for(Int_t pI = 0; pI < aPt_p[tI]->GetN(); ++pI){
      aPt_p[tI]->GetPoint(pI, xVal, yVal);

      Double_t yErrHigh = aPt_p[tI]->GetErrorYhigh(pI);
      Double_t yErrLow = aPt_p[tI]->GetErrorYlow(pI);
      Double_t xErrHigh = aPt_p[tI]->GetErrorXhigh(pI);
      Double_t xErrLow = aPt_p[tI]->GetErrorXlow(pI);

      if(1.-yVal > 0){
	if(1.-yVal < minNotZero) minNotZero = 1.-yVal;
      }
      else{
	yErrHigh = 0;
	yErrLow = 0;
      }

      if(1.-yVal < 0.01 && !below1Perc[tI]) xVal1Perc[tI] = xVal - xErrLow;
      if(1.-yVal < 0.01) below1Perc[tI] = true;
      else below1Perc[tI] = false;

      aPt_Inv_p[tI]->SetPoint(pI, xVal, 1.-yVal);
      aPt_Inv_p[tI]->SetPointError(pI, xErrLow, xErrHigh, yErrHigh, yErrLow);
    }

    aPt_p[tI]->SetMarkerStyle(styles[tI%nStyles]);
    aPt_p[tI]->SetMarkerColor(kp.getColor(tI%nColors));
    aPt_p[tI]->SetLineColor(kp.getColor(tI%nColors));
    aPt_p[tI]->SetMarkerSize(1.);

    aPt_Inv_p[tI]->SetMarkerStyle(styles[tI%nStyles]);
    aPt_Inv_p[tI]->SetMarkerColor(kp.getColor(tI%nColors));
    aPt_Inv_p[tI]->SetLineColor(kp.getColor(tI%nColors));
    aPt_Inv_p[tI]->SetMarkerSize(1.);

    aPt_p[tI]->Draw("P");
    leg_p->AddEntry(aPt_p[tI], trigList.at(tI).c_str(), "P L");
    legInv_p->AddEntry(aPt_p[tI], trigList.at(tI).c_str(), "P L");
  }

  TLatex* label_p = new TLatex();
  label_p->SetNDC();
  label_p->SetTextFont(43);
  label_p->SetTextSize(12);

  label_p->DrawLatex(0.2, 0.95, (addedTag + ", |#eta| < " + prettyString(absEtaMax,2,false)).c_str());

  TLine* line_p = new TLine();
  line_p->SetLineStyle(2);
  line_p->DrawLine(binLow, 1, binHi, 1);

  for(Int_t tI = 0; tI < nTrig; ++tI){
    line_p->SetLineColor(kp.getColor(tI%nColors));
    if(xVal1Perc[tI] > 0 && below1Perc[tI]) line_p->DrawLine(xVal1Perc[tI], 0.0, xVal1Perc[tI], 1.1);
  }
  line_p->SetLineColor(1);

  leg_p->Draw("SAME");

  checkMakeDir("pdfDir");
  checkMakeDir("pdfDir/" + dateStr);
  std::string canvSaveName = "quickTurnOn_" + jetOrEGTree + "_" + addedTag;
  while(canvSaveName.find("/") != std::string::npos){canvSaveName.replace(canvSaveName.find("/"), 1, "_");}
  std::string canvSaveNameInv = canvSaveName + "_INV_" + dateStr + ".pdf";
  std::string canvSaveNameInvPNG = canvSaveName + "_INV_" + dateStr + ".png";
  std::string canvSaveNamePNG = canvSaveName + "_" + dateStr + ".png";
  canvSaveName = canvSaveName + "_" + dateStr + ".pdf";
  canv_p->SaveAs(("pdfDir/" + dateStr + "/" + canvSaveName).c_str());
  canv_p->SaveAs(("pdfDir/" + dateStr + "/" + canvSaveNamePNG).c_str());
  delete canv_p;

  canv_p = new TCanvas("canv_p", "", 450, 450);
  canv_p->SetTopMargin(0.01);
  canv_p->SetRightMargin(0.01);
  canv_p->SetLeftMargin(0.12);
  canv_p->SetBottomMargin(0.12);

  canv_p->cd();

  dummyHist_p->SetMaximum(3.);
  dummyHist_p->SetMinimum(minNotZero/3.);
  dummyHist_p->GetYaxis()->SetTitle("Inefficiency");
  dummyHist_p->DrawCopy();
  gStyle->SetOptStat(0);

  for(Int_t tI = 0; tI < nTrig; ++tI){
    aPt_Inv_p[tI]->Draw("P");
  }


  label_p->DrawLatex(0.2, 0.95, (addedTag + ", |#eta| < " + prettyString(absEtaMax,1,false)).c_str());
  line_p->DrawLine(binLow, 1, binHi, 1);
  legInv_p->Draw("SAME");
  gPad->SetLogy();

  for(Int_t tI = 0; tI < nTrig; ++tI){
    line_p->SetLineColor(kp.getColor(tI%nColors));
    if(xVal1Perc[tI] > 0) line_p->DrawLine(xVal1Perc[tI], minNotZero/3., xVal1Perc[tI], 3.);
  }
  line_p->SetLineColor(1);

  canv_p->SaveAs(("pdfDir/" + dateStr + "/" + canvSaveNameInv).c_str());
  canv_p->SaveAs(("pdfDir/" + dateStr + "/" + canvSaveNameInvPNG).c_str());
  delete canv_p;

  delete line_p;
  delete leg_p;
  delete legInv_p;

  for(Int_t tI = 0; tI < nTrig; ++tI){
    delete aPt_p[tI];
    delete trigHist_p[tI];
  }

  delete denomHist_p;
  delete dummyHist_p;

  inOffFile_p->Close();
  delete inOffFile_p;

  if(!isStrSame(inHLTFileName, inOffFileName)){
    inHLTFile_p->Close();
    delete inHLTFile_p;
  }
  

  std::cout << "Match Count: " << matchCount << "/" << nEntries << " = " << ((Double_t)matchCount)/((Double_t)nEntries) << std::endl;

  for(auto const& map : mapOfEventsInRun){
    std::cout << " " << map.first << ": " << mapOfEventsInRunMatched[map.first] << "/" << map.second << " = " << ((Double_t)mapOfEventsInRunMatched[map.first])/((Double_t)map.second) << std::endl;
  }

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 6){
    std::cout << "Usage: ./bin/quickTrigger.exe <inOffFileName> <jetOrEGTree> <inHLTFileName> <commaSeparatedTrigList> <addedTag>" << std::endl;
    return 1;
  }
  
  int retVal = 0;
  retVal += quickTrigger(argv[1], argv[2], argv[3], argv[4], argv[5]);
  return retVal;
}
