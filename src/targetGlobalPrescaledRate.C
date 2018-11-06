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
#include "include/returnRootFileContentsList.h"
#include "include/stringUtil.h"
#include "include/plotUtilities.h"

int targetGlobalPrescaledRate(const std::string inFileName, const Int_t mbRateKHz, const Double_t globalRateTarget)
{
  if(!checkFile(inFileName)){
    std::cout << "Warning: Given inFileName \'" << inFileName << "\' is not a valid file. return 1" << std::endl;
    return 1;
  }
  else if(inFileName.find(".root") == std::string::npos){
    std::cout << "Warning: Given inFileName \'" << inFileName << "\' is not a valid .root file. return 1" << std::endl;
    return 1;
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

  //Pick Branches
  std::vector<std::string> trigNames;
  TObjArray* initListOfBranches = hltTree_p->GetListOfBranches();
  for(Int_t bI = 0; bI < initListOfBranches->GetEntries(); ++bI){
    std::string branchName = initListOfBranches->At(bI)->GetName();

    if(branchName.find("Prescl") != std::string::npos) continue;
    if(branchName.size() < 4) continue;
    if(branchName.substr(0,4).find("HLT_") == std::string::npos) continue;

    trigNames.push_back(branchName);
  }

  const Int_t nTrig = trigNames.size();
  Int_t trigVal[nTrig];
  Int_t trigFires[nTrig];

  hltTree_p->SetBranchStatus("*", 0);
  for(Int_t bI = 0; bI < nTrig; ++bI){
    hltTree_p->SetBranchStatus(trigNames.at(bI).c_str(), 1);
    hltTree_p->SetBranchAddress(trigNames.at(bI).c_str(), &(trigVal[bI]));

    trigFires[bI] = 0;
  }

  const Int_t nEntries = hltTree_p->GetEntries();
  const Int_t nDiv = TMath::Max(1, nEntries/20);

  std::cout << "Processing " << nEntries << "..." << std::endl;
  for(Int_t entry = 0; entry < nEntries; ++entry){
    if(entry%nDiv == 0) std::cout << " Entry " << entry << "/" << nEntries << std::endl;

    hltTree_p->GetEntry(entry);

    for(Int_t bI = 0; bI < nTrig; ++bI){
      if(trigVal[bI] == 1){
	++trigFires[bI];
      }
    }
  }

  inFile_p->Close();
  delete inFile_p;
  
  std::cout << "#: Name, Fraction, Rate from MB (" << mbRateKHz << " kHz), Prescale to target (" << prettyString(globalRateTarget, 2, false) << ")" << std::endl;
  for(Int_t bI = 0; bI < nTrig; ++bI){
    Double_t frac = ((Double_t)trigFires[bI])/((Double_t)nEntries);
    Double_t rate = ((Double_t)mbRateKHz*1000.)*frac;

    std::cout << " " << bI << "/" << nTrig << ": " << trigNames[bI] << ", " << frac << ", " << rate << ", " << getNearestPrime((Int_t)rate/globalRateTarget) << std::endl;
  }
  std::cout << std::endl;

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 4){
    std::cout << "Usage: ./bin/targetGlobalPrescaledRate.exe <inFileName> <mbRateKHz> <globalRateTarget>" << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += targetGlobalPrescaledRate(argv[1], std::stoi(argv[2]), std::stod(argv[3]));
  return retVal;
}
