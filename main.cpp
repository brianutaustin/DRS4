#include <iostream>
#include <cmath>
#include <cstring>
#include <vector>

#include "DRS4.h"

int main() {
  DRS4* DataSet1 = new DRS4("./Cryo100mV1kall.bin");
  DataSet1->Decode(-1);
  DataSet1->SaveWaveformToROOTFile();
  DataSet1->SaveDataToROOTFile(kPulseArea);
  DataSet1->SaveDataToROOTFile(kPulseAmplitude);
  DataSet1->SaveDataToROOTFile(kRiseTime);
  DataSet1->SaveDataToROOTFile(kBaseline);

  return 0;
}
