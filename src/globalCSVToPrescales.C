//cpp dependencies
#include <fstream>
#include <iostream>
#include <map>
#include <string>

//ROOT dependencies
#include "TDatime.h"
#include "TFile.h"
#include "TTree.h"
#include "TObjArray.h"

//Local dependencies
#include "include/checkMakeDir.h"
#include "include/listOfPrimes.h"
#include "include/plotUtilities.h"
#include "include/returnRootFileContentsList.h"
#include "include/stringUtil.h"


int globalCSVToPrescales(const std::string inCSVName, const std::string inRootName = "", const double deviation = 0.1)
{
  if(!checkFile(inCSVName) || inCSVName.find(".csv") == std::string::npos){
    std::cout << "Given input \'" << inCSVName << "\' is invalid return 1" << std::endl;
    return 1;
  }

  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;

  checkMakeDir("output");
  checkMakeDir("output/" + dateStr);

  std::vector<int> collRates;
  std::vector<int> l1PrescalePos;
  std::vector<int> l1RatePos;
  std::vector<int> hltPrescalePos;
  std::vector<int> hltRatePos;
  std::vector<std::map<std::string, int> > l1CollTriggerPrescaleMap;
  std::vector<std::map<std::string, double> > l1CollTriggerRateMap;
  std::vector<std::map<std::string, int> > hltCollTriggerL1PrescaleMap;
  std::vector<std::map<std::string, int> > hltCollTriggerHLTPrescaleMap;
  std::vector<std::map<std::string, double> > hltCollTriggerRateMap;
  std::vector<std::map<std::string, std::string> > hltCollTriggerL1NameMap;
  std::map<std::string, std::string> triggerToPDMap;

  std::ifstream inFile(inCSVName.c_str());
  std::string tempStr;

  std::getline(inFile, tempStr);
  tempStr = tempStr + ",";
  while(tempStr.substr(0,1).find(",") != std::string::npos){tempStr.replace(0,1,"");}
  while(tempStr.find(",,") != std::string::npos){tempStr.replace(tempStr.find(",,"), 2,",");}
  while(tempStr.find(" ") != std::string::npos){tempStr.replace(tempStr.find(" "), 1,"");}
  while(tempStr.find("kHz") != std::string::npos){tempStr.replace(tempStr.find("kHz"), 3,"");}

  std::cout << "'" << tempStr << "'" << std::endl;

  while(tempStr.find(",") != std::string::npos){
    std::map<std::string, int> temp;
    std::map<std::string, std::string> temp2;
    std::map<std::string, double> temp3;

    int num = std::stoi(tempStr.substr(0, tempStr.find(",")));

    collRates.push_back(num);
    l1CollTriggerPrescaleMap.push_back(temp);
    l1CollTriggerRateMap.push_back(temp3);
    hltCollTriggerL1PrescaleMap.push_back(temp);
    hltCollTriggerHLTPrescaleMap.push_back(temp);
    hltCollTriggerRateMap.push_back(temp3);
    hltCollTriggerL1NameMap.push_back(temp2);
    tempStr.replace(0, tempStr.find(",")+1, "");
  }

  std::getline(inFile, tempStr);
  unsigned int pos = 0;
  while(tempStr.find(",") != std::string::npos){
    std::string subStr = tempStr.substr(0, tempStr.find(","));

    if(subStr.find("L1 Prescale") != std::string::npos) l1PrescalePos.push_back(pos);
    else if(subStr.find("L1 Rate (Unprescaled)") != std::string::npos) l1RatePos.push_back(pos);
    else if(subStr.find("HLT Prescale") != std::string::npos) hltPrescalePos.push_back(pos);
    else if(subStr.find("Unprescaled rate") != std::string::npos) hltRatePos.push_back(pos);

    tempStr.replace(0, tempStr.find(",")+1, "");
    ++pos;
  }

  while(std::getline(inFile, tempStr)){
    if(tempStr.size() == 0) continue;

    std::vector<std::string> strVect;
    while(tempStr.find(",") != std::string::npos){
      strVect.push_back(tempStr.substr(0, tempStr.find(",")));
      tempStr.replace(0, tempStr.find(",")+1, "");
    }

    if(strVect.size() < 3) continue;
    if(strVect.at(1).substr(0, 4).find("HLT_") == std::string::npos) continue;
    if(strVect.at(2).substr(0, 3).find("L1_") == std::string::npos) continue;

    for(unsigned int cI = 0; cI < collRates.size(); ++cI){     
      int l1Prescale = std::stoi(strVect.at(l1PrescalePos.at(cI)));
      int hltPrescale = std::stoi(strVect.at(hltPrescalePos.at(cI)));

      int l1PrimeLow = 0;
      if(l1Prescale != 0) l1PrimeLow = getNearestPrimeLow(l1Prescale);
      int l1PrimeHigh = 0;
      if(l1Prescale != 0) l1PrimeHigh = getNearestPrimeHigh(l1Prescale);

      int hltPrimeLow = 0;
      if(hltPrescale != 0) hltPrimeLow = getNearestPrimeLow(hltPrescale);
      int hltPrimeHigh = 0;
      if(hltPrescale != 0) hltPrimeHigh = getNearestPrimeHigh(hltPrescale);

      if(l1PrimeLow != l1Prescale && l1PrimeHigh != l1Prescale && l1Prescale != 0){
	std::cout << "WARNING: L1 prescale for path \'" << strVect.at(2) << "\' is not prime in " << collRates.at(cI) << " kHz. Current: " << l1Prescale << ". Consider " << l1PrimeLow << " or " << l1PrimeHigh << std::endl;
      }

      if(hltPrimeLow != hltPrescale && hltPrimeHigh != hltPrescale && hltPrescale != 0){
	std::cout << "WARNING: HLT prescale for path \'" << strVect.at(1) << "\' is not prime in " << collRates.at(cI) << " kHz. Current: " << hltPrescale << ". Consider " << hltPrimeLow << " or " << hltPrimeHigh << std::endl;
      }

      double l1Rate = 0;
      double hltRate = 0;
      std::string l1RateStr = strVect.at(l1RatePos.at(cI));
      std::string hltRateStr = strVect.at(hltRatePos.at(cI));

      if(l1RateStr.size() != 0 && l1RateStr.find("DIV") == std::string::npos) l1Rate = std::stod(l1RateStr);
      if(hltRateStr.size() != 0 && hltRateStr.find("DIV") == std::string::npos) hltRate = std::stod(hltRateStr);

      

      if(l1CollTriggerPrescaleMap.at(cI).count(strVect.at(2)) == 0){
	(l1CollTriggerPrescaleMap.at(cI))[strVect.at(2)] = l1Prescale;
	(l1CollTriggerRateMap.at(cI))[strVect.at(2)] = l1Rate;

	if(strVect.at(2).find("L1_SingleJet56_BptxAND") != std::string::npos) triggerToPDMap[strVect.at(2)] = "HIHardProbes";
	else if(strVect.at(2).find("L1_SingleMuOpen_BptxAND") != std::string::npos) triggerToPDMap[strVect.at(2)] = "HISingleMuon";
	else triggerToPDMap[strVect.at(2)] = strVect.at(3);
      }
      else{
	if(l1Prescale != (l1CollTriggerPrescaleMap.at(cI))[strVect.at(2)]){
	  std::cout << "WARNING: Discrepancy in prescales for \'" << strVect.at(2) << "\', HLT \'" << strVect.at(1) << "\', collision rate " << collRates.at(cI) << "." << std::endl;
	}
	if(TMath::Abs(l1Rate - (l1CollTriggerRateMap.at(cI))[strVect.at(2)]) > 0.1){	  
	  std::cout << "WARNING: Discrepancy in rates for \'" << strVect.at(2) << "\', HLT \'" << strVect.at(1) << "\', collision rate " << collRates.at(cI) << "." << std::endl;	  
	}
      }

      if(hltCollTriggerHLTPrescaleMap.at(cI).count(strVect.at(1)) == 0){
	(hltCollTriggerL1PrescaleMap.at(cI))[strVect.at(1)] = l1Prescale;
	(hltCollTriggerHLTPrescaleMap.at(cI))[strVect.at(1)] = hltPrescale;
	(hltCollTriggerRateMap.at(cI))[strVect.at(1)] = hltRate;
	(hltCollTriggerL1NameMap.at(cI))[strVect.at(1)] = strVect.at(2);
	triggerToPDMap[strVect.at(1)] = strVect.at(3);
      }
      else{
	if(hltPrescale != (hltCollTriggerHLTPrescaleMap.at(cI))[strVect.at(1)]){
	  std::cout << "WARNING: Discrepancy in prescales for \'" << strVect.at(1) << "\', collision rate " << collRates.at(cI) << "." << std::endl;
	}
	if(TMath::Abs(hltRate - (hltCollTriggerRateMap.at(cI))[strVect.at(1)]) > 0.1){
	  std::cout << "WARNING: Discrepancy in rates for \'" << strVect.at(1) << "\', collision rate " << collRates.at(cI) << "." << std::endl;	  
	}
      }
    }
  }

  for(auto const& trigger : l1CollTriggerRateMap.at(0)){
    for(unsigned int cI = 1; cI < collRates.size(); ++cI){
      double ratio = ((Double_t)collRates.at(cI))/((Double_t)collRates.at(0));
      double reRate = trigger.second*ratio;
      double compRate = (l1CollTriggerRateMap.at(cI))[trigger.first];
      
      if(trigger.second < 1) continue;
      if(TMath::Abs(reRate - compRate)/reRate > .1){
	std::cout << "WARNING: trigger \'" << trigger.first << "\' is not scaling from coll rate \'" << collRates.at(0) << " kHz\' to \'" << collRates.at(cI) << " kHz\', " << trigger.second << "*" << ratio << "=" << reRate << ", " << compRate << std::endl;
      }
    }
  }


  for(auto const& trigger : hltCollTriggerRateMap.at(0)){
    for(unsigned int cI = 1; cI < collRates.size(); ++cI){
      double ratio = ((Double_t)collRates.at(cI))/((Double_t)collRates.at(0));
      double reRate = trigger.second*ratio;
      double compRate = (hltCollTriggerRateMap.at(cI))[trigger.first];
      
      if(trigger.second < 1) continue;
      if(TMath::Abs(reRate - compRate)/reRate > .2){
	std::cout << "WARNING: trigger \'" << trigger.first << "\' is not scaling from coll rate \'" << collRates.at(0) << " kHz\' to \'" << collRates.at(cI) << " kHz\', " << trigger.second << "*" << ratio << "=" << reRate << ", " << compRate << std::endl;
      }
    }
  }


  inFile.close();

  if(inRootName.size() != 0){
    if(!checkFile(inRootName) || inRootName.find(".root") == std::string::npos){
      std::cout << "WARNING: Given inRootName \'" << inRootName << "\' is invalid. skipping rate check" << std::endl;
    }
    else{
    
      TFile* inRootFile_p = TFile::Open(inRootName.c_str(), "READ");
      std::vector<std::string> hltTrees = returnRootFileContentsList(inRootFile_p, "TTree", "HltTree");
      if(hltTrees.size() == 0){
	std::cout << "WARNING: Given inRootName \'" << inRootName << "\' contains no HLT Tree. skipping rate check" << std::endl;
      }	 
      else{
	TTree* hltTree_p = (TTree*)inRootFile_p->Get(hltTrees.at(0).c_str());
	std::vector<std::string> listOfBranches;
	TObjArray* initListOfBranches = (TObjArray*)hltTree_p->GetListOfBranches();

	for(Int_t oI = 0; oI < initListOfBranches->GetEntries(); ++oI){
	  std::string tempBranch = initListOfBranches->At(oI)->GetName();
	  if(tempBranch.find("Prescale") != std::string::npos) continue;
	  
	  listOfBranches.push_back(tempBranch);
	}

	double nEntries = hltTree_p->GetEntries();
	
	for(auto const& rate : l1CollTriggerRateMap.at(0)){
	  if(rate.second == 0) continue;
	  std::string trigName = rate.first;
	  
	  bool isGood = false;
	  for(auto const& branch : listOfBranches){
	    if(isStrSame(branch, trigName)){
	      isGood = true;
	      break;
	    }
	  }

	  if(!isGood){
	    std::cout << "WARNING: Branch \'" << trigName << "\' is missing from inRootFile \'" << inRootName << "\'. skipping..." << std::endl;
	    continue;
	  }
	  
	  double ratio = rate.second/(1000.*collRates.at(0));
	  double tempEntries = hltTree_p->GetEntries(trigName.c_str());
	  double eval = (tempEntries/nEntries - ratio)/ratio;

	  if(TMath::Abs(eval) > deviation) std::cout << "WARNING: Branch \'" << trigName << "\' deviates in root file from prediction by > " << prettyString(deviation*100, 1, false) << "% (" << prettyString(eval*100, 1, false) << "%), prediction " << ratio << ",  root file " << tempEntries/nEntries << std::endl;
	  
	  double rateOut = tempEntries*(1000.*collRates.at(0))/nEntries;
	  if(trigName.find("L1_MinimumBias") != std::string::npos) rateOut = collRates.at(0)*1000.;
	  std::cout << "CSV Coll " << collRates.at(0) << " kHz: " << trigName << ", " << rateOut << std::endl;
	}	

	for(auto const& rate : hltCollTriggerRateMap.at(0)){
	  if(rate.second == 0) continue;
	  std::string trigName = rate.first;
	  
	  bool isGood = false;
	  for(auto const& branch : listOfBranches){
	    if(isStrSame(branch, trigName)){
	      isGood = true;
	      break;
	    }
	  }

	  if(!isGood){
	    std::cout << "WARNING: Branch \'" << trigName << "\' is missing from inRootFile \'" << inRootName << "\'. skipping..." << std::endl;
	    continue;
	  }
	  
	  double ratio = rate.second/(1000.*collRates.at(0));
	  double tempEntries = hltTree_p->GetEntries(trigName.c_str());
	  double eval = (tempEntries/nEntries - ratio)/ratio;

	  if(TMath::Abs(eval) > deviation) std::cout << "WARNING: Branch \'" << trigName << "\' deviates in root file from prediction by > " << prettyString(deviation*100, 1, false) << "% (" << prettyString(eval*100, 1, false) << "%), prediction " << ratio << ",  root file " << tempEntries/nEntries << std::endl;

	  double rateOut = tempEntries*(1000.*collRates.at(0))/nEntries;
	  if(trigName.find("L1_MinimumBias") != std::string::npos) rateOut = collRates.at(0)*1000.;
	  std::cout << "CSV Coll " << collRates.at(0) << " kHz: " << trigName << ", " << rateOut << std::endl;
	}	
      }

      inRootFile_p->Close();
      delete inRootFile_p;
    }
  }

  for(unsigned int cI = 0; cI < collRates.size(); ++cI){
    std::ofstream l1File(("output/" + dateStr + "/l1" + std::to_string(collRates.at(cI)) + "kHz_" + dateStr + ".csv").c_str());
    for(auto const& l1Iter : l1CollTriggerPrescaleMap.at(cI)){
      std::string trigName = l1Iter.first;
      while(trigName.find("_") != std::string::npos){trigName.replace(trigName.find("_"), 1, "");}
      trigName = "HLT_" + trigName + "_v1";

      int prescale = l1Iter.second;
      if(prescale == 0) prescale = -1;
      else if(triggerToPDMap[l1Iter.first].size() == 0) prescale = -1;

      l1File << trigName << ", " << prescale << ", " << triggerToPDMap[l1Iter.first] << ", " << triggerToPDMap[l1Iter.first] << ",-1" << std::endl;
    }

    l1File.close();

    std::ofstream hltFile(("output/" + dateStr + "/hlt" + std::to_string(collRates.at(cI)) + "kHz_" + dateStr + ".csv").c_str());
    for(auto const& hltIter : hltCollTriggerHLTPrescaleMap.at(cI)){
      int prescale = (hltCollTriggerL1PrescaleMap.at(cI))[hltIter.first]*hltIter.second;
      if(prescale == 0) prescale = -1;
      else if(triggerToPDMap[hltIter.first].size() == 0) prescale = -1;

      hltFile << hltIter.first << ", " << prescale << ", " << triggerToPDMap[hltIter.first] << ", " << triggerToPDMap[hltIter.first] << ",-1" << std::endl;
    }
    hltFile.close();

    std::ofstream hltFileTSV(("output/" + dateStr + "/hlt" + std::to_string(collRates.at(cI)) + "kHz_" + dateStr + ".txt").c_str());

    std::map<std::string, std::vector<std::string> > pdToTSVStr;
    std::vector<unsigned int> sizes = {0, 0, 0, 0, 0, 4};
    for(auto const& hltIter : hltCollTriggerHLTPrescaleMap.at(cI)){
      bool dontKeep = triggerToPDMap[hltIter.first].find("HIHardProbes") == std::string::npos;
      //      bool dontKeep = triggerToPDMap[hltIter.first].find("HIHardProbes") == std::string::npos && triggerToPDMap[hltIter.first].find("HIHardProbes") == std::string::npos;
      if(dontKeep) continue;

      if(pdToTSVStr.count(triggerToPDMap[hltIter.first]) == 0) pdToTSVStr[triggerToPDMap[hltIter.first]] = {};

      if(hltIter.first.size() > sizes.at(0)) sizes.at(0) = hltIter.first.size();
      if((hltCollTriggerL1NameMap.at(cI))[hltIter.first].size() > sizes.at(1)) sizes.at(1) = (hltCollTriggerL1NameMap.at(cI))[hltIter.first].size();
      if(triggerToPDMap[hltIter.first].size() > sizes.at(2)) sizes.at(2) = triggerToPDMap[hltIter.first].size();
      if(std::to_string((hltCollTriggerL1PrescaleMap.at(cI))[hltIter.first]).size() > sizes.at(3)) sizes.at(3) = std::to_string((hltCollTriggerL1PrescaleMap.at(cI))[hltIter.first]).size();
      if(std::to_string(hltIter.second).size() > sizes.at(4)) sizes.at(4) = std::to_string(hltIter.second).size();
    }

    for(auto const& hltIter : hltCollTriggerHLTPrescaleMap.at(cI)){
      //      bool dontKeep = triggerToPDMap[hltIter.first].find("HIHardProbes") == std::string::npos && triggerToPDMap[hltIter.first].find("HIHardProbes") == std::string::npos;
      bool dontKeep = triggerToPDMap[hltIter.first].find("HIHardProbes") == std::string::npos;
      if(dontKeep) continue;

      std::string numStr = "3000";
      if(triggerToPDMap[hltIter.first].find("HIHardProbesPer") != std::string::npos) numStr = "1500";
      
      std::vector<std::string> strings = {hltIter.first, (hltCollTriggerL1NameMap.at(cI))[hltIter.first], triggerToPDMap[hltIter.first], std::to_string((hltCollTriggerL1PrescaleMap.at(cI))[hltIter.first]), std::to_string(hltIter.second), numStr};

      std::string finalString = "";
      for(unsigned int sI = 0; sI < sizes.size()-1; ++sI){
	while(strings.at(sI).size() < sizes.at(sI)){strings.at(sI) = strings.at(sI) + " ";}
	finalString = finalString + strings.at(sI) + " ";
      }
      finalString = finalString + " " + strings.at(strings.size()-1);

      pdToTSVStr[triggerToPDMap[hltIter.first]].push_back(finalString);
    }

    for(auto const& pd : pdToTSVStr){
      for(auto const& finalString : pd.second){
	hltFileTSV << finalString << std::endl;
      }
    }
    hltFileTSV.close();
  }

  std::map<std::string, double> l1rates = l1CollTriggerRateMap.at(0);
  std::map<std::string, double> hltrates = hltCollTriggerRateMap.at(0);

  std::cout << "L1 TRIGGER, NAME, FRACTION," << std::endl;
  for(auto const& l1 : l1rates){
    std::cout << "L1 TRIGGER, " << l1.first << ", " << l1.second/(1000.*collRates.at(0)) << std::endl;
  }
  std::cout << std::endl;

  std::cout << "HLT TRIGGER, NAME, FRACTION," << std::endl;
  for(auto const& hlt : hltrates){
    std::cout << "HLT TRIGGER, " << hlt.first << ", " << hlt.second/(1000.*collRates.at(0)) << std::endl;
  }

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc < 2 || argc > 4){
    std::cout << "Usage: ./bin/globalCSVToPrescales.exe <inCSVName> <inRootName-Opt> <deviation-Opt>" << std::endl;
    return 1;
  }

  int retVal = 0;
  if(argc == 2) retVal += globalCSVToPrescales(argv[1]);
  else if(argc == 3) retVal += globalCSVToPrescales(argv[1], argv[2]);
  else if(argc == 4) retVal += globalCSVToPrescales(argv[1], argv[2], std::stod(argv[3]));
  return retVal;
}
