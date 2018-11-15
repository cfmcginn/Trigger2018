//cpp dependencies
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

//ROOT dependencies
#include "TFile.h"
#include "TTree.h"
#include "TObjArray.h"
#include "TMath.h"
#include "TH1F.h"
#include "TDatime.h"

//Local dependencies
#include "include/checkMakeDir.h"
#include "include/listOfPrimes.h"
#include "include/plotUtilities.h"
#include "include/quickCentralityTable_nBins200_Temp_20181030.h"
#include "include/returnRootFileContentsList.h"
#include "include/runLumiEventKey.h"
#include "include/stringUtil.h"

int doPrescaling(const std::string inFileName, const std::string prescaleConfigName, const std::string l1XmlFileName, const double inCollisionRatekHz, const bool disableXML = false, const bool disablePrimeOverride = false, const std::string inHiBinFileName = "")
{
  if(!checkFile(inFileName)){
    std::cout << "Warning: Given inFileName \'" << inFileName << "\' is not a valid file. return 1" << std::endl;
    return 1;
  }
  else if(inFileName.find(".root") == std::string::npos){
    std::cout << "Warning: Given inFileName \'" << inFileName << "\' is not a valid .root file. return 1" << std::endl;
    return 1;
  }

  if(!checkFile(prescaleConfigName)){
    std::cout << "Warning: Given prescaleConfigName \'" << prescaleConfigName << "\' is not a valid file. return 1" << std::endl;
    return 1;
  }
  else if(prescaleConfigName.find(".txt") == std::string::npos && prescaleConfigName.find(".csv") == std::string::npos){
    std::cout << "Warning: Given prescaleConfigName \'" << prescaleConfigName << "\' is not a valid .txt or .csv file. return 1" << std::endl;
    return 1;
  }

  if(!checkFile(l1XmlFileName) && !disableXML){
    std::cout << "Warning: Given l1XmlFileName \'" << l1XmlFileName << "\' is not a valid file. return 1" << std::endl;
    return 1;
  }
  else if(l1XmlFileName.find(".xml") == std::string::npos && !disableXML){
    std::cout << "Warning: Given l1XmlFileName \'" << l1XmlFileName << "\' is not a valid .xml file. return 1" << std::endl;
    return 1;
  }

  bool doHibin = inHiBinFileName.size() != 0 && checkFile(inHiBinFileName) && inHiBinFileName.find(".root") != std::string::npos;
  std::map<unsigned long long, int> runLumiEventKeyToHiBin;
  if(doHibin){
    TFile* inFile_p = new TFile(inHiBinFileName.c_str(), "READ");
    TTree* hiTree_p = (TTree*)inFile_p->Get("hiEvtAnalyzer/HiTree");

    UInt_t run_, lumi_;
    ULong64_t evt_;
    Int_t hiBin_;
    Float_t hiHF_;

    hiTree_p->SetBranchStatus("*", 0);
    hiTree_p->SetBranchStatus("run", 1);
    hiTree_p->SetBranchStatus("lumi", 1);
    hiTree_p->SetBranchStatus("evt", 1);
    hiTree_p->SetBranchStatus("hiBin", 1);
    hiTree_p->SetBranchStatus("hiHF", 1);

    hiTree_p->SetBranchAddress("run", &run_);
    hiTree_p->SetBranchAddress("lumi", &lumi_);
    hiTree_p->SetBranchAddress("evt", &evt_);
    hiTree_p->SetBranchAddress("hiBin", &hiBin_);
    hiTree_p->SetBranchAddress("hiHF", &hiHF_);
    
    const Int_t nHIEntries = hiTree_p->GetEntries();
    for(Int_t entry = 0; entry < nHIEntries; ++entry){
      hiTree_p->GetEntry(entry);

      unsigned long long key = keyFromRunLumiEvent(run_, lumi_, evt_);
      runLumiEventKeyToHiBin[key] = getHiBinFromHiHF_Temp(hiHF_);
    }

    inFile_p->Close();
    delete inFile_p;
  }
  

  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;

  TFile* inFile_p = new TFile(inFileName.c_str(), "READ");

  //Pick TTree
  std::vector<std::string> listOfHLTTrees = returnRootFileContentsList(inFile_p, "TTree", "hlt");
  if(listOfHLTTrees.size() == 0){
    std::cout << "Given inFileName \'" << inFileName << "\' contains no TTree w/ \'hlt\' in name. return 1" << std::endl;
    inFile_p->Close();
    delete inFile_p;
    return 1;
  }
  else if(listOfHLTTrees.size() > 1){
    std::cout << "Warning: Given inFileName \'" << inFileName << "\' contains multiple TTree w/ \'hlt\' in name. Picking first, \'" << listOfHLTTrees.at(0) << "\'. Please check file that this is right choice" << std::endl;
  }

  TTree* hltTree_p = (TTree*)inFile_p->Get(listOfHLTTrees.at(0).c_str());

  Int_t run_, lumi_;
  ULong64_t evt_;

  //Pick Branches
  std::vector<std::string> finalListOfBranches;
  TObjArray* initListOfBranches = hltTree_p->GetListOfBranches();
  for(Int_t bI = 0; bI < initListOfBranches->GetEntries(); ++bI){
    std::string branchName = initListOfBranches->At(bI)->GetName();

    if(branchName.find("Prescl") != std::string::npos) continue;
    if(branchName.size() < 4) continue;
    if(branchName.substr(0,4).find("HLT_") == std::string::npos) continue;

    finalListOfBranches.push_back(branchName);
  }

  const Int_t nCentBins = 20;
  const Int_t centBins[nCentBins+1] = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100};
  //Old sizes, bad
  //  const Double_t rawSizes[nCentBins] = {6.64, 4.79, 3.69, 2.95, 2.4, 1.93, 1.6, 1.33, 1.06, 0.9, 0.73, 0.63, 0.5, 0.41, 0.36, 0.33, 0.28, 0.25, 0.26, 0.24};
  //  const Double_t aodSizes[nCentBins] = {3.32, 2.39, 1.84, 1.47, 1.2, 0.97, 0.8, 0.67, 0.53, 0.45, 0.37, 0.31, 0.25, 0.21, 0.18, 0.17, 0.14, 0.12, 0.13, 0.12};

  //New sizes, good
  //From doc: https://docs.google.com/spreadsheets/d/1d2Aq2SoWX5ZlGXB4wiukHPnPpUnhY7ZPLGikLr4IOAU/edit#gid=0
  const Double_t rawSizes[nCentBins] = {3.73, 3.10, 2.60, 2.19, 1.86, 1.56, 1.31, 1.10, 0.93, 0.77, 0.64, 0.54, 0.46, 0.39, 0.35, 0.32, 0.30, 0.29, 0.31, 0.34};
  const Double_t aodSizes[nCentBins] = {3.46, 2.41, 1.78, 1.35, 1.05, 0.82, 0.65, 0.52, 0.40, 0.33, 0.25, 0.21, 0.16, 0.12, 0.10, 0.08, 0.06, 0.05, 0.04, 0.03};

  std::vector<std::string> trigNames;
  std::vector<int> trigPrescale;
  std::vector<std::string> trigPD;
  std::vector<std::string> trigSubPD;
  std::vector<int> trigThreshold;

  std::map<std::string, bool> pdMapIsFired;
  std::map<std::string, std::vector<int> > pdMapCentrality;
  std::map<std::string, int> pdMapToFires;
  std::map<std::string, std::map<std::string, int> > pdMapToFiresMatchedOtherPD;
  std::map<std::string, std::map<std::string, std::vector<int> > > pdMapCentralityMatchedOtherPD;
  std::map<std::string, std::map<std::string, int> > pdMapToFiresTriggerOtherPD;

  std::map<std::string, bool> subPDMapIsFired;
  std::map<std::string, int> subPDMapToFires;

  std::vector<std::string> uniqueSubPD;
  std::vector<std::vector<double> > subPDThresholds;

  std::ifstream prescaleConfig(prescaleConfigName.c_str());
  std::string tempStr;
  while(std::getline(prescaleConfig, tempStr)){
    if(tempStr.size() == 0) continue;

    std::string trigName = tempStr.substr(0, tempStr.find(","));
    tempStr.replace(0, tempStr.find(",")+1, "");
    int prescale = std::stoi(tempStr.substr(0, tempStr.find(",")));
    tempStr.replace(0, tempStr.find(",")+1, "");
    std::string pd = tempStr.substr(0, tempStr.find(","));  
    tempStr.replace(0, tempStr.find(",")+1, "");
    std::string subPD = tempStr.substr(0, tempStr.find(","));  
    tempStr.replace(0, tempStr.find(",")+1, "");
    int threshold = std::stoi(tempStr.substr(0, tempStr.find(",")));

    if(prescale < 0) continue;

    bool isInFile = false;
    for(unsigned int bI = 0; bI < finalListOfBranches.size(); ++bI){
      if(isStrSame(finalListOfBranches.at(bI), trigName)){
	isInFile = true;
	break;
      }
    }

    if(!isInFile){
      std::cout << "Warning: trigger \'" << trigName << "\' requested from file \'" << prescaleConfigName << "\' is not found in file \'" << inFileName << "\'. skipping" << std::endl;
      continue;
    }

    trigNames.push_back(trigName);
    if(disablePrimeOverride) trigPrescale.push_back(prescale);
    else trigPrescale.push_back(getNearestPrime(prescale));
    trigPD.push_back(pd);
    trigSubPD.push_back(subPD);
    trigThreshold.push_back(threshold);
  
    if(pdMapToFires.count(pd) == 0){
      pdMapToFires[pd] = 0;
      pdMapCentrality[pd] = {};
      for(Int_t cI = 0; cI < nCentBins; ++cI){pdMapCentrality[pd].push_back(0);}
    }
    if(subPDMapToFires.count(subPD) == 0) subPDMapToFires[subPD] = 0;

    
    int idPos = -1;
    bool isUniqueSubPD = true;
    for(unsigned int pI = 0; pI < uniqueSubPD.size(); ++pI){
      if(isStrSame(uniqueSubPD.at(pI), subPD)){
	isUniqueSubPD = false;
	idPos = pI;
	break;
      }
    }

    if(isUniqueSubPD){
      uniqueSubPD.push_back(subPD);
      subPDThresholds.push_back({(Double_t)threshold});
    }
    else{
      subPDThresholds.at(idPos).push_back((Double_t)threshold);
    }
  }
  prescaleConfig.close();

  const Int_t nTrig = trigNames.size();
  Int_t trigVal[nTrig];
  Int_t trigValPrescaled[nTrig];
  Int_t trigFires[nTrig];
  Int_t trigPrescaledFires[nTrig];
  
  for(auto const& pd1 : pdMapToFires){
    for(auto const& pd2 : pdMapToFires){
      (pdMapToFiresMatchedOtherPD[pd1.first])[pd2.first] = 0;
      (pdMapCentralityMatchedOtherPD[pd1.first])[pd2.first] = {};

      for(Int_t cI = 0; cI < nCentBins; ++cI){(pdMapCentralityMatchedOtherPD[pd1.first])[pd2.first].push_back(0);}
    }

    for(Int_t tI = 0; tI < nTrig; ++tI){
      if(isStrSame(pd1.first, trigPD[tI])) continue;
      (pdMapToFiresTriggerOtherPD[pd1.first])[trigNames[tI]] = 0;
    }
  }


  std::string matchingL1FromXML[nTrig];
  std::vector<std::string> fullXmlList;
  std::vector<bool> fullXmlListMatched;

  if(!disableXML){
    std::ifstream xmlFile(l1XmlFileName.c_str());
    while(std::getline(xmlFile, tempStr)){
      if(tempStr.find("<name>L1_") == std::string::npos) continue;
      tempStr.replace(tempStr.find("<name>"), std::string("<name>").size(), "");
      tempStr.replace(tempStr.find("</name>"), std::string("</name>").size(), "");
      
      while(tempStr.find(" ") != std::string::npos){tempStr.replace(tempStr.find(" "), 1, "");}
      
      fullXmlList.push_back(tempStr);
      fullXmlListMatched.push_back(false);
    }
    xmlFile.close();
  }

  for(Int_t tI = 0; tI < nTrig; ++tI){
    std::string tempTrigName = trigNames.at(tI);
    tempTrigName.replace(0,4,"");
    tempTrigName.replace(tempTrigName.rfind("_v"), tempTrigName.size(), "");

    bool trigIsFound = false;
    if(disableXML){
      trigIsFound = true;
      matchingL1FromXML[tI] = trigNames.at(tI);
    }
    else{
      for(unsigned int xI = 0; xI < fullXmlList.size(); ++xI){
	if(fullXmlListMatched.at(xI)) continue;
	std::string tempL1Name = fullXmlList.at(xI);
	while(tempL1Name.find("_") != std::string::npos){tempL1Name.replace(tempL1Name.find("_"), 1, "");}
	//      std::cout << "tempL1Name: \'" << tempL1Name << "\', \'" << tempTrigName << "\'" << std::endl;
	
	if(isStrSame(tempL1Name, tempTrigName)){
	  fullXmlListMatched.at(xI) = true;
	  matchingL1FromXML[tI] = fullXmlList.at(xI);
	  trigIsFound = true;
	  break;
	}
      }
      
      if(!trigIsFound){
	std::cout << "WARNING: \'" << trigNames.at(tI) << "\' is not found in xml \'" << l1XmlFileName << "\'. Setting blank" << std::endl;
	matchingL1FromXML[tI] = "MISSING";
      }
    }
  }

  std::string outFileName = inFileName;
  while(outFileName.find("/") != std::string::npos){
    outFileName.replace(0, outFileName.find("/")+1, "");
  }
  std::string outFileName2 = prescaleConfigName;
  while(outFileName2.find("/") != std::string::npos){
    outFileName2.replace(0, outFileName2.find("/")+1, "");
  }
  

  checkMakeDir("output");
  checkMakeDir("output/" + dateStr);
  if(outFileName.find(".root") != std::string::npos) outFileName.replace(outFileName.find(".root"), outFileName.size(), "");
  if(outFileName2.find(".") != std::string::npos) outFileName2.replace(outFileName2.find("."), outFileName2.size(), "");

  outFileName = "output/" + dateStr + "/" + outFileName + "_" + outFileName2 + "_RatePlots_" + dateStr + ".root";

  TFile* outFile_p = new TFile(outFileName.c_str(), "RECREATE");

  const Int_t nSubPD = uniqueSubPD.size();
  TH1F* subPDFiresAtKHz_h[nSubPD];
  for(Int_t sI = 0; sI < nSubPD; ++sI){
    std::vector<double> temp = subPDThresholds.at(sI);
    std::sort(std::begin(temp), std::end(temp));
    subPDThresholds.at(sI) = temp;

    const Int_t nBins = subPDThresholds.at(sI).size();
    Double_t bins[nBins+1];

    if(nBins != 1){
      bins[0] = subPDThresholds.at(sI).at(0) - (subPDThresholds.at(sI).at(1) - subPDThresholds.at(sI).at(0))/2.;
      
      for(Int_t bIX = 0; bIX < nBins-1; ++bIX){
	bins[bIX+1] = subPDThresholds.at(sI).at(bIX) + (subPDThresholds.at(sI).at(bIX+1) - subPDThresholds.at(sI).at(bIX))/2.;
      }
      
      bins[nBins] = subPDThresholds.at(sI).at(nBins-1) + (subPDThresholds.at(sI).at(nBins-1) - subPDThresholds.at(sI).at(nBins-2))/2.;
    }
    else{
      std::cout << "Warning: subPD \'" << uniqueSubPD.at(sI) << "\' has only 1 threshold" << std::endl;
      bins[0] = subPDThresholds.at(sI).at(0) - 1;
      bins[1] = subPDThresholds.at(sI).at(0) + 1;
    }
      
    subPDFiresAtKHz_h[sI] = new TH1F(("subPDFires" + std::to_string((Int_t)inCollisionRatekHz) + "KHz_" + uniqueSubPD.at(sI) + "_h").c_str(), (";Threshold;Rate at " + std::to_string((Int_t)inCollisionRatekHz) + " kHz (Hz)").c_str(), nBins, bins);
  }

  hltTree_p->SetBranchStatus("*", 0);
  hltTree_p->SetBranchStatus("Run", 1);
  hltTree_p->SetBranchStatus("LumiBlock", 1);
  hltTree_p->SetBranchStatus("Event", 1);

  hltTree_p->SetBranchAddress("Run", &run_);
  hltTree_p->SetBranchAddress("LumiBlock", &lumi_);
  hltTree_p->SetBranchAddress("Event", &evt_);

  for(Int_t bI = 0; bI < nTrig; ++bI){
    hltTree_p->SetBranchStatus(trigNames.at(bI).c_str(), 1);
    hltTree_p->SetBranchAddress(trigNames.at(bI).c_str(), &(trigVal[bI]));

    trigFires[bI] = 0;
    trigPrescaledFires[bI] = 0;
  }

  const Int_t nEntries = hltTree_p->GetEntries();
  const Int_t nDiv = TMath::Max(1, nEntries/20);

  Int_t totalFires = 0;

  std::cout << "Processing " << nEntries << "..." << std::endl;
  for(Int_t entry = 0; entry < nEntries; ++entry){
    if(entry%nDiv == 0) std::cout << " Entry " << entry << "/" << nEntries << std::endl;

    hltTree_p->GetEntry(entry);

    unsigned long long key = keyFromRunLumiEvent((UInt_t)run_, (UInt_t)lumi_, evt_);
    if(doHibin){
      if(runLumiEventKeyToHiBin.count(key) == 0) continue;
    }


    for(auto const &iter : pdMapIsFired){
      pdMapIsFired[iter.first] = false;
    }

    for(auto const &iter : subPDMapIsFired){
      subPDMapIsFired[iter.first] = false;
    }
    
    bool doesGlobalFire = false;

    for(Int_t bI = 0; bI < nTrig; ++bI){
      trigValPrescaled[bI] = trigVal[bI];
      if(trigVal[bI] == 1){
	if(trigFires[bI]%trigPrescale.at(bI) == 0){
	  pdMapIsFired[trigPD[bI]] = true;
	  subPDMapIsFired[trigSubPD[bI]] = true;
	  doesGlobalFire = true;
	  ++trigPrescaledFires[bI];
	}
	else trigValPrescaled[bI] = 0;
	++trigFires[bI];
      }
    }
    
    for(auto const &iter : pdMapIsFired){
      if(iter.second){
	++(pdMapToFires[iter.first]);

	Int_t centPos = -1;
	if(doHibin){
	  Int_t hiBin = runLumiEventKeyToHiBin[key];
	  
	  for(Int_t cI = 0; cI < nCentBins; ++cI){
	    if(centBins[cI] <= hiBin/2 && centBins[cI+1] > hiBin/2){
	      ++(pdMapCentrality[iter.first].at(cI));
	      centPos = cI;
	      break;
	    }
	  }
	}


	for(auto const &iter2 : pdMapIsFired){
	  if(iter2.second){
	    ++((pdMapToFiresMatchedOtherPD[iter.first])[iter2.first]);
	    if(doHibin) ++(((pdMapCentralityMatchedOtherPD[iter.first])[iter2.first]).at(centPos));
	  }
	}

	for(Int_t tI = 0; tI < nTrig; ++tI){
	  if(isStrSame(trigPD[tI], iter.first)) continue;
	  if(trigValPrescaled[tI] == 0) continue;

	  ++((pdMapToFiresTriggerOtherPD[iter.first])[trigNames[tI]]);
	}
      }

    }

    for(auto const &iter : subPDMapIsFired){
      if(iter.second) ++(subPDMapToFires[iter.first]);
    }

    if(doesGlobalFire) ++totalFires;
  }

  inFile_p->Close();
  delete inFile_p;

  outFile_p->cd();

  for(Int_t bI = 0; bI < nTrig; ++bI){
    Int_t subPDPos = -1;

    for(Int_t sI = 0; sI < nSubPD; ++sI){
      if(isStrSame(uniqueSubPD.at(sI), trigSubPD[sI])){
	subPDPos = sI;
	break;
      }
    }

    Int_t binPos = subPDFiresAtKHz_h[subPDPos]->FindBin(trigThreshold.at(bI));

    Double_t val = ((Double_t)trigFires[bI])*inCollisionRatekHz*1000./((Double_t)nEntries);
    Double_t valErr = ((Double_t)trigFires[bI] + TMath::Sqrt(trigFires[bI]))*inCollisionRatekHz*1000./((Double_t)nEntries) - val;

    subPDFiresAtKHz_h[subPDPos]->SetBinContent(binPos, val);
    subPDFiresAtKHz_h[subPDPos]->SetBinError(binPos, valErr);
  }
  
  for(Int_t sI = 0; sI < nSubPD; ++sI){
    subPDFiresAtKHz_h[sI]->Write("", TObject::kOverwrite);
    delete subPDFiresAtKHz_h[sI];
  }

  outFile_p->Close();
  delete outFile_p;

  checkMakeDir("output");
  checkMakeDir("output/" + dateStr);
  const std::string outPrescaleName = "output/" + dateStr + "/prescales_" + std::to_string((Int_t)inCollisionRatekHz) + "kHz_" + dateStr + ".csv";
  std::ofstream outPrescaleFile(outPrescaleName.c_str());
  outPrescaleFile << "L1 Trigger,Unprescaled Rate in Hz (" << std::to_string((Int_t)inCollisionRatekHz) << " kHz),Prescale (" << std::to_string((Int_t)inCollisionRatekHz) << " kHz),Prescaled Rate in Hz (" << std::to_string((Int_t)inCollisionRatekHz) << " kHz)," << std::endl;
  for(Int_t bI = 0; bI < nTrig; ++bI){
    outPrescaleFile << matchingL1FromXML[bI] << "," << ((Double_t)trigFires[bI])*inCollisionRatekHz*1000./((Double_t)nEntries) << "," << trigPrescale[bI] << "," << ((Double_t)trigPrescaledFires[bI])*inCollisionRatekHz*1000./((Double_t)nEntries) << "," << std::endl;
  }
  outPrescaleFile.close();
  
  std::cout << "#: Name, PD, SubPD, Final prescale, Fires, Prescaled Fires, Rate at " << (Int_t)inCollisionRatekHz << "kHz (Hz), Prescaled Rate at " << (Int_t)inCollisionRatekHz << "kHz (Hz)" << std::endl;
  for(Int_t bI = 0; bI < nTrig; ++bI){
    std::cout << " " << bI << "/" << nTrig << ": " << trigNames[bI] << ", " << trigPD[bI] << ", " << trigSubPD[bI] << ", " << trigPrescale[bI] << ", " << trigFires[bI] << ", " << trigPrescaledFires[bI] << ", " << ((Double_t)trigFires[bI])*inCollisionRatekHz*1000./((Double_t)nEntries) << ", " << ((Double_t)trigPrescaledFires[bI])*inCollisionRatekHz*1000./((Double_t)nEntries) << std::endl;
  }

  std::cout << std::endl;

  std::cout << "SUPER SUMMARY: PD, Total Prescaled Fires, Rate at " << (Int_t)inCollisionRatekHz << "kHz (Hz)";
  if(doHibin) std::cout << ", Avg. Cent, Avg. RAW+AOD size (MB), Avg. RAW size (MB), Avg. AOD size (MB)" << std::endl;
  else std::cout << std::endl;

  std::map<std::string, int>  uniqueOverlapsCounts;
  std::map<std::string, double>  uniqueOverlapsTotalSizes;
  std::map<std::string, double>  uniqueOverlapsRAWSizes;
  std::map<std::string, double>  uniqueOverlapsAODSizes;

  for(auto const &iter : pdMapToFires){
    std::cout << " SUPER SUMMARY: " << iter.first << ", " << iter.second << ", " << ((Double_t)iter.second)*inCollisionRatekHz*1000./((Double_t)nEntries);
    
    Double_t cent = 0;
    Double_t totalSize = 0;
    Double_t rawSize = 0;
    Double_t aodSize = 0;
    
    if(doHibin){
      for(Int_t cI = 0; cI < nCentBins; ++cI){
	cent += ((centBins[cI] + centBins[cI+1])*0.5)*((Double_t)pdMapCentrality[iter.first].at(cI))/((Double_t)iter.second);

	totalSize += ((Double_t)pdMapCentrality[iter.first].at(cI))/((Double_t)iter.second)*(rawSizes[cI] + aodSizes[cI]);
	rawSize += ((Double_t)pdMapCentrality[iter.first].at(cI))/((Double_t)iter.second)*(rawSizes[cI]);
	aodSize += ((Double_t)pdMapCentrality[iter.first].at(cI))/((Double_t)iter.second)*(aodSizes[cI]);
      }

      std::cout << ", " << cent << ", " << prettyString(totalSize, 2, false) << ", " << prettyString(rawSize, 2, false)  << ", " << prettyString(aodSize, 2, false) << std::endl;
    }
    else std::cout << std::endl;

    for(auto const &iter2 : pdMapToFiresMatchedOtherPD[iter.first]){
      std::cout << "  PD OVERLAP SUMMARY: Fires && w/ " << iter2.first << ": " << iter2.second << "/" << iter.second << "=" << ((Double_t)iter2.second)/((Double_t)iter.second) << std::endl;

      std::string keyStr = iter.first + " && " + iter2.first;
      std::string keyStr2 = iter2.first + " && " + iter.first;

      if(isStrSame(iter.first, iter2.first)) continue;
      if(uniqueOverlapsCounts.count(keyStr) != 0) continue;
      if(uniqueOverlapsCounts.count(keyStr2) != 0) continue;

      int counts = iter2.second;
      uniqueOverlapsCounts[keyStr] = counts;

      if(doHibin){
	Double_t totalOverSize = 0;
	Double_t rawOverSize = 0;
	Double_t aodOverSize = 0;
	for(Int_t cI = 0; cI < nCentBins; ++cI){
	  totalOverSize += ((Double_t)((pdMapCentralityMatchedOtherPD[iter.first])[iter2.first]).at(cI))/((Double_t)iter2.second)*(rawSizes[cI] + aodSizes[cI]);
	  rawOverSize += ((Double_t)((pdMapCentralityMatchedOtherPD[iter.first])[iter2.first]).at(cI))/((Double_t)iter2.second)*(rawSizes[cI]);
	  aodOverSize += ((Double_t)((pdMapCentralityMatchedOtherPD[iter.first])[iter2.first]).at(cI))/((Double_t)iter2.second)*(aodSizes[cI]);
	}
	
	uniqueOverlapsTotalSizes[keyStr] = totalOverSize;
	uniqueOverlapsRAWSizes[keyStr] = rawOverSize;
	uniqueOverlapsAODSizes[keyStr] = aodOverSize;
      }
    }

    std::vector<std::string> keyStrings = {iter.first + " && MBFull",  iter.first + " && MBReduced"};
    std::vector<double> keyRate = {500., 6000.};
    std::vector<double> factor = {1., .65};

    for(unsigned int kI = 0; kI < keyStrings.size(); ++kI){
      uniqueOverlapsCounts[keyStrings.at(kI)] = keyRate.at(kI)*iter.second/(1000.*inCollisionRatekHz);
      uniqueOverlapsTotalSizes[keyStrings.at(kI)] = totalSize*factor.at(kI);
      uniqueOverlapsRAWSizes[keyStrings.at(kI)] = rawSize*factor.at(kI);
      uniqueOverlapsAODSizes[keyStrings.at(kI)] = aodSize*factor.at(kI);
    }
    
    std::vector<std::string> topThreeNames = {"", "", ""};
    std::vector<int> topThreeCounts = {-1, -1, -1};

    for(auto const &iter2 : pdMapToFiresTriggerOtherPD[iter.first]){
      if(iter2.second > topThreeCounts.at(0)){
	topThreeCounts.at(2) = topThreeCounts.at(1);
	topThreeNames.at(2) = topThreeNames.at(1);

	topThreeCounts.at(1) = topThreeCounts.at(0);
	topThreeNames.at(1) = topThreeNames.at(0);

	topThreeCounts.at(0) = iter2.second;
	topThreeNames.at(0) = iter2.first;
      }
      else if(iter2.second > topThreeCounts.at(1)){
	topThreeCounts.at(2) = topThreeCounts.at(1);
	topThreeNames.at(2) = topThreeNames.at(1);

	topThreeCounts.at(1) = iter2.second;
	topThreeNames.at(1) = iter2.first;
      }
      else if(iter2.second > topThreeCounts.at(2)){
	topThreeCounts.at(2) = iter2.second;
	topThreeNames.at(2) = iter2.first;
      }
    }
    for(unsigned int i = 0; i < topThreeNames.size(); ++i){
      std::cout << "  TRIGGER OVERLAP SUMMARY: Fires && w/ " << topThreeNames.at(i) << ": " << topThreeCounts.at(i) << "/" << iter.second << "=" << ((Double_t)topThreeCounts.at(i))/((Double_t)iter.second) << std::endl;
    }
  }

  std::cout << std::endl;

  std::cout << "SUMMARY: SubPD, Total Prescaled Fires, Rate at " << (Int_t)inCollisionRatekHz << "kHz (Hz)" << std::endl;
  for(auto const &iter : subPDMapToFires){
    std::cout << " SUMMARY: " << iter.first << ", " << iter.second << ", " << ((Double_t)iter.second)*inCollisionRatekHz*1000./((Double_t)nEntries) << std::endl;
  }

  std::cout << std::endl;

  std::cout << "OVERLAP SUMMARY 2: Overlapping PDs, Total Counts, Total Rate (Hz), Total Size" << std::endl;

  for(auto const& iter : uniqueOverlapsCounts){
    std::cout << "OVERLAP SUMMARY 2: " << iter.first << ", " << iter.second << ", " << ((Double_t)iter.second)*inCollisionRatekHz*1000./((Double_t)nEntries) << ", " << uniqueOverlapsTotalSizes[iter.first] << std::endl;
  }
  

  std::cout << "SUPER SUMMARY: Total fires, Rate at " << (Int_t)inCollisionRatekHz << " kHz (Hz): " << totalFires << ", " << ((Double_t)totalFires)*inCollisionRatekHz*1000./((Double_t)nEntries) << std::endl;

  std::cout << std::endl;

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc < 5 || argc > 8){
    std::cout << "Usage: ./bin/doPrescaling.exe <inFileName> <prescaleConfigName> <l1XmlFileName> <inCollisionRatekHz> <overrideXML-Optional> <disablePrimeOverride-Optional> <hiBinFile-optional>" << std::endl;
    return 1;
  }

  int retVal = 0;
  if(argc == 5) retVal += doPrescaling(argv[1], argv[2], argv[3], std::stod(argv[4]));
  else if(argc == 6) retVal += doPrescaling(argv[1], argv[2], argv[3], std::stod(argv[4]), std::stoi(argv[5]));
  else if(argc == 7) retVal += doPrescaling(argv[1], argv[2], argv[3], std::stod(argv[4]), std::stoi(argv[5]), std::stoi(argv[6]));
  else if(argc == 8) retVal += doPrescaling(argv[1], argv[2], argv[3], std::stod(argv[4]), std::stoi(argv[5]), std::stoi(argv[6]), argv[7]);
  return retVal;
}
