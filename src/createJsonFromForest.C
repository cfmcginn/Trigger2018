//cpp dependencies
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>

//ROOT dependencies
#include "TFile.h"
#include "TTree.h"
#include "TDatime.h"

//Local dependencies
#include "include/checkMakeDir.h"
#include "include/returnRootFileContentsList.h"
#include "include/stringUtil.h"

int createJsonFromL1(const std::string inFileName)
{
  if(!checkFile(inFileName)){
    std::cout << "inFileName \'" << inFileName << "\' is not valid. return 1" << std::endl;
    return 1;
  }
  else if(inFileName.find(".root") == std::string::npos){
    std::cout << "inFileName \'" << inFileName << "\' is not valid. return 1" << std::endl;
    return 1;
  }

  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;
  
  std::string outFileName = inFileName;
  while(outFileName.find("/") != std::string::npos){outFileName.replace(0, outFileName.find("/")+1, "");}
  if(outFileName.find(".root") != std::string::npos) outFileName.replace(outFileName.find(".root"), 5, "");

  checkMakeDir("output");
  checkMakeDir("output/" + dateStr);
  
  outFileName = "output/" + dateStr + "/" + outFileName + "_JSON_" + dateStr + ".txt";

  std::map<UInt_t, std::map<UInt_t, UInt_t> > runLumisMap;

  TFile* inFile_p = new TFile(inFileName.c_str(), "READ");
  std::vector<std::string> fileList = returnRootFileContentsList(inFile_p, "TTree", "");
  int eventTreePos = -1;

  for(unsigned int fI = 0; fI < fileList.size(); ++fI){
    if(fileList.at(fI).find("hlt") != std::string::npos && fileList.at(fI).find("HltTree") != std::string::npos) eventTreePos = (int)fI;
    else if(fileList.at(fI).find("hiEvt") != std::string::npos && fileList.at(fI).find("HiTree") != std::string::npos) eventTreePos = (int)fI;
  }

  TTree* eventTree_p = NULL;
  if(eventTreePos >= 0) eventTree_p = (TTree*)inFile_p->Get(fileList.at(eventTreePos).c_str());
  else{
    std::cout << "Has no event tree in \'" << inFileName << "\'. return 1" << std::endl;

    inFile_p->Close();
    delete inFile_p;

    return 1;
  }

  UInt_t runHLT_, lumiHLT_;

  UInt_t runHI_, lumiHI_;

  bool isHI = fileList.at(eventTreePos).find("hiEvt") != std::string::npos;
  

  if(!isHI){
    std::cout << "USING HLT TREE" << std::endl;
    eventTree_p->SetBranchStatus("*", 0);
    eventTree_p->SetBranchStatus("Run", 1);
    eventTree_p->SetBranchStatus("LumiBlock", 1);

    eventTree_p->SetBranchAddress("Run", &runHLT_);
    eventTree_p->SetBranchAddress("LumiBlock", &lumiHLT_);
  }
  else{
    std::cout << "USING HI TREE" << std::endl;
    eventTree_p->SetBranchStatus("*", 0);
    eventTree_p->SetBranchStatus("run", 1);
    eventTree_p->SetBranchStatus("lumi", 1);

    eventTree_p->SetBranchAddress("run", &runHI_);
    eventTree_p->SetBranchAddress("lumi", &lumiHI_);
  }

  const Int_t nEntries = eventTree_p->GetEntries();
  const Int_t nDiv = TMath::Max(1, nEntries/20);

  std::cout << "Processing " << nEntries << "..." << std::endl;
  for(Int_t entry = 0; entry < nEntries; ++entry){
    if(entry%nDiv == 0) std::cout << " Entry " << entry << "/" << nEntries << std::endl;

    eventTree_p->GetEntry(entry);

    std::map<UInt_t, UInt_t> temp;
    if(!isHI){
      temp = runLumisMap[(UInt_t)runHLT_];
      temp[(UInt_t)lumiHLT_] = 1;
      runLumisMap[(UInt_t)runHLT_] = temp;
    }
    else{
      temp = runLumisMap[(UInt_t)runHI_];
      temp[(UInt_t)lumiHI_] = 1;
      runLumisMap[(UInt_t)runHI_] = temp;
    }    
  }
  
  inFile_p->Close();
  delete inFile_p;

  std::vector<UInt_t> sortedRuns;
  std::vector< std::vector<UInt_t> > sortedLumis;

  for(auto const &runs : runLumisMap){
    sortedRuns.push_back(runs.first);    
    sortedLumis.push_back({});
  }

  std::sort(std::begin(sortedRuns), std::end(sortedRuns));

  unsigned int i = 0;
  for(auto const &run : sortedRuns){
    
    std::map<UInt_t, UInt_t> temp = runLumisMap[run];
    for(auto const &lumis : temp){
      sortedLumis[i].push_back(lumis.first);
    }

    std::sort(std::begin(sortedLumis[i]), std::end(sortedLumis[i]));
    ++i;
  }
  
  i = 0;  

  std::ofstream outFile(outFileName.c_str());

  std::cout << "{";
  outFile << "{";
  bool passedFirst = false;
  while(i < sortedRuns.size()){
    std::string lumiStr = "";
    std::string lumiStr2 = "";

    std::vector<UInt_t> temp = sortedLumis.at(i);
    for(auto const &lumis : temp){
      lumiStr = lumiStr + std::to_string(lumis) + ",";
    }

    std::string lumi1 = "";
    std::string lumi2 = "";

    while(lumiStr.find(",") != std::string::npos){
      if(lumi1.size() == 0){
	lumi1 = lumiStr.substr(0, lumiStr.find(","));
	lumiStr2 = "[[" + lumi1 + ", ";
      }
      else if(lumi2.size() == 0) lumi2 = lumiStr.substr(0, lumiStr.find(","));
      else{
	std::string testLumi = lumiStr.substr(0,lumiStr.find(","));
	//	std::cout << "lumi1, lumi2, testlumi: " << lumi1 << ", " << lumi2 << ", " << testLumi << std::endl;
	int lumi2Int = std::stoi(lumi2);
	if(isStrSame(testLumi, std::to_string(lumi2Int+1))){
	  lumi2 = testLumi;
	  //	  std::cout << " Outcome open" << std::endl;
	}
	else{
	  //	  std::cout << " Outcome close" << std::endl;
	  lumiStr2 = lumiStr2 + lumi2 + "], [" + testLumi + ", ";
	  lumi2 = "";
	  lumi1 = testLumi;
	}
      }

      lumiStr.replace(0,lumiStr.find(",")+1, "");
    }

    if(lumi2.size() == 0) lumiStr2 = lumiStr2.substr(0,lumiStr2.size()-2);
    else lumiStr2 = lumiStr2 + lumi2;
    lumiStr2 = lumiStr2 + "]]";

    if(lumiStr2.size() != 0){
      if(passedFirst){
	std::cout << ", ";
	outFile << ", ";
      }

      std::cout << "\"" << sortedRuns.at(i) << "\": " << lumiStr2;
      outFile << "\"" << sortedRuns.at(i) << "\": " << lumiStr2;
      passedFirst = true;
    }
    ++i;
  }
  std::cout << "}" << std::endl;
  outFile << "}" << std::endl;

  outFile.close();

  return 0;
}


int main(int argc, char* argv[])
{
  if(argc != 2){
    std::cout << "Usage: ./bin/createJsonFromL1.exe <inFileName>" << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += createJsonFromL1(argv[1]);
  return retVal;
}
  
