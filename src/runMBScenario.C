//cpp dependencies
#include <iostream>
#include <string>
#include <vector>

//ROOT dependencies
#include "TMath.h"

//Local dependencies
#include "include/plotUtilities.h"

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

  const Double_t rateHIHardProbes[nCollRates] = {18, 20, 25, 31, 63, 72};
  const Double_t rateHIHardProbesLower[nCollRates] = {9, 14, 9, 9, 18, 31};
  const Double_t rateHIHardProbesPeripheral[nCollRates] = {12, 13, 13, 14, 30, 35};
  const Double_t rateHIHeavyFlavor[nCollRates] = {23, 28, 22, 17, 18, 19};
  const Double_t rateHIDoubleMuon[nCollRates] = {31, 48, 58, 67, 79, 89};
  const Double_t rateHISingleMuon[nCollRates] = {46, 54, 65, 68, 128, 131};
  const Double_t rateHIForward[nCollRates] = {799, 999, 1299, 1598, 1998, 2297};
  const Double_t rateHIHighMultiplicityETTAsymm[nCollRates] = {0, 0, 0, 0, 104, 100};

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

  double nEvtFullMB = 0;
  double nEvtRedMB = 0;
  
  double nHIHardProbes = 0;
  double nHIHardProbesLower = 0;
  double nHIHardProbesPeripheral = 0;
  double nHIHeavyFlavor = 0;
  double nHIDoubleMuon = 0;
  double nHISingleMuon = 0;
  double nHIForward = 0;
  double nHIHighMultiplicityETTAsymm = 0;

  double nHIHardProbesOverlapFull = 0;
  double nHIHardProbesLowerOverlapFull = 0;
  double nHIHardProbesPeripheralOverlapFull = 0;
  double nHIHeavyFlavorOverlapFull = 0;
  double nHIDoubleMuonOverlapFull = 0;
  double nHISingleMuonOverlapFull = 0;
  double nHIForwardOverlapFull = 0;
  double nHIHighMultiplicityETTAsymmOverlapFull = 0;

  double nHIHardProbesOverlapRed = 0;
  double nHIHardProbesLowerOverlapRed = 0;
  double nHIHardProbesPeripheralOverlapRed = 0;
  double nHIHeavyFlavorOverlapRed = 0;
  double nHIDoubleMuonOverlapRed = 0;
  double nHISingleMuonOverlapRed = 0;
  double nHIForwardOverlapRed = 0;
  double nHIHighMultiplicityETTAsymmOverlapRed = 0;

  double finalFullRate = 0;
  double finalRedRate = 0;

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

    double thisEvtFullMB = (double)((instantLumi/(1000.*collRates[lumiPos]))*mbFullRates.at(lumiPos)*assumedUpTime/((Double_t)(fillTWorst)));
    double thisEvtRedMB = (double)((instantLumi/(1000.*collRates[lumiPos]))*mbRedRates.at(lumiPos)*assumedUpTime/((Double_t)(fillTWorst)));

    double thisEvtHIHardProbes = (double)((instantLumi/(1000.*collRates[lumiPos]))*rateHIHardProbes[lumiPos]*assumedUpTime/((Double_t)(fillTWorst)));
    double thisEvtHIHardProbesLower = (double)((instantLumi/(1000.*collRates[lumiPos]))*rateHIHardProbesLower[lumiPos]*assumedUpTime/((Double_t)(fillTWorst)));
    double thisEvtHIHardProbesPeripheral = (double)((instantLumi/(1000.*collRates[lumiPos]))*rateHIHardProbesPeripheral[lumiPos]*assumedUpTime/((Double_t)(fillTWorst)));
    double thisEvtHIHeavyFlavor = (double)((instantLumi/(1000.*collRates[lumiPos]))*rateHIHeavyFlavor[lumiPos]*assumedUpTime/((Double_t)(fillTWorst)));
    double thisEvtHIDoubleMuon = (double)((instantLumi/(1000.*collRates[lumiPos]))*rateHIDoubleMuon[lumiPos]*assumedUpTime/((Double_t)(fillTWorst)));
    double thisEvtHISingleMuon = (double)((instantLumi/(1000.*collRates[lumiPos]))*rateHISingleMuon[lumiPos]*assumedUpTime/((Double_t)(fillTWorst)));
    double thisEvtHIForward = (double)((instantLumi/(1000.*collRates[lumiPos]))*rateHIForward[lumiPos]*assumedUpTime/((Double_t)(fillTWorst)));
    double thisEvtHIHighMultiplicityETTAsymm = (double)((instantLumi/(1000.*collRates[lumiPos]))*rateHIHighMultiplicityETTAsymm[lumiPos]*assumedUpTime/((Double_t)(fillTWorst)));

    double fullFraction = mbFullRates.at(lumiPos)/(collRates[lumiPos]*1000.);
    double redFraction = mbRedRates.at(lumiPos)/(collRates[lumiPos]*1000.);

    nHIHardProbes += thisEvtHIHardProbes;
    nHIHardProbesLower += thisEvtHIHardProbesLower;
    nHIHardProbesPeripheral += thisEvtHIHardProbesPeripheral;
    nHIHeavyFlavor += thisEvtHIHeavyFlavor;
    nHIDoubleMuon += thisEvtHIDoubleMuon;
    nHISingleMuon += thisEvtHISingleMuon;
    nHIForward += thisEvtHIForward;
    nHIHighMultiplicityETTAsymm += thisEvtHIHighMultiplicityETTAsymm;

    nHIHardProbesOverlapFull += thisEvtHIHardProbes*fullFraction;
    nHIHardProbesLowerOverlapFull += thisEvtHIHardProbesLower*fullFraction;
    nHIHardProbesPeripheralOverlapFull += thisEvtHIHardProbesPeripheral*fullFraction;
    nHIHeavyFlavorOverlapFull += thisEvtHIHeavyFlavor*fullFraction;
    nHIDoubleMuonOverlapFull += thisEvtHIDoubleMuon*fullFraction;
    nHISingleMuonOverlapFull += thisEvtHISingleMuon*fullFraction;
    nHIForwardOverlapFull += thisEvtHIForward*fullFraction;
    nHIHighMultiplicityETTAsymmOverlapFull += thisEvtHIHighMultiplicityETTAsymm*fullFraction;

    nHIHardProbesOverlapRed += thisEvtHIHardProbes*redFraction;
    nHIHardProbesLowerOverlapRed += thisEvtHIHardProbesLower*redFraction;
    nHIHardProbesPeripheralOverlapRed += thisEvtHIHardProbesPeripheral*redFraction;
    nHIHeavyFlavorOverlapRed += thisEvtHIHeavyFlavor*redFraction;
    nHIDoubleMuonOverlapRed += thisEvtHIDoubleMuon*redFraction;
    nHISingleMuonOverlapRed += thisEvtHISingleMuon*redFraction;
    nHIForwardOverlapRed += thisEvtHIForward*redFraction;
    nHIHighMultiplicityETTAsymmOverlapRed += thisEvtHIHighMultiplicityETTAsymm*redFraction;

    double evtSizeFull = (((double)fillTWorst)/assumedUpTime)*mbFullRawSize*((double)thisEvtFullMB)/(1024);
    double evtSizeRed = (((double)fillTWorst)/assumedUpTime)*mbRedRawSize*((double)thisEvtRedMB)/(1024);

    double totalSize = evtSizeFull + evtSizeRed + physSizePerColl[lumiPos];
    if(totalSize > maxSize[lumiPos]) maxSize[lumiPos] = totalSize;

       
    nEvtFullMB += thisEvtFullMB;
    nEvtRedMB += thisEvtRedMB;

    finalFullRate = thisEvtFullMB*(((double)fillTWorst)/assumedUpTime);
    finalRedRate = thisEvtRedMB*(((double)fillTWorst)/assumedUpTime);
  }

  std::cout << "Assumed uptime: " << assumedUpTime << " seconds" << std::endl;
  std::cout << "Peak collision rate (Hz): " << collRateAtT(fillStartTWorst) << std::endl;
  std::cout << "Fill length (seconds): " << fillTWorst << std::endl;
  std::cout << "Final collision rate (Hz): " << collRateAtT(fillStartTWorst + fillTWorst) << std::endl;
  std::cout << " Final full rate (Hz): " << finalFullRate << std::endl;
  std::cout << " Final reduced rate (Hz): " << finalRedRate << std::endl;
  std::cout << "Full RAW Event size (MB): " << mbFullRawSize << std::endl;
  std::cout << "Reduced RAW Event size (MB): " << mbRedRawSize << std::endl;

  std::vector<std::string> countStrings = {"TotalMB: " + std::to_string(nEvtFullMB + nEvtRedMB), " Reduced MB: " + std::to_string(nEvtRedMB), " Full MB: " + std::to_string(nEvtFullMB)};
  unsigned int finalSize = 40;
  for(unsigned int sI = 0; sI < countStrings.size(); ++sI){
    std::string count = countStrings.at(sI);
    while(count.size() < finalSize){count.replace(count.find(":"), 1, ": ");}
    std::cout << count << std::endl;
  }

  std::cout << "Full MB / Total: " << prettyString(((double)nEvtFullMB)/((double)(nEvtFullMB + nEvtRedMB)), 3, false) << std::endl;

  std::cout << "Max size / column: " << std::endl;
  for(Int_t cI = 0; cI < nCollRates; ++cI){
    std::cout << " " << collRates[cI] << " kHz: " << maxSize[cI] << " GB/s" << std::endl;
  }

  std::vector<std::string> pdNames = {"HIHardProbes", "HIHardProbesLower", "HIHardProbesPeripheral", "HIHeavyFlavor", "HIDoubleMuon", "HISingleMuon", "HIForward", "HIHighMultiplicityETTAsymm"};
  std::vector<double> pdTotal = {nHIHardProbes, nHIHardProbesLower, nHIHardProbesPeripheral, nHIHeavyFlavor, nHIDoubleMuon, nHISingleMuon, nHIForward, nHIHighMultiplicityETTAsymm};
  std::vector<double> pdOverlapFull = {nHIHardProbesOverlapFull, nHIHardProbesLowerOverlapFull, nHIHardProbesPeripheralOverlapFull, nHIHeavyFlavorOverlapFull, nHIDoubleMuonOverlapFull, nHISingleMuonOverlapFull, nHIForwardOverlapFull, nHIHighMultiplicityETTAsymmOverlapFull};
  std::vector<double> pdOverlapRed = {nHIHardProbesOverlapRed, nHIHardProbesLowerOverlapRed, nHIHardProbesPeripheralOverlapRed, nHIHeavyFlavorOverlapRed, nHIDoubleMuonOverlapRed, nHISingleMuonOverlapRed, nHIForwardOverlapRed, nHIHighMultiplicityETTAsymmOverlapRed};

  std::vector<std::string> topStr = {"PD", "Total Counts", "Overlap FullMB", "Fraction", "Overlap ReducedMB", "Fraction"};
  std::vector<long unsigned int> sizes = {topStr.at(0).size(), topStr.at(1).size(), topStr.at(2).size(), topStr.at(3).size(), topStr.at(4).size(), topStr.at(5).size()};

  for(unsigned int i = 0; i < pdNames.size(); ++i){
    if(sizes.at(0) < pdNames.at(i).size()) sizes.at(0) = pdNames.at(i).size();

    std::string tempStr = std::to_string(((unsigned long long)pdTotal.at(i)));
    if(sizes.at(1) < tempStr.size()) sizes.at(1) = tempStr.size();

    tempStr = std::to_string(((unsigned long long)pdOverlapFull.at(i)));
    if(sizes.at(2) < tempStr.size()) sizes.at(2) = tempStr.size();

    tempStr = prettyString(pdOverlapFull.at(i)/pdTotal.at(i), 3, false);
    if(sizes.at(3) < tempStr.size()) sizes.at(3) = tempStr.size();

    tempStr = std::to_string(((unsigned long long)pdOverlapRed.at(i)));
    if(sizes.at(4) < tempStr.size()) sizes.at(4) = tempStr.size();

    tempStr = prettyString(pdOverlapRed.at(i)/pdTotal.at(i), 3, false);
    if(sizes.at(5) < tempStr.size()) sizes.at(5) = tempStr.size();
  }

  std::string finalTopStr = "";
  for(unsigned int tI = 0; tI < topStr.size(); ++tI){
    std::string tempTop = topStr.at(tI);
    while(tempTop.size() < sizes.at(tI)+1){tempTop = tempTop + " ";}
    finalTopStr = finalTopStr + tempTop;
  }
  
  std::cout << finalTopStr << std::endl;
  for(unsigned int i = 0; i < pdNames.size(); ++i){
    finalTopStr = "";
    std::string tempTop = pdNames.at(i);
    while(tempTop.size() < sizes.at(0)+1){tempTop = tempTop + " ";}
    finalTopStr = finalTopStr + tempTop;

    tempTop = std::to_string((unsigned long long)pdTotal.at(i));
    while(tempTop.size() < sizes.at(1)+1){tempTop = tempTop + " ";}
    finalTopStr = finalTopStr + tempTop;

    tempTop = std::to_string((unsigned long long)pdOverlapFull.at(i));
    while(tempTop.size() < sizes.at(2)+1){tempTop = tempTop + " ";}
    finalTopStr = finalTopStr + tempTop;

    tempTop = prettyString(pdOverlapFull.at(i)/pdTotal.at(i), 3, false);
    while(tempTop.size() < sizes.at(3)+1){tempTop = tempTop + " ";}
    finalTopStr = finalTopStr + tempTop;

    tempTop = std::to_string((unsigned long long)pdOverlapRed.at(i));
    while(tempTop.size() < sizes.at(4)+1){tempTop = tempTop + " ";}
    finalTopStr = finalTopStr + tempTop;

    tempTop = prettyString(pdOverlapRed.at(i)/pdTotal.at(i), 3, false);
    while(tempTop.size() < sizes.at(5)+1){tempTop = tempTop + " ";}
    finalTopStr = finalTopStr + tempTop;

    
    std::cout << finalTopStr << std::endl;
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
