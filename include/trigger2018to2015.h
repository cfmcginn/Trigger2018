#ifndef TRIGGER2018TO2015_H
#define TRIGGER2018TO2015_H

#include "include/stringUtil.h"

const Int_t nTriggers = 54;
const std::string triggerNames2018[nTriggers] = {"HLT_HICsAK4PFJet100Eta1p5_v1",
						 "HLT_HICsAK4PFJet60Eta1p5_Centrality_30_100_v1",
						 "HLT_HICsAK4PFJet60Eta1p5_Centrality_50_100_v1",
						 "HLT_HICsAK4PFJet60Eta1p5_v1",
						 "HLT_HICsAK4PFJet80Eta1p5_Centrality_30_100_v1",
						 "HLT_HICsAK4PFJet80Eta1p5_Centrality_50_100_v1",
						 "HLT_HICsAK4PFJet80Eta1p5_v1",
						 "HLT_HIGEDPhoton10_Cent30_100_v1",
						 "HLT_HIGEDPhoton10_Cent50_100_v1",
						 "HLT_HIGEDPhoton10_EB_v1",
						 "HLT_HIGEDPhoton10_v1",
						 "HLT_HIGEDPhoton20_Cent30_100_v1",
						 "HLT_HIGEDPhoton20_Cent50_100_v1",
						 "HLT_HIGEDPhoton20_EB_v1",
						 "HLT_HIGEDPhoton20_v1",
						 "HLT_HIGEDPhoton30_Cent30_100_v1",
						 "HLT_HIGEDPhoton30_Cent50_100_v1",
						 "HLT_HIGEDPhoton30_EB_v1",
						 "HLT_HIGEDPhoton30_v1",
						 "HLT_HIGEDPhoton40_EB_v1",
						 "HLT_HIGEDPhoton40_v1",
						 "HLT_HIIslandPhoton10_Eta1p5_v1",
						 "HLT_HIIslandPhoton10_Eta3p1_Cent30_100_v1",
						 "HLT_HIIslandPhoton10_Eta3p1_Cent50_100_v1",
						 "HLT_HIIslandPhoton10_Eta3p1_v1",
						 "HLT_HIIslandPhoton20_Eta1p5_v1",
						 "HLT_HIIslandPhoton20_Eta3p1_Cent30_100_v1",
						 "HLT_HIIslandPhoton20_Eta3p1_Cent50_100_v1",
						 "HLT_HIIslandPhoton20_Eta3p1_v1",
						 "HLT_HIIslandPhoton30_Eta1p5_v1",
						 "HLT_HIIslandPhoton30_Eta3p1_Cent30_100_v1",
						 "HLT_HIIslandPhoton30_Eta3p1_Cent50_100_v1",
						 "HLT_HIIslandPhoton30_Eta3p1_v1",
						 "HLT_HIIslandPhoton40_Eta1p5_v1",
						 "HLT_HIIslandPhoton40_Eta3p1_v1",
						 "HLT_HIPuAK4CaloJet100Eta5p1_v1",
						 "HLT_HIPuAK4CaloJet100_35_Eta0p7_v1",
						 "HLT_HIPuAK4CaloJet100_35_Eta1p1_v1",
						 "HLT_HIPuAK4CaloJet40Eta5p1_Centrality_30_100_v1",
						 "HLT_HIPuAK4CaloJet40Eta5p1_Centrality_50_100_v1",
						 "HLT_HIPuAK4CaloJet40Eta5p1_v1",
						 "HLT_HIPuAK4CaloJet60Eta2p4_CSVv2WP0p75_v1",
						 "HLT_HIPuAK4CaloJet60Eta2p4_DeepCSV0p4_v1",
						 "HLT_HIPuAK4CaloJet60Eta5p1_Centrality_30_100_v1",
						 "HLT_HIPuAK4CaloJet60Eta5p1_Centrality_50_100_v1",
						 "HLT_HIPuAK4CaloJet60Eta5p1_v1",
						 "HLT_HIPuAK4CaloJet80Eta2p4_CSVv2WP0p75_v1",
						 "HLT_HIPuAK4CaloJet80Eta2p4_DeepCSV0p4_v1",
						 "HLT_HIPuAK4CaloJet80Eta5p1_Centrality_30_100_v1",
						 "HLT_HIPuAK4CaloJet80Eta5p1_Centrality_50_100_v1",
						 "HLT_HIPuAK4CaloJet80Eta5p1_v1",
						 "HLT_HIPuAK4CaloJet80_35_Eta0p7_v1",
						 "HLT_HIPuAK4CaloJet80_35_Eta1p1_v1",
						 "HLT_HIPuAK4CaloJet80_45_45_Eta2p1_v1"};

const std::string triggerNames2015[nTriggers] = {"HLT_HIPuAK4CaloJet100_Eta5p1_v1",
						 "HLT_HIPuAK4CaloJet60_Eta5p1_Cent30_100_v1",
						 "HLT_HIPuAK4CaloJet60_Eta5p1_Cent50_100_v1",
						 "HLT_HIPuAK4CaloJet60_Eta5p1_v1",
						 "HLT_HIPuAK4CaloJet80_Eta5p1_Cent30_100_v1",
						 "HLT_HIPuAK4CaloJet80_Eta5p1_Cent50_100_v1",
						 "HLT_HIPuAK4CaloJet80_Eta5p1_v1",
						 "HLT_HISinglePhoton10_Eta3p1_Cent30_100_v1",
						 "HLT_HISinglePhoton10_Eta3p1_Cent50_100_v1",
						 "HLT_HISinglePhoton10_Eta1p5_v1",
						 "HLT_HISinglePhoton10_Eta3p1_v1",
						 "HLT_HISinglePhoton20_Eta3p1_Cent30_100_v1",
						 "HLT_HISinglePhoton20_Eta3p1_Cent50_100_v1",
						 "HLT_HISinglePhoton20_Eta1p5_v1",
						 "HLT_HISinglePhoton20_Eta3p1_v1",
						 "HLT_HISinglePhoton30_Eta3p1_Cent30_100_v1",
						 "HLT_HISinglePhoton30_Eta3p1_Cent50_100_v1",
						 "HLT_HISinglePhoton30_Eta1p5_v1",
						 "HLT_HISinglePhoton30_Eta3p1_v1",
						 "HLT_HISinglePhoton40_Eta1p5_v1",
						 "HLT_HISinglePhoton40_Eta3p1_v1",
						 "HLT_HISinglePhoton10_Eta1p5_v1",
						 "HLT_HISinglePhoton10_Eta3p1_Cent30_100_v1",
						 "HLT_HISinglePhoton10_Eta3p1_Cent50_100_v1",
						 "HLT_HISinglePhoton10_Eta3p1_v1",
						 "HLT_HISinglePhoton20_Eta1p5_v1",
						 "HLT_HISinglePhoton20_Eta3p1_Cent30_100_v1",
						 "HLT_HISinglePhoton20_Eta3p1_Cent50_100_v1",
						 "HLT_HISinglePhoton20_Eta3p1_v1",
						 "HLT_HISinglePhoton30_Eta1p5_v1",
						 "HLT_HISinglePhoton30_Eta3p1_Cent30_100_v1",
						 "HLT_HISinglePhoton30_Eta3p1_Cent50_100_v1",
						 "HLT_HISinglePhoton30_Eta3p1_v1",
						 "HLT_HISinglePhoton40_Eta1p5_v1",
						 "HLT_HISinglePhoton40_Eta3p1_v1",
						 "HLT_HIPuAK4CaloJet100_Eta5p1_v1",
						 "HLT_HIPuAK4CaloJet100_Jet35_Eta0p7_v1",
						 "HLT_HIPuAK4CaloJet100_Jet35_Eta1p1_v1",
						 "HLT_HIPuAK4CaloJet40_Eta5p1_Cent30_100_v1",
						 "HLT_HIPuAK4CaloJet40_Eta5p1_Cent50_100_v1",
						 "HLT_HIPuAK4CaloJet40_Eta5p1_v1",
						 "HLT_HIPuAK4CaloBJetCSV60_Eta2p1_v1",
						 "HLT_HIPuAK4CaloBJetCSV60_Eta2p1_v1",
						 "HLT_HIPuAK4CaloJet60_Eta5p1_Cent30_100_v1",
						 "HLT_HIPuAK4CaloJet60_Eta5p1_Cent50_100_v1",
						 "HLT_HIPuAK4CaloJet60_Eta5p1_v1",
						 "HLT_HIPuAK4CaloBJetCSV80_Eta2p1_v1",
						 "HLT_HIPuAK4CaloBJetCSV80_Eta2p1_v1",
						 "HLT_HIPuAK4CaloJet80_Eta5p1_Cent30_100_v1",
						 "HLT_HIPuAK4CaloJet80_Eta5p1_Cent50_100_v1",
						 "HLT_HIPuAK4CaloJet80_Eta5p1_v1",
						 "HLT_HIPuAK4CaloJet80_Jet35_Eta0p7_v1",
						 "HLT_HIPuAK4CaloJet80_Jet35_Eta1p1_v1",
						 "HLT_HIPuAK4CaloJet80_45_45_Eta2p1_v1"};

const int triggerCounts2015[nTriggers] = {2057111, 2040474, 1383320, 3030261, 1413907, 399292, 2322000, 2622848, 1511790, 1252406, 1423996, 1644565, 569603, 1530204, 1365172, 1142752, 362931, 1336771, 2689282, 1360320, 1977241, 1252406, 2622848, 1511790, 1423996, 1530204, 1644565, 569603, 1365172, 1336771, 1142752, 362931, 2689282, 1360320, 1977241, 2057111, 616697, 1050698, 3158308, 2427002, 2531753, 1678269, 1678269, 2040474, 1383320, 3030261, 445273, 445273, 1413907, 399292, 2322000, 1769513, 2387885, 1251285};

std::string triggerName2018to2015(const std::string inStr)
{
  std::string outStr = "";

  for(Int_t tI = 0; tI < nTriggers; ++tI){
    if(isStrSame(triggerNames2018[tI], inStr)){
      outStr = triggerNames2015[tI];
      break;
    }
  }

  return outStr;
}

int getCountsFromTrigger2015(const std::string inStr)
{
  int retVal = 0;
  for(Int_t tI = 0; tI < nTriggers; ++tI){
    if(isStrSame(triggerNames2015[tI], inStr)){
      retVal = triggerCounts2015[tI];
      break;
    }
  }

  return retVal;
}

int getCountsFromTrigger2018To2015(const std::string inStr)
{
  const std::string match2015 = triggerName2018to2015(inStr);
  if(match2015.size() == 0) return 0;

  int retVal = 0;
  for(Int_t tI = 0; tI < nTriggers; ++tI){
    if(isStrSame(triggerNames2015[tI], match2015)){
      retVal = triggerCounts2015[tI];
      break;
    }
  }

  return retVal;
}

#endif
