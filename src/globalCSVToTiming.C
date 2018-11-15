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


int globalCSVToTiming(const std::string inCSVName, const std::string timingCSVName, const std::string origPrescaleCSV, const int inCollRate)
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

  bool isGoodInColl = false;
  int collPos = 0;
  for(auto const& coll : collRates){
    if(coll == inCollRate){
      isGoodInColl = true;
      break;
    }
    ++collPos;
  }

  if(!isGoodInColl){
    std::cout << "ERROR: \'" << inCollRate << "\' is not found in collRates: ";
    for(auto const& coll : collRates){
      std::cout << coll << ", ";
    }
    std::cout << std::endl; 
    std::cout << "Return 1." << std::endl;
    inFile.close();
    return 1;
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

    if(tempStr.find("DIV") != std::string::npos) continue;
    if(tempStr.find("Err") != std::string::npos) continue;

    //    std::cout << tempStr << std::endl;

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

      
      double l1Rate = 0;
      double hltRate = 0;
      std::string l1RateStr = strVect.at(l1RatePos.at(cI));
      std::string hltRateStr = strVect.at(hltRatePos.at(cI));

      
      if(l1RateStr.size() != 0 && l1RateStr.find("DIV") == std::string::npos && l1RateStr.find("Err") == std::string::npos) l1Rate = std::stod(l1RateStr);
      if(hltRateStr.size() != 0 && hltRateStr.find("DIV") == std::string::npos && hltRateStr.find("Err") == std::string::npos) hltRate = std::stod(hltRateStr);

            

      if(l1CollTriggerPrescaleMap.at(cI).count(strVect.at(2)) == 0){
	(l1CollTriggerPrescaleMap.at(cI))[strVect.at(2)] = l1Prescale;
	(l1CollTriggerRateMap.at(cI))[strVect.at(2)] = l1Rate;
      }

      
      if(hltCollTriggerHLTPrescaleMap.at(cI).count(strVect.at(1)) == 0){
	(hltCollTriggerL1PrescaleMap.at(cI))[strVect.at(1)] = l1Prescale;
	(hltCollTriggerHLTPrescaleMap.at(cI))[strVect.at(1)] = hltPrescale;
	(hltCollTriggerRateMap.at(cI))[strVect.at(1)] = hltRate;
	(hltCollTriggerL1NameMap.at(cI))[strVect.at(1)] = strVect.at(2);
	triggerToPDMap[strVect.at(1)] = strVect.at(3);
      }
    }
  }

  inFile.close();


  inFile.open(timingCSVName.c_str());
  std::map<std::string, std::vector<std::string> > mapHLTToTiming;

  std::getline(inFile, tempStr);
  std::getline(inFile, tempStr);

  std::cout << tempStr << std::endl;
  tempStr.replace(0, tempStr.find(",")+1, "");
  std::string l1MBBit = tempStr.substr(0, tempStr.find(","));
  tempStr.replace(0, tempStr.find(",")+1, "");
  std::string l1MBRateStr = tempStr.substr(0, tempStr.find("."));
  while(l1MBRateStr.find("\"") != std::string::npos){l1MBRateStr.replace(l1MBRateStr.find("\""), 1, "");}
  while(l1MBRateStr.find(",") != std::string::npos){l1MBRateStr.replace(l1MBRateStr.find(","), 1, "");}
  std::cout << l1MBBit << ": " << "\'" << l1MBRateStr << "\'" << std::endl;

  const double l1MBCorr = 1.15;
  const double l1MBRate = l1MBCorr*std::stod(l1MBRateStr);

  while(std::getline(inFile, tempStr)){
    while(tempStr.find(" ") != std::string::npos){tempStr.replace(tempStr.find(" "), 1, "");}
    while(tempStr.find("\t") != std::string::npos){tempStr.replace(tempStr.find("\t"), 1, "");}
    if(tempStr.size() < 4) continue;
    if(tempStr.substr(0,4).find("HLT_") == std::string::npos) continue;

    if(tempStr.substr(tempStr.size()-1, 1).find(",") == std::string::npos) tempStr = tempStr + ",";

    //    std::cout << tempStr << std::endl;
    std::string trigStr = tempStr.substr(0, tempStr.find(","));
    tempStr.replace(0, tempStr.find(",")+1, "");
   
    std::vector<std::string> trigVect;
    while(tempStr.find(",") != std::string::npos){
      trigVect.push_back(tempStr.substr(0, tempStr.find(",")));
      tempStr.replace(0, tempStr.find(",")+1, "");
    }
    if(trigVect.size() == 0) continue;
    //    std::cout << " " << trigVect.at(0) << std::endl;
    if(trigVect.at(0).size() < 2) continue;
    if(trigVect.at(0).substr(0,3).find("L1_") == std::string::npos) continue;
    if(trigVect.at(1).size() == 0) continue;

    mapHLTToTiming[trigStr] = trigVect;
    //    std::cout << trigStr << std::endl;
  }

  inFile.close();

  int collPosOrig = -1;
  const std::string collSearchStr = "HI" + std::to_string(inCollRate) + "kHz";
  std::map<std::string, double> origPrescaleMap;
  inFile.open(origPrescaleCSV.c_str());
  std::getline(inFile, tempStr);
  if(tempStr.substr(tempStr.size()-1, 1).find(",") == std::string::npos) tempStr = tempStr + ",";
  
  int tempPos = 0;
  while(tempStr.find(",") != std::string::npos){
    std::string interv = tempStr.substr(0, tempStr.find(","));

    if(isStrSame(interv, collSearchStr)){
      collPosOrig = tempPos;
      break;
    }

    tempPos++;
    tempStr.replace(0, tempStr.find(",")+1, "");
  }

  if(collPosOrig < 0){
    std::cout << "Cannot find original coll. \'" << inCollRate << "\' in file \'" << origPrescaleCSV << "\'" << std::endl;
    inFile.close();
    return 1;
  }

  while(std::getline(inFile, tempStr)){
    std::cout << tempStr << std::endl;
    if(tempStr.substr(tempStr.size()-1, 1).find(",") == std::string::npos) tempStr = tempStr + ",";

    std::vector<std::string> collVect;
    while(tempStr.find(",") != std::string::npos){
      collVect.push_back(tempStr.substr(0, tempStr.find(",")));
      tempStr.replace(0, tempStr.find(",")+1, "");
    }
    
    origPrescaleMap[collVect.at(0)] = std::stod(collVect.at(collPosOrig));    
  }

  inFile.close();


  int val = -1;
  int mapSize = 0;
  for(auto const& map : mapHLTToTiming){
    if(val >= 0){
      if(val != (int)map.second.size()) std::cout << "WARNING: MAP VECTOR SIZES NOT SAME" << std::endl;
    }
    else{
      val = map.second.size();
      
      std::cout << map.first << ", ";
      for(auto const v : map.second){
	std::cout << v << ", ";
      }
      std::cout << std::endl;
      
    }
    ++mapSize;
  }
  std::cout << "Size of map: " << mapSize << std::endl;

  
  for(unsigned int cI = 0; cI < collRates.size(); ++cI){
    const std::string outFileNameCSV = "output/" + dateStr + "/timing_" + std::to_string(collRates.at(cI)) + "kHz.csv";
    const std::string outFileNameTSV = "output/" + dateStr + "/timing_" + std::to_string(collRates.at(cI)) + "kHz.tsv";

    std::vector<double> totalTime;
    std::vector<std::vector<std::string> > totalStrVect;
    std::vector<std::string> topLineStrings = {"HLT Path (Sorted by timing)", "PD", "L1 Path", "L1 Rate Delivered (Hz)", "Timing/L1 (ms)", "Timing/L1 (ms) * L1 Rate"};
    std::vector<unsigned int> sizes = {(unsigned int)topLineStrings.at(0).size(), (unsigned int)topLineStrings.at(1).size(), (unsigned int)topLineStrings.at(2).size(), (unsigned int)topLineStrings.at(3).size(), (unsigned int)topLineStrings.at(4).size(), (unsigned int)topLineStrings.at(5).size()};

    for(auto const& map : mapHLTToTiming){
      std::string path = map.first;
      std::vector<std::string> tempTrig = map.second;

      if(l1CollTriggerPrescaleMap.at(cI)[tempTrig.at(0)] == 0) continue;
      if(hltCollTriggerHLTPrescaleMap.at(cI)[path] == 0) continue;

      const double l1Rate = std::stod(tempTrig.at(1))*((Double_t)collRates.at(cI)*1000)/l1MBRate;
      const double l1RatePrescaled = l1Rate/((Double_t)l1CollTriggerPrescaleMap.at(cI)[tempTrig.at(0)]);

      const double hltPrescaleRatio = ((Double_t)hltCollTriggerHLTPrescaleMap.at(cI)[path])/((Double_t)origPrescaleMap[path]);
      
      const double newTiming = std::stod(tempTrig.at(tempTrig.size()-2))/hltPrescaleRatio;
      const double newTimingTimesL1Prescaled = newTiming*l1RatePrescaled;
    
      totalTime.push_back(newTimingTimesL1Prescaled);
      totalStrVect.push_back({path, triggerToPDMap[path], tempTrig.at(0), prettyString(l1RatePrescaled, 1, false), prettyString(newTiming, 1, false), prettyString(newTimingTimesL1Prescaled, 1, false)});

      for(unsigned int sI = 0; sI < sizes.size(); ++sI){
	if(totalStrVect.at(totalStrVect.size()-1).at(sI).size() > sizes.at(sI)) sizes.at(sI) = totalStrVect.at(totalStrVect.size()-1).at(sI).size();
      }
    }

    for(unsigned int sI = 0; sI < totalTime.size(); ++sI){
      for(unsigned int sI2 = sI + 1; sI2 < totalTime.size(); ++sI2){
	if(totalTime.at(sI) < totalTime.at(sI2)){
	  double tempTotal = totalTime.at(sI);
	  std::vector<std::string> tempTotalStrVect = totalStrVect.at(sI);

	  totalTime.at(sI) = totalTime.at(sI2);
	  totalStrVect.at(sI) = totalStrVect.at(sI2);


	  totalTime.at(sI2) = tempTotal;
	  totalStrVect.at(sI2) = tempTotalStrVect;
	}
      }
    }

    std::ofstream outFileCSV(outFileNameCSV.c_str());
    std::string outStr = "";
    
    for(unsigned int sI = 0; sI < topLineStrings.size(); ++sI){
      std::string temp = topLineStrings.at(sI);
      outStr = outStr + temp + ",";
    }

    outFileCSV << outStr << std::endl;
    for(auto const& sI : totalStrVect){
      outStr = "";
      for(auto const& sI2 : sI){
	outStr = outStr + sI2 + ",";
      }
      outFileCSV << outStr << std::endl;
    }
    outFileCSV.close();

    

    std::ofstream outFileTSV(outFileNameTSV.c_str());
    outStr = "";
    for(unsigned int sI = 0; sI < topLineStrings.size(); ++sI){
      std::string temp = topLineStrings.at(sI);
      while(temp.size() < sizes.at(sI)+1){temp = temp + " ";};
      outStr = outStr + temp;
    }

    outFileTSV << outStr << std::endl;
    for(auto const& sI : totalStrVect){
      outStr = "";
      
      for(unsigned int sI2 = 0; sI2 < sI.size(); ++sI2){
	std::string temp = sI.at(sI2);
	if(sI2 < sizes.size()-1){
	  while(temp.size() < sizes.at(sI2)+1){temp = temp + " ";};
	}
	outStr = outStr + temp;
      }
      outFileTSV << outStr << std::endl;
    }
    outFileTSV.close();

  }

  inFile.close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 5){
    std::cout << "Usage: ./bin/globalCSVToTiming.exe <inCSVName> <timingCSVName> <origPrescaleCSV> <inCollRate>" << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += globalCSVToTiming(argv[1], argv[2], argv[3], std::stoi(argv[4]));
  return retVal;
}
