//cpp dependencies
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

//ROOT dependencies
#include "TMath.h"

//local dependencies
#include "include/checkMakeDir.h"
#include "include/trigger2018to2015.h"

double collRateAtT(double inT)
{
  double par0 = 30784.145;
  double par1 = 31701.962;
  double par2 = -24451.485;
  double par3 = 253984.65;
  double par4 = 9421.2384;

  return par0*TMath::Exp(-inT/par1)*TMath::Exp(par2/inT)+par3*TMath::Exp(-inT/par4);
}

int countSummary(std::string inThreshList, const std::string inDir, const std::string fillCSV)
{
  if(!checkDir(inDir)){
    std::cout << "Given inDir \'" << inDir << "\' is invalid. return 1" << std::endl;
    return 1;
  }

  std::vector<int> threshList;
  inThreshList = inThreshList + ",";
  while(inThreshList.find(",,") != std::string::npos){inThreshList.replace(inThreshList.find(",,"), 2, ",");}
  while(inThreshList.find(",") != std::string::npos){
    threshList.push_back(std::stoi(inThreshList.substr(0, inThreshList.find(","))));
    inThreshList.replace(0, inThreshList.find(",")+1, "");
  }

  const double fullMBRate = 500;
  const double reducedMBRate = 6000;
  
  const double assumedUpTime = 6000000000./6500.;
  //  const double fillStartTWorst = 21850; //30 khz
  const double fillStartTWorst = 25076;//23 khz
  const int fillTWorst = 10*60*60;

  const double fillStartTMid = 18563;
  const int fillTMid = 8*60*60;

  const double fillStartTLikely = 16100;
  const int fillTLikely = 8*60*60;
  //  const double fillEndT = fillStartTWorst + fillTWorst;  

  std::sort(std::begin(threshList), std::end(threshList));

  std::map<std::string, std::vector<double> > pdNameToRates;
  std::map<std::string, std::vector<double> > pdNameToRawSizes;
  std::map<std::string, std::vector<double> > pdNameToTotSizes;
  std::map<std::string, std::vector<double> > pdOverNameToRates;
  std::map<std::string, std::vector<double> > pdOverNameToTotSizes;


  std::map<std::string, std::vector<double> > hltNameToRates;
  std::map<std::string, double> hltNameToFiringFrac;
  std::map<std::string, double> hltNameToMinRate;
  std::map<std::string, double> hltNameToMaxRate;
  std::map<std::string, double> hltNameTo23KHz;
  std::map<std::string, double> hltNameTo46KHz;
  std::map<std::string, double> hltNameToWorstCase;
  std::map<std::string, double> hltNameToMidCase;
  std::map<std::string, double> hltNameToLikelyCase;

  std::map<std::string, double> pdNameToWorstCase;
  std::map<std::string, double> pdNameToMidCase;
  std::map<std::string, double> pdNameToLikelyCase;

  std::map<std::string, double> pdOverNameToWorstCase;
  std::map<std::string, double> pdOverNameToMidCase;
  std::map<std::string, double> pdOverNameToLikelyCase;

  std::map<std::string, double> pdSizeToWorstCase;
  std::map<std::string, double> pdSizeToMidCase;
  std::map<std::string, double> pdSizeToLikelyCase;

  std::map<std::string, double> pdOverSizeToWorstCase;
  std::map<std::string, double> pdOverSizeToMidCase;
  std::map<std::string, double> pdOverSizeToLikelyCase;

  std::vector<int> totalPerThresh;

  double nEvtTotal = -1;

  for(unsigned int i = 0; i < threshList.size(); ++i){
    std::string fileName = inDir + "/results_hlt" + std::to_string(threshList.at(i)) + ".txt";

    std::string tempStr;
    std::ifstream inFile(fileName.c_str());
    std::cout << "file: " << fileName << std::endl;
    while(std::getline(inFile, tempStr)){
      if(tempStr.find("Processing") != std::string::npos){
	tempStr.replace(0, std::string("Processing").size(), "");
	while(tempStr.find(".") != std::string::npos) tempStr.replace(tempStr.find("."), 1, "");
	while(tempStr.find(" ") != std::string::npos) tempStr.replace(tempStr.find(" "), 1, "");
	
	if(nEvtTotal < 0) nEvtTotal = std::stod(tempStr);
	else if(nEvtTotal != std::stod(tempStr)) std::cout << "WARNING: NEVTTOTOTAL DOES NOT MATCH" << std::endl;

	continue;
      }
      

      if(tempStr.find("SUPER SUMMARY: H") != std::string::npos){	
	tempStr.replace(0, tempStr.find(": ")+2, "");
	tempStr = tempStr +",";
	while(tempStr.find(",,") != std::string::npos){tempStr.replace(tempStr.find(",,"), 2, ",");}

	std::vector<std::string> lineVect;
	while(tempStr.find(",") != std::string::npos){
	  lineVect.push_back(tempStr.substr(0, tempStr.find(",")));
	  tempStr.replace(0, tempStr.find(",")+1, "");
	}
	
 	if(i == 0){
	  pdNameToRates[lineVect.at(0)] = {};
	  pdNameToRawSizes[lineVect.at(0)] = {};
	  pdNameToTotSizes[lineVect.at(0)] = {};

	  pdNameToWorstCase[lineVect.at(0)] = 0;
	  pdNameToMidCase[lineVect.at(0)] = 0;
	  pdNameToLikelyCase[lineVect.at(0)] = 0;
	  
	  pdSizeToWorstCase[lineVect.at(0)] = 0;
	  pdSizeToMidCase[lineVect.at(0)] = 0;
	  pdSizeToLikelyCase[lineVect.at(0)] = 0;
	}
      
	pdNameToRates[lineVect.at(0)].push_back(std::stod(lineVect.at(2)));
	pdNameToRawSizes[lineVect.at(0)].push_back(std::stod(lineVect.at(4)));
	pdNameToTotSizes[lineVect.at(0)].push_back(std::stod(lineVect.at(3)));
      
	continue;
      }

      if(tempStr.find("OVERLAP SUMMARY 2: H") != std::string::npos){	
	tempStr.replace(0, tempStr.find(": ")+2, "");
	tempStr = tempStr +",";
	while(tempStr.find(",,") != std::string::npos){tempStr.replace(tempStr.find(",,"), 2, ",");}


	std::vector<std::string> lineVect;
	while(tempStr.find(",") != std::string::npos){
	  lineVect.push_back(tempStr.substr(0, tempStr.find(",")));
	  tempStr.replace(0, tempStr.find(",")+1, "");
	}
	
 	if(i == 0){
	  pdOverNameToRates[lineVect.at(0)] = {};
	  pdOverNameToTotSizes[lineVect.at(0)] = {};

	  pdOverNameToWorstCase[lineVect.at(0)] = 0;
	  pdOverNameToMidCase[lineVect.at(0)] = 0;
	  pdOverNameToLikelyCase[lineVect.at(0)] = 0;
	  
	  pdOverSizeToWorstCase[lineVect.at(0)] = 0;
	  pdOverSizeToMidCase[lineVect.at(0)] = 0;
	  pdOverSizeToLikelyCase[lineVect.at(0)] = 0;
	}

	pdOverNameToRates[lineVect.at(0)].push_back(std::stod(lineVect.at(2)));
	pdOverNameToTotSizes[lineVect.at(0)].push_back(std::stod(lineVect.at(3)));

	continue;
      }
      
      if(tempStr.find("/") == std::string::npos) continue;
      if(tempStr.find(":") == std::string::npos) continue;
      if(tempStr.size() < 2) continue;
      if(tempStr.substr(0,2).find("  ") != std::string::npos) continue;


      tempStr.replace(0, tempStr.find(":")+1, "");
      std::string trigName = tempStr.substr(0, tempStr.find(","));
      while(trigName.find(" ") != std::string::npos){trigName.replace(trigName.find(" "), 1, "");}

      tempStr.replace(0, tempStr.find(",")+1, "");
      tempStr.replace(0, tempStr.find(",")+1, "");
      tempStr.replace(0, tempStr.find(",")+1, "");
      tempStr.replace(0, tempStr.find(",")+1, "");
      std::string fires = tempStr.substr(0, tempStr.find(","));
      while(fires.find(" ") != std::string::npos){fires.replace(fires.find(" "), 1, "");}

      std::string rate = tempStr.substr(tempStr.rfind(",")+1, tempStr.size());
      while(rate.find(" ") != std::string::npos){rate.replace(rate.find(" "), 1, "");}
      
      if(trigName.find("Jet100") != std::string::npos && trigName.find("Cent") != std::string::npos) continue;
      if(trigName.find("Jet120") != std::string::npos) continue;
      if(trigName.find("Photon40") != std::string::npos && trigName.find("Cent") != std::string::npos) continue;
      if(trigName.find("Photon50") != std::string::npos) continue;
      if(trigName.find("Photon60") != std::string::npos) continue;

      if(trigName.find("HIEle") != std::string::npos) continue;
      if(trigName.find("HIDoubleEle") != std::string::npos) continue;
      if(trigName.find("HIMinimumB") != std::string::npos) continue;

      if(i == 0){
	hltNameToFiringFrac[trigName] = std::stod(fires)/nEvtTotal;
	hltNameToRates[trigName] = {};
	hltNameToMinRate[trigName] = std::stod(rate);
	hltNameToMaxRate[trigName] = std::stod(rate);
	hltNameTo23KHz[trigName] = 0;
	hltNameTo46KHz[trigName] = 0;
	hltNameToWorstCase[trigName] = 0;
	hltNameToMidCase[trigName] = 0;
	hltNameToLikelyCase[trigName] = 0;
      }

     
      if(hltNameToMinRate[trigName] > std::stod(rate)) hltNameToMinRate[trigName] = std::stod(rate);
      if(hltNameToMaxRate[trigName] < std::stod(rate)) hltNameToMaxRate[trigName] = std::stod(rate);

      hltNameToRates[trigName].push_back(std::stod(rate));
    }
    inFile.close();
  }

  std::ifstream csvFile(fillCSV.c_str());
  std::string tempStr;
  std::vector<double> lumiVals;
  while(std::getline(csvFile, tempStr)){
    for(unsigned int tI = 0; tI < 10; ++tI){
      tempStr.replace(0, tempStr.find(",")+1, "");
    }
    tempStr = tempStr.substr(0, tempStr.find(","));
    lumiVals.push_back(std::stod(tempStr));
  }
  csvFile.close();

  /*
  std::cout << "Minimum Counts: " << std::endl;
  for(auto const& min : hltNameToMinRate){
    std::cout << " " << min.first << "," << min.second*assumedUpTime << std::endl;
  }

  std::cout << "Maximum Counts: " << std::endl;
  for(auto const& max : hltNameToMaxRate){
    std::cout << " " << max.first << "," << max.second*assumedUpTime << std::endl;
  }
  */

  for(unsigned int i = 0; i < lumiVals.size(); ++i){
    int ratePos23 = -1;
    int ratePos46 = -1;
    for(unsigned int tI = 0; tI < threshList.size()-1; ++tI){
      if(lumiVals.at(i) >= 1000*threshList.at(tI) && lumiVals.at(i) < 1000*threshList.at(tI+1)){
	ratePos23 = tI+1;
      }

      if(lumiVals.at(i)*2 >= 1000*threshList.at(tI) && lumiVals.at(i)*2 < 1000*threshList.at(tI+1)){
	ratePos46 = tI+1;
      }
    }
    if(ratePos23 < 0) ratePos23 = 0;
    if(ratePos46 < 0) ratePos46 = 0;


    for(auto const& hlt : hltNameToRates){
      double currentRate23Khz = hlt.second[ratePos23]*lumiVals.at(i)/((double)1000.*threshList.at(ratePos23));
      double currentRate46Khz = hlt.second[ratePos46]*2*lumiVals.at(i)/((double)1000.*threshList.at(ratePos46));
      double nEvt23 = currentRate23Khz*assumedUpTime*(1./(double)lumiVals.size());
      double nEvt46 = currentRate23Khz*.25*assumedUpTime*(1./(double)lumiVals.size()) + currentRate46Khz*.75*assumedUpTime*(1./(double)lumiVals.size());
      hltNameTo23KHz[hlt.first] += nEvt23;
      hltNameTo46KHz[hlt.first] += nEvt46;
    }
  }

  for(int tI = 0; tI < fillTWorst; ++tI){
    double currentLumi = collRateAtT(fillStartTWorst+tI);

    int ratePos = -1;
    for(unsigned int tI = 0; tI < threshList.size()-1; ++tI){
      if(currentLumi >= 1000*threshList.at(tI) && currentLumi < 1000*threshList.at(tI+1)){
	ratePos = tI+1;
      }
    }
    if(ratePos < 0) ratePos = 0;

    for(auto const& hlt : hltNameToRates){
      double currentRate = hlt.second[ratePos]*currentLumi/((double)1000.*threshList.at(ratePos));
      double nEvt = currentRate*assumedUpTime*(1./(double)fillTWorst);
      hltNameToWorstCase[hlt.first] += nEvt;
      hltNameToMidCase[hlt.first] += nEvt*1./2.;
      hltNameToLikelyCase[hlt.first] += nEvt*1./4.;
    }


    for(auto const& pd : pdNameToRates){
      double currentRate = pd.second[ratePos]*currentLumi/((double)1000.*threshList.at(ratePos));
      double nEvt = currentRate*assumedUpTime*(1./(double)fillTWorst);
      double currentTotSize = nEvt*pdNameToTotSizes[pd.first].at(ratePos);
      pdNameToWorstCase[pd.first] += nEvt;
      pdNameToMidCase[pd.first] += nEvt*1./2.;
      pdNameToLikelyCase[pd.first] += nEvt*1./4.;

      pdSizeToWorstCase[pd.first] += currentTotSize;
      pdSizeToMidCase[pd.first] += currentTotSize*1./2.;
      pdSizeToLikelyCase[pd.first] += currentTotSize*1./4.;
    }

    for(auto const& pd : pdOverNameToRates){
      double currentRate = pd.second[ratePos]*currentLumi/((double)1000.*threshList.at(ratePos));
      double nEvt = currentRate*assumedUpTime*(1./(double)fillTWorst);
      double currentTotSize = nEvt*pdOverNameToTotSizes[pd.first].at(ratePos);
      pdOverNameToWorstCase[pd.first] += nEvt;
      pdOverNameToMidCase[pd.first] += nEvt*1./2.;
      pdOverNameToLikelyCase[pd.first] += nEvt*1./4.;

      pdOverSizeToWorstCase[pd.first] += currentTotSize;
      pdOverSizeToMidCase[pd.first] += currentTotSize*1./2.;
      pdOverSizeToLikelyCase[pd.first] += currentTotSize*1./4.;
    }
    
  }


  for(int tI = 0; tI < fillTMid; ++tI){
    double currentLumi = collRateAtT(fillStartTMid+tI);

    int ratePos = -1;
    for(unsigned int tI = 0; tI < threshList.size()-1; ++tI){
      if(currentLumi >= 1000*threshList.at(tI) && currentLumi < 1000*threshList.at(tI+1)){
	ratePos = tI+1;
      }
    }
    if(ratePos < 0) ratePos = 0;


    for(auto const& hlt : hltNameToRates){
      double currentRate = hlt.second[ratePos]*currentLumi/((double)1000.*threshList.at(ratePos));
      double nEvt = currentRate*assumedUpTime*(1./(double)fillTMid);
      hltNameToMidCase[hlt.first] += nEvt*2./4.;
    }

    for(auto const& pd : pdNameToRates){
      double currentRate = pd.second[ratePos]*currentLumi/((double)1000.*threshList.at(ratePos));
      double nEvt = currentRate*assumedUpTime*(1./(double)fillTMid);
      double currentTotSize = nEvt*pdNameToTotSizes[pd.first].at(ratePos);
      pdNameToMidCase[pd.first] += nEvt*2./4.;
      pdSizeToMidCase[pd.first] += currentTotSize*2./4.;
    }

    for(auto const& pd : pdOverNameToRates){
      double currentRate = pd.second[ratePos]*currentLumi/((double)1000.*threshList.at(ratePos));
      double nEvt = currentRate*assumedUpTime*(1./(double)fillTMid);
      double currentTotSize = nEvt*pdOverNameToTotSizes[pd.first].at(ratePos);
      pdOverNameToMidCase[pd.first] += nEvt*2./4.;
      pdOverSizeToMidCase[pd.first] += currentTotSize*2./4.;
    }
  }

  for(int tI = 0; tI < fillTLikely; ++tI){
    double currentLumi = collRateAtT(fillStartTLikely+tI);

    int ratePos = -1;
    for(unsigned int tI = 0; tI < threshList.size()-1; ++tI){
      if(currentLumi >= 1000*threshList.at(tI) && currentLumi < 1000*threshList.at(tI+1)){
	ratePos = tI+1;
      }
    }
    if(ratePos < 0) ratePos = 0;


    for(auto const& hlt : hltNameToRates){
      double currentRate = hlt.second[ratePos]*currentLumi/((double)1000.*threshList.at(ratePos));
      double nEvt = currentRate*assumedUpTime*(1./(double)fillTLikely);
      hltNameToLikelyCase[hlt.first] += nEvt*3./4.;
    }

    for(auto const& pd : pdNameToRates){
      double currentRate = pd.second[ratePos]*currentLumi/((double)1000.*threshList.at(ratePos));
      double nEvt = currentRate*assumedUpTime*(1./(double)fillTLikely);
      double currentTotSize = nEvt*pdNameToTotSizes[pd.first].at(ratePos);
      pdNameToLikelyCase[pd.first] += nEvt*3./4.;
      pdSizeToLikelyCase[pd.first] += currentTotSize*3./4.;
    }

    for(auto const& pd : pdOverNameToRates){
      double currentRate = pd.second[ratePos]*currentLumi/((double)1000.*threshList.at(ratePos));
      double nEvt = currentRate*assumedUpTime*(1./(double)fillTLikely);
      double currentTotSize = nEvt*pdOverNameToTotSizes[pd.first].at(ratePos);
      pdOverNameToLikelyCase[pd.first] += nEvt*3./4.;
      pdOverSizeToLikelyCase[pd.first] += currentTotSize*3./4.;
    }
  }

  /*
  std::cout << "23khz Counts: " << std::endl;
  std::cout << "TrigName,Counts from Trig,Counts from MB" << std::endl;
  for(auto const& hlt : hltNameTo23KHz){
    std::cout << " " << hlt.first << "," << hlt.second << "," << hltNameToFiringFrac[hlt.first]*500*assumedUpTime << std::endl;
  }
  */
  /*
  std::cout << std::endl;
  std::cout << "46khz Counts: " << std::endl;
  std::cout << "TrigName,Counts from Trig,Counts from MB" << std::endl;
  for(auto const& hlt : hltNameTo46KHz){
    std::cout << " " << hlt.first << "," << hlt.second << "," << hltNameToFiringFrac[hlt.first]*500*assumedUpTime << std::endl;
  }
  */
  
  std::cout << std::endl;
  std::cout << "23khz worst case Counts: " << std::endl;
  std::cout << "TrigName,Counts from Trig,Counts from MB,Counts From MB reduced,,Trigger 2015,Counts 2015,Ratio 2018/2015," << std::endl;
  //  std::cout << "TrigName,Counts from Trig,Counts from MB,Counts From MB reduced,,Trigger 2015,Counts 2015,Ratio 2018/2015,Ratio MB/Trigger,Ratio MB Reduced/Trigger," << std::endl;
  for(auto const& hlt : hltNameToWorstCase){
    std::string outStr = " " + hlt.first + "," + std::to_string(((int)hlt.second)) + "," + std::to_string(((int)(hltNameToFiringFrac[hlt.first]*500*assumedUpTime))) + "," + std::to_string(((int)(hltNameToFiringFrac[hlt.first]*6000*assumedUpTime)));

    int counts2015 = getCountsFromTrigger2018To2015(hlt.first);
    if(counts2015 != 0){
      double ratio = TMath::Max((double)hlt.second, (double)hltNameToFiringFrac[hlt.first]*500*assumedUpTime)/((double)counts2015);
      outStr = outStr + ",," + triggerName2018to2015(hlt.first) + "," + std::to_string(counts2015) + "," + std::to_string(ratio) + ",";
    }
    else{
      outStr = outStr + ",,,,,";
    }

    /*
    double ratioMB = (hltNameToFiringFrac[hlt.first]*500*assumedUpTime)/hlt.second;
    double ratioMBRed = (hltNameToFiringFrac[hlt.first]*6000*assumedUpTime)/hlt.second;
    outStr = outStr + std::to_string(ratioMB) + "," + std::to_string(ratioMBRed) + ",";
    */

    std::cout << outStr << std::endl;
  }
  std::cout << std::endl;
  std::cout << "23khz worst case pd counts: " << std::endl;
  std::cout << "PD Name, Counts, AOD+RAW Size (TB)" << std::endl;
 
  double totalEvt = 0 ;
  double totalSize = 0 ;
  double overEvt = 0 ;
  double overSize = 0 ;
  for(auto const& pd : pdNameToWorstCase){
    std::cout << pd.first << ", " << pd.second << ", " << pdSizeToWorstCase[pd.first]/(1024*1024) << std::endl;
    
    totalEvt += pd.second;
    totalSize += pdSizeToWorstCase[pd.first]/(1024*1024);
  }

  std::cout << "PD Overlap Name, Counts, AOD+RAW Size (TB)" << std::endl;
  for(auto const& pd : pdOverNameToWorstCase){
    std::cout << pd.first << ", " << pd.second << ", " << pdOverSizeToWorstCase[pd.first]/(1024*1024) << std::endl;

    overEvt += pd.second;
    overSize += pdOverSizeToWorstCase[pd.first]/(1024*1024);
  }
  std::cout << "Total Events, Over Events, Ratio" << std::endl;
  std::cout << totalEvt << ", " << overEvt << ", " << overEvt/totalEvt << std::endl;
  std::cout << "Total Size, OverSize, Ratio" << std::endl;
  std::cout << totalSize << ", " << overSize << ", " << overSize/totalSize << std::endl;

  std::cout << std::endl;
  std::cout << "23khz 2/4 40khz 2/4 mid case Counts: " << std::endl;
  std::cout << "TrigName,Counts from Trig,Counts from MB,CountsFrom MB reduced,,Trigger 2015,Counts 2015,Ratio 2018/2015," << std::endl;
  //  std::cout << "TrigName,Counts from Trig,Counts from MB,CountsFrom MB reduced,,Trigger 2015,Counts 2015,Ratio 2018/2015,Ratio MB/Trigger,Ratio MB Reduced/Trigger," << std::endl;
  for(auto const& hlt : hltNameToMidCase){
    std::string outStr = " " + hlt.first + "," + std::to_string(((int)hlt.second)) + "," + std::to_string(((int)(hltNameToFiringFrac[hlt.first]*500*assumedUpTime))) + "," + std::to_string(((int)(hltNameToFiringFrac[hlt.first]*6000*assumedUpTime)));

    int counts2015 = getCountsFromTrigger2018To2015(hlt.first);
    if(counts2015 != 0){
      double ratio = TMath::Max((double)hlt.second, (double)hltNameToFiringFrac[hlt.first]*500*assumedUpTime)/((double)counts2015);
      outStr = outStr + ",," + triggerName2018to2015(hlt.first) + "," + std::to_string(counts2015) + "," + std::to_string(ratio) + ",";
    }
    else{
      outStr = outStr + ",,,,,";
    }
    /*
    double ratioMB = (hltNameToFiringFrac[hlt.first]*500*assumedUpTime)/hlt.second;
    double ratioMBRed = (hltNameToFiringFrac[hlt.first]*6000*assumedUpTime)/hlt.second;
    outStr = outStr + std::to_string(ratioMB) + "," + std::to_string(ratioMBRed) + ",";
    */
    std::cout << outStr << std::endl;
  }

  std::cout << std::endl;
  std::cout << "23khz 1/4 50khz 3/4 likely case Counts: " << std::endl;
  std::cout << "PD Name, Counts, AOD+RAW Size (TB)" << std::endl;
  totalEvt = 0;
  totalSize = 0;
  overEvt = 0;
  overSize = 0;
  for(auto const& pd : pdNameToMidCase){
    std::cout << pd.first << ", " << pd.second << ", " << pdSizeToMidCase[pd.first]/(1024*1024) << std::endl;

    totalEvt += pd.second;
    totalSize += pdSizeToMidCase[pd.first]/(1024*1024);
  }

  std::cout << "PD Overlap Name, Counts, AOD+RAW Size (TB)" << std::endl;
  for(auto const& pd : pdOverNameToMidCase){
    std::cout << pd.first << ", " << pd.second << ", " << pdOverSizeToMidCase[pd.first]/(1024*1024) << std::endl;

    overEvt += pd.second;
    overSize += pdOverSizeToMidCase[pd.first]/(1024*1024);
  }

  std::cout << "Total Events, Over Events, Ratio" << std::endl;
  std::cout << totalEvt << ", " << overEvt << ", " << overEvt/totalEvt << std::endl;
  std::cout << "Total Size, OverSize, Ratio" << std::endl;
  std::cout << totalSize << ", " << overSize << ", " << overSize/totalSize << std::endl;

  std::cout << std::endl;
  std::cout << "23khz 1/4 50khz 3/4 likely case Counts: " << std::endl;
  std::cout << "TrigName,Counts from Trig,Counts from MB,Counts from MB reduced,,Trigger 2015,Counts 2015,Ratio 2018/2015," << std::endl;
  //  std::cout << "TrigName,Counts from Trig,Counts from MB,Counts from MB reduced,,Trigger 2015,Counts 2015,Ratio 2018/2015,Ratio MB/Trigger,Ratio MB Reduced/Trigger," << std::endl;
  for(auto const& hlt : hltNameToLikelyCase){
    std::string outStr = " " + hlt.first + "," + std::to_string(((int)hlt.second)) + "," + std::to_string(((int)(hltNameToFiringFrac[hlt.first]*500*assumedUpTime))) + "," + std::to_string(((int)(hltNameToFiringFrac[hlt.first]*6000*assumedUpTime)));

    int counts2015 = getCountsFromTrigger2018To2015(hlt.first);
    if(counts2015 != 0){
      double ratio = TMath::Max((double)hlt.second, (double)hltNameToFiringFrac[hlt.first]*500*assumedUpTime)/((double)counts2015);
      outStr = outStr + ",," + triggerName2018to2015(hlt.first) + "," + std::to_string(counts2015) + "," + std::to_string(ratio) + ",";
    }
    else{
      outStr = outStr + ",,,,,";
    }
    /*
    double ratioMB = (hltNameToFiringFrac[hlt.first]*500*assumedUpTime)/hlt.second;
    double ratioMBRed = (hltNameToFiringFrac[hlt.first]*6000*assumedUpTime)/hlt.second;
    outStr = outStr + std::to_string(ratioMB) + "," + std::to_string(ratioMBRed) + ",";
    */
    std::cout << outStr << std::endl;
  }

  std::cout << std::endl;
  std::cout << "23khz 1/4 50khz 3/4 likely case Counts: " << std::endl;
  std::cout << "PD Name, Counts, AOD+RAW Size (TB)" << std::endl;
  totalEvt = 0;
  totalSize = 0;
  overEvt = 0;
  overSize = 0;
  for(auto const& pd : pdNameToLikelyCase){
    std::cout << pd.first << ", " << pd.second << ", " << pdSizeToLikelyCase[pd.first]/(1024*1024) << std::endl;

    totalEvt += pd.second;
    totalSize += pdSizeToLikelyCase[pd.first]/(1024*1024);
  }
  std::cout << "PD Overlap Name, Counts, AOD+RAW Size (TB)" << std::endl;
  for(auto const& pd : pdOverNameToLikelyCase){
    std::cout << pd.first << ", " << pd.second << ", " << pdOverSizeToLikelyCase[pd.first]/(1024*1024) << std::endl;

    overEvt += pd.second;
    overSize += pdOverSizeToLikelyCase[pd.first]/(1024*1024);
  }


  std::cout << "Total Events, Over Events, Ratio" << std::endl;
  std::cout << totalEvt << ", " << overEvt << ", " << overEvt/totalEvt << std::endl;
  std::cout << "Total Size, OverSize, Ratio" << std::endl;
  std::cout << totalSize << ", " << overSize << ", " << overSize/totalSize << std::endl;

  std::cout << std::endl;

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 4){
    std::cout << "./bin/countSummary.exe <inThreshList> <inDir> <csvFile>" << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += countSummary(argv[1], argv[2], argv[3]);
  return retVal;
}
