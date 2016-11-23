#include <iostream>
#include <cmath>
#include <cstring>
#include <vector>

#include "DRS4.h"

int main() {
  DRS4* DataSet1 = new DRS4("/home/dphan/Workspace/drs4Script/Cryo100mV1kall.bin");
  DataSet1->OpenBinaryDataFile();
  DataSet1->SetNumberOfEvents(1000);
  DataSet1->AccessTimeInfo();
  DataSet1->AccessEventInfo();

  return 0;
}
