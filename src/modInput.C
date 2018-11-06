//cpp dependencies
#include <iostream>
#include <string>
#include <fstream>

//ROOT dependencies
#include "TDatime.h"

//Local dependencies
#include "include/checkMakeDir.h"

int modInput(const std::string inFileName)
{
  if(!checkFile(inFileName)){
    std::cout << "Given inFileName \'" << inFileName << "\' is invalid. return 1" << std::endl;
    return 1;
  }
  
  if(inFileName.find(".txt") != std::string::npos && inFileName.find(".csv") != std::string::npos && inFileName.find(".tsv") != std::string::npos){
    std::cout << "Given inFileName \'" << inFileName << "\' is invalid. return 1" << std::endl;
    return 1;    
  }

  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;

  std::string outFileName = inFileName;
  if(outFileName.find(".txt") != std::string::npos) outFileName.replace(outFileName.find(".txt"), 4, "");
  else if(outFileName.find(".csv") != std::string::npos) outFileName.replace(outFileName.find(".csv"), 4, "");
  else if(outFileName.find(".tsv") != std::string::npos) outFileName.replace(outFileName.find(".tsv"), 4, "");
  outFileName = outFileName + "_ModFormat_" + dateStr + ".csv";

  std::ofstream outFile(outFileName.c_str());

  std::ifstream inFile(inFileName.c_str());
  std::string tempStr;
  while(std::getline(inFile, tempStr)){
    while(tempStr.find(" ") != std::string::npos){tempStr.replace(tempStr.find(" "), 1, ",");}
    while(tempStr.find("\t") != std::string::npos){tempStr.replace(tempStr.find("\t"), 1, ",");}
    while(tempStr.find(",,") != std::string::npos){tempStr.replace(tempStr.find(",,"), 2, ",");}

    if(tempStr.size() == 0) continue;
    if(tempStr.substr(0,1).find("#") != std::string::npos) continue;

    outFile << tempStr.substr(0, tempStr.find(",")+1);
    tempStr.replace(0, tempStr.find(",")+1, "");
    tempStr.replace(0, tempStr.find(",")+1, "");
    std::string pdStr = tempStr.substr(0, tempStr.find(","));
    tempStr.replace(0, tempStr.find(",")+1, "");

    int prescale = std::stoi(tempStr.substr(0,tempStr.find(",")));
    tempStr.replace(0, tempStr.find(",")+1, "");
    prescale *= std::stoi(tempStr.substr(0,tempStr.find(",")));

    if(prescale == 0) prescale = -1;

    outFile << prescale << "," << pdStr << "," << pdStr << "SubPD,-1,";

    outFile << std::endl;
  }

  inFile.close();

  outFile.close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 2){
    std::cout << "Usage: ./bin/modInput.exe <inFileName>" << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += modInput(argv[1]);
  return retVal;
}
