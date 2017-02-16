#include <iostream>
#include <cmath>
#include <cstring>
#include <vector>

#include "DRS4.h"

int main() {
  DRS4* DataSet1 = new DRS4("./VUVTPB5mV_cosmic.dat");
  DataSet1->Decode(100000);

  return 0;
}
