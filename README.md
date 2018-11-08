# Trigger2018
Code for L1 and HLT 2018

To build area, run make (recommend -j4 minimum)

For Cross-checking global google doc do:
./bin/globalCSVToPrescales.exe \<inputCSVOfGoogleDoc\> \<optionalInputOpenHLTForRateCheck\>

Note: Argument \<inputCSVOfGoogleDoc\> cannot be a csv directly from google doc (there are problems w/ terminating characters). Rather, save to local preferred spreadsheet format, then export to csv from there

Note: Argument \<optionalInputOpenHLTForRateCheck\> is an openHLT file for x-checking of rates in doc. If absent, doc will only be checked for internal consistency + prime prescaling

Recommended use:

./bin/globalCSVToPrescales.exe \<inputCSVOfGoogleDoc\> \<optionalInputOpenHLTForRateCheck\> \>& out.log

grep WARNING out.log  # for all warnings

grep WARNING out.log | grep presc # for prescale warnings

grep WARNING out.log | grep deviatt # for rate warnings
