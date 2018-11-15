//cpp dependencies
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

//ROOT dependencies
#include "TDatime.h"
#include "TMath.h"

//Local dependencies
#include "include/checkMakeDir.h"
#include "include/listOfPrimes.h"

std::string cleanStr(std::string inStr)
{
  while(inStr.find(" ") != std::string::npos){inStr.replace(inStr.find(" "), 1, "");}
  while(inStr.find("\t") != std::string::npos){inStr.replace(inStr.find("\t"), 1, "");}
  if(inStr.size() != 0){
    if(inStr.substr(inStr.size()-1, 1).find(",") == std::string::npos) inStr = inStr + ",";
  }

  return inStr;
}

std::vector<std::string> strToVect(std::string inStr)
{
  std::vector<std::string> retVect;
  while(inStr.find(",") != std::string::npos){
    retVect.push_back(inStr.substr(0, inStr.find(",")));
    inStr.replace(0, inStr.find(",")+1, "");
  }
  if(inStr.size() != 0) retVect.push_back(inStr);

  return retVect;
}

int quickScale(const std::string inFileName)
{
  if(!checkFile(inFileName) || inFileName.find(".csv") == std::string::npos){
    std::cout << "ERROR: inFileName \'" << inFileName << "\' is invalid. return 1" << std::endl;
    return 1;
  }

  std::ifstream inFile(inFileName.c_str());
  std::string tempStr;

  while(std::getline(inFile, tempStr)){
    tempStr = cleanStr(tempStr);
    if(tempStr.size() != 0) break;
  }
  tempStr = cleanStr(tempStr);

  const double target = 1.0;
  std::vector<std::string> collRates = strToVect(tempStr);
  std::vector<std::vector<std::string> > highPtStrings;
  std::vector<std::string> collRates2;
  std::vector<unsigned int> hltPrescalePos;
  std::vector<unsigned int> hltRatePos;

  int counts = 0;
  std::cout << "Coll rates: ";
  for(unsigned int cI = 0; cI < collRates.size(); ++cI){
    if(collRates.at(cI).size() == 0) continue;
    collRates.at(cI).replace(collRates.at(cI).find("kHz"), collRates.at(cI).size(), "");
    std::cout << collRates.at(cI) << ", ";
    collRates2.push_back(collRates.at(cI));
    ++counts;
  }
  std::cout << std::endl;

  while(std::getline(inFile, tempStr)){
    if(tempStr.size() != 0) break;
  }

  std::vector<std::string> topLine = strToVect(tempStr);
  for(unsigned int tI = 0; tI < topLine.size(); ++tI){
    if(topLine.at(tI).find("HLT Prescale") != std::string::npos) hltPrescalePos.push_back(tI);
    else if(topLine.at(tI).find("Prescaled rate") != std::string::npos) hltRatePos.push_back(tI);
  }

  std::cout << "Sizes: " << counts << ", " << hltPrescalePos.size() << ", " << hltRatePos.size() << std::endl;

  bool gotHighPt = false;
  while(std::getline(inFile, tempStr)){
    tempStr = cleanStr(tempStr);
    std::vector<std::string> tempVect = strToVect(tempStr);

    if(tempVect.at(0).find("HighPT") != std::string::npos){
      gotHighPt = true;
      continue;
    }
    else if(tempVect.at(0).size() != 0 && gotHighPt) break;

    if(!gotHighPt) continue;

    highPtStrings.push_back(tempVect);
  }

  inFile.close();

  std::string outFileName = inFileName;
  outFileName.replace(outFileName.find("."), outFileName.size(), "");
  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;

  outFileName = outFileName + "_ReScale_" + dateStr + ".csv";

  std::ofstream outFile(outFileName.c_str());

  outFile << "Trigger,";
  for(unsigned int cI = 0; cI < collRates2.size(); ++cI){
    if(std::stod(collRates2.at(cI)) > 31) break;
    outFile << collRates2.at(cI) << ",";
    //    outFile << "Prescale,Rate,Target,NewPrescale,";
  }
  outFile << std::endl;

  for(unsigned int lI = 0; lI < highPtStrings.size(); ++lI){
    outFile << highPtStrings.at(lI).at(1) << ",";
    if(highPtStrings.at(lI).at(1).size() != 0){
      for(unsigned int cI = 0; cI < collRates2.size(); ++cI){
	if(std::stod(collRates2.at(cI)) > 31) break;
	//	std::cout << collRates2.at(cI) << ", " << highPtStrings.at(lI).at(1) << ", \'" << highPtStrings.at(lI).at(hltRatePos.at(cI)) << "\', \'" << highPtStrings.at(lI).at(hltPrescalePos.at(cI)) << "\'" << std::endl;

	double hltRate = std::stod(highPtStrings.at(lI).at(hltRatePos.at(cI)));
	int hltPrescale = std::stoi(highPtStrings.at(lI).at(hltPrescalePos.at(cI)));
	
	int newPrescale = 1;
	if(hltPrescale == 0) newPrescale = 0;
	else if(hltPrescale != 1) newPrescale = getNearestPrime(hltPrescale*hltRate/target);
	if(newPrescale == -1){
	  newPrescale = hltPrescale;
	  //	  std::cout << " hltPrescale, hltRate, target: " << hltPrescale << ", " << hltRate << ", " << target << std::endl;
	}

	outFile << newPrescale << ",";
	//	outFile << hltPrescale << ", " << hltRate << ", " << target << ", " << newPrescale << ",";
      }
      outFile << std::endl;
    }
    else{
      for(unsigned int cI = 0; cI < collRates2.size(); ++cI){
	if(std::stod(collRates2.at(cI)) > 31) break;
	outFile << "," << "," << "," <<",";
	outFile << ",";
      }
      outFile << std::endl;
    }

  }

  outFile.close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 2){
    std::cout << "Usage: ./bin/quickScale.exe <inFileName>. return 1" << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += quickScale(argv[1]);
  return retVal;
}
