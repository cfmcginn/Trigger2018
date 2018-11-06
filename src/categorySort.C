//cpp dependencies
#include <fstream>
#include <iostream>
#include <map>
#include <string>

//ROOT dependencies
#include "TDatime.h"

//Local dependencies
#include "include/checkMakeDir.h"

int categorySort(const std::string inFileName, std::string categoryList)
{
  if(!checkFile(inFileName)){
    std::cout << "Given inFileName \'" << inFileName << "\' is not valid. return 1" << std::endl;
    return 1;
  }

  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;

  std::string outFileName = inFileName;
  outFileName.replace(outFileName.find(".txt"), 4, ("_Sorted_" + dateStr + ".txt").c_str());

  std::map<std::string, std::vector<std::string>> categoryMap;
  categoryList = categoryList + ",";
  while(categoryList.find(",,") != std::string::npos){categoryList.replace(categoryList.find(",,"), 2, ",");}
  while(categoryList.find(",") != std::string::npos){
    categoryMap[categoryList.substr(0, categoryList.find(","))] = {};
    categoryList.replace(0, categoryList.find(",")+1, "");
  }

  std::string tempStr;
  std::ifstream inFile(inFileName.c_str());
  while(std::getline(inFile, tempStr)){
    if(tempStr.size() == 0) continue;
    if(tempStr.substr(0,1).find("#") != std::string::npos) continue;
    
    
    std::string tempStr2 = tempStr;
    while(tempStr.find(" ") != std::string::npos){tempStr.replace(tempStr.find(" "), 1, ",");}
    while(tempStr.find("\t") != std::string::npos){tempStr.replace(tempStr.find("\t"), 1, ",");}
    while(tempStr.find(",,") != std::string::npos){tempStr.replace(tempStr.find(",,"), 2, ",");}

    bool isFound = false;
    for(auto const& cat : categoryMap){
      if(tempStr.find(cat.first + ",") != std::string::npos){
	isFound = true;
	categoryMap[cat.first].push_back(tempStr2);
	//	cat.second.push_back(tempStr);
	break;
      }
    }

    if(!isFound) std::cout << "Warning \'" << tempStr << "\' is not found." << std::endl;
  }
  inFile.close();

  std::ofstream outFile(outFileName.c_str());

  for(auto const& cat : categoryMap){
    for(unsigned int i = 0; i < cat.second.size(); ++i){
      std::string outStr = cat.second[i];
      while(outStr.find("HIHardProbesPrescaled") != std::string::npos){outStr.replace(outStr.find("HIHardProbesPrescaled"), std::string("HIHardProbesPrescaled").size(), "HIHardProbesLower");}
      outFile << outStr << std::endl;
    }
  }

  outFile.close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 3){
    std::cout << "Usage: ./bin/categorySort.exe <inFileName> <commaSeparatedCategories>. return 1" << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += categorySort(argv[1], argv[2]);
  return retVal;
}
