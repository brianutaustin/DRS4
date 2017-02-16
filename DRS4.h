/*
  Original version created by:      Stefan Ritt <stefan.ritt@psi.ch>
  Current version developed by:     UTKL HEP group <The University of Texas at Austin>
*/

#ifndef DRS4_H
#define DRS4_H

#include <cstring>
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>

#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "Getline.h"
#include "TRandom.h"

struct THEADER {
  char           TimeHeaderLabel[4];
  char           BoardSerialNumberLabel[2];
  unsigned short BoardSerialNumberValue;
};

struct EHEADER {
  char           EventHeaderLabel[4];
  unsigned int   EventSerialNumberValue;
  unsigned short YearValue;
  unsigned short MonthValue;
  unsigned short DayValue;
  unsigned short HourValue;
  unsigned short MinuteValue;
  unsigned short SecondValue;
  unsigned short MillisecondValue;
  unsigned short ReservedCharValue;
  char           BoardSerialNumberLabel[2];
  unsigned short BoardSerialNumberValue;
  char           TriggerCellLabel[2];
  unsigned short TriggerCellValue;
};

struct RAWWAVEFORM {
  double RawWaveform[4][1024];
  double RawTime[4][1024];
};

enum VariableIndex {
  kPulseArea,
  kPulseAmplitude,
  kRiseTime,
  kBaseline,
};

class DRS4 {
public:
  DRS4();
  DRS4(std::string);
  virtual ~DRS4();
  void SetNumberOfEvents(int);
  void OpenBinaryDataFile();
  void AccessTimeInfo();
  void AccessEventInfo();
  void Decode(int);

private:
  std::string DataFileName;
  std::string ROOTFileName; // Raw Waveform Container File Name

  bool UseAllEventsFlag;

  unsigned int NumberOfEvents;

  FILE * BinaryDataFile;
  TTree * DataTree;

  THEADER TimeHeader;
  EHEADER EventHeader;

  /*
    Temporary variables to access the file header, the time header and the event headers
  */
  char Header[4];
  unsigned short Voltage[1024];
  double Waveform[4][1024];
  double Time[4][1024];
  float TimeBinWidth[4][1024];

};

#endif
