#include <iostream>
#include <cmath>
#include <cstring>
#include <vector>

#include "DRS4.h"

int main() {
  DRS4* DataSet1 = new DRS4("./Cryo100mV1kall.bin");
  DataSet1->OpenBinaryDataFile();
  DataSet1->SetNumberOfEvents(-1);
  DataSet1->AccessTimeInfo();
  DataSet1->AccessEventInfo();
  DataSet1->SaveRawWaveformToROOTFile();
//  DataSet1->SaveDataToROOTFile(kPulseArea);
//  DataSet1->SaveDataToROOTFile(kPulseAmplitude);
//  DataSet1->SaveDataToROOTFile(kRiseTime);

  return 0;
}
