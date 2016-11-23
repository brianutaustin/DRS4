/*
  Original version created by:      Stefan Ritt <stefan.ritt@psi.ch>
  Current version developed by:     UTKL HEP group <The University of Texas at Austin>
*/

#ifndef DRS4_H
#define DRS4_H

#include <cstring>
#include <iostream>

#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "Getline.h"

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

class DRS4 {
public:
  DRS4();
  DRS4(std::string);
  virtual ~DRS4();
  void OpenBinaryDataFile();
  void SaveRawWaveformToROOTFile();
  void AccessTimeInfo();
  void AccessEventInfo();
  void SetNumberOfEvents(int);

private:
  std::string DataFileName;
  std::string ROOTFileName;

  int NumberOfEvents;

  FILE * BinaryDataFile;

  TFile * ROOTDataFile;
  TTree * RawWaveformDataTree;

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
