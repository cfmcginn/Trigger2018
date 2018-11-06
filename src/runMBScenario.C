//cpp dependencies
#include <iostream>
#include <string>
#include <vector>

//ROOT dependencies
#include "TMath.h"

double collRateAtT(double inT)
{
  double par0 = 30784.145;
  double par1 = 31701.962;
  double par2 = -24451.485;
  double par3 = 253984.65;
  double par4 = 9421.2384;

  return par0*TMath::Exp(-inT/par1)*TMath::Exp(par2/inT)+par3*TMath::Exp(-inT/par4);
}

int runMBScenario(const std::string commaSeparatedMBFull, const std::string commaSeparatedMBRed)
{
  //  const double assumedUpTime = 6000000000./6500.; // this is the time if we ran 6500 flat
  const double assumedUpTime = (6000000000./6500.)*(6000000000./4543408551.); // time accounting for decay
  const double fillStartTWorst = 25076;//23 khz
  const int fillTWorst = 10*60*60;

  const Int_t nCollRates = 6;
  const Double_t collRates[nCollRates] = {8, 10, 13, 16, 20, 23};
  const Double_t physSizePerColl[nCollRates] = {0.4715524, 0.5749264, 0.63435266, 0.64974664, 0.9991709, 1.119982};
  Double_t maxSize[nCollRates] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  const Double_t mbFullRawSize = 1.0; //in MB, per event
  const Double_t mbRedRawSize = .65; //in MB, per event

  std::vector<int> mbFullRates;
  std::vector<int> mbRedRates;
  std::string commaSeparatedMBFullCopy = commaSeparatedMBFull;
  std::string commaSeparatedMBRedCopy = commaSeparatedMBRed;

  if(commaSeparatedMBFullCopy.substr(commaSeparatedMBFullCopy.size()-1, 1).find(",") == std::string::npos) commaSeparatedMBFullCopy = commaSeparatedMBFullCopy + ",";

  if(commaSeparatedMBRedCopy.substr(commaSeparatedMBRedCopy.size()-1, 1).find(",") == std::string::npos) commaSeparatedMBRedCopy = commaSeparatedMBRedCopy + ",";
  
  while(commaSeparatedMBFullCopy.find(",") != std::string::npos){
    mbFullRates.push_back(std::stod(commaSeparatedMBFullCopy.substr(0, commaSeparatedMBFullCopy.find(","))));
    commaSeparatedMBFullCopy.replace(0, commaSeparatedMBFullCopy.find(",")+1, "");
  }

  if(mbFullRates.size() != (unsigned int)nCollRates){
    std::cout << "ERROR: Given inputRates for MBFull, \'" << commaSeparatedMBFull << "\', does not match needed number, " << nCollRates << ", only " << mbFullRates.size() << ". return 1" << std::endl;
    return 1;
  }

  while(commaSeparatedMBRedCopy.find(",") != std::string::npos){
    mbRedRates.push_back(std::stod(commaSeparatedMBRedCopy.substr(0, commaSeparatedMBRedCopy.find(","))));
    commaSeparatedMBRedCopy.replace(0, commaSeparatedMBRedCopy.find(",")+1, "");
  }

  if(mbRedRates.size() != (unsigned int)nCollRates){
    std::cout << "ERROR: Given inputRates for MBRed, \'" << commaSeparatedMBRed << "\', does not match needed number, " << nCollRates << ", only " << mbRedRates.size() << ". return 1" << std::endl;
    return 1;
  }

  unsigned long long nEvtFullMB = 0;
  unsigned long long nEvtRedMB = 0;

  for(int i = 0; i < fillTWorst; ++i){
    double instantLumi = collRateAtT(i + fillStartTWorst);
    int lumiPos = -1;
    for(Int_t nI = 0; nI < nCollRates; ++nI){
      if(instantLumi < collRates[nI]*1000.){
	lumiPos = nI;
	break;
      }
    }
    if(lumiPos < 0 && instantLumi > 1000.*collRates[nCollRates-1]) lumiPos = nCollRates-1;

    unsigned long long thisEvtFullMB = (unsigned long long)((instantLumi/(1000.*collRates[lumiPos]))*mbFullRates.at(lumiPos)*assumedUpTime/((Double_t)(fillTWorst)));
    unsigned long long thisEvtRedMB = (unsigned long long)((instantLumi/(1000.*collRates[lumiPos]))*mbRedRates.at(lumiPos)*assumedUpTime/((Double_t)(fillTWorst)));

    double evtSizeFull = (((double)fillTWorst)/assumedUpTime)*mbFullRawSize*((double)thisEvtFullMB)/(1024);
    double evtSizeRed = (((double)fillTWorst)/assumedUpTime)*mbRedRawSize*((double)thisEvtRedMB)/(1024);

    double totalSize = evtSizeFull + evtSizeRed + physSizePerColl[lumiPos];
    if(totalSize > maxSize[lumiPos]) maxSize[lumiPos] = totalSize;

    //    std::cout << i << "/" << fillTWorst << ": " << instantLumi << ", " << collRates[lumiPos] << ", " << mbFullRates.at(lumiPos) << ", " << assumedUpTime << ", " << fillTWorst << ", " << thisEvtFullMB << ", " << thisEvtRedMB << ", " << nEvtFullMB << ", " << nEvtRedMB << std::endl;
       
    nEvtFullMB += thisEvtFullMB;
    nEvtRedMB += thisEvtRedMB;
  }

  std::cout << "Assumed uptime: " << assumedUpTime << " seconds" << std::endl;
  std::cout << "Peak collision rate (Hz): " << collRateAtT(fillStartTWorst) << std::endl;
  std::cout << "Fill length (seconds): " << fillTWorst << std::endl;
  std::cout << "Final collision rate (Hz): " << collRateAtT(fillStartTWorst + fillTWorst) << std::endl;
  std::cout << "TotalMB: " << nEvtFullMB + nEvtRedMB << std::endl;
  std::cout << " ReducedMB: " << nEvtRedMB << std::endl;
  std::cout << " FullMB: " << nEvtFullMB << std::endl;

  std::cout << "Max size / column: " << std::endl;
  for(Int_t cI = 0; cI < nCollRates; ++cI){
    std::cout << " " << collRates[cI] << " kHz: " << maxSize[cI] << " GB/s" << std::endl;
  }

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 3){
    std::cout << "Usage: ./bin/runMBScenario.exe <commaSeparatedMBFull> <commaSeparatedMBRed>" << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += runMBScenario(argv[1], argv[2]);
  return retVal;
}
