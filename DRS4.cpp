/*
   Original version created by:      Stefan Ritt <stefan.ritt@psi.ch>
   Current version developed by:     UTKL HEP group <The University of Texas at Austin>
 */

#include "DRS4.h"

DRS4::DRS4() {
}

DRS4::DRS4(std::string datafilename) {
  DataFileName = datafilename;
  ROOTFileName = DataFileName;
  ROOTFileName.erase(ROOTFileName.length() - 3, ROOTFileName.length());
  ROOTFileName = ROOTFileName + "root";
}

DRS4::~DRS4() {
  fclose(BinaryDataFile);
}

void DRS4::OpenBinaryDataFile() {
  BinaryDataFile = fopen(DataFileName.c_str(), "r");
  if (BinaryDataFile == 0) {
    std::cout << DataFileName << " not found." << std::endl;
    return;
  }
  return;
}

void DRS4::SetNumberOfEvents(int n) {
  if (n == -1) {
    NumberOfEvents = 1;
    UseAllEventsFlag = true;
  } else {
    NumberOfEvents = n;
    UseAllEventsFlag = false;
  }
  return;
}

void DRS4::Decode(int n) {
  OpenBinaryDataFile();
  SetNumberOfEvents(n);
  AccessTimeInfo();
  AccessEventInfo();

  return;
}

void DRS4::AccessTimeInfo() {
  // Read time header
  fread(&TimeHeader, sizeof(TimeHeader), 1, BinaryDataFile);
  std::cout << "Found data for board number " << TimeHeader.BoardSerialNumberValue << "." << std::endl;

  // Read time bin widths
  memset(TimeBinWidth, sizeof(TimeBinWidth), 0);
  for (int ChannelNumber = 0; ChannelNumber < 5; ChannelNumber++) {
    fread(Header, sizeof(Header), 1, BinaryDataFile);
    if (Header[0] != 'C') {
      // Event header found
      fseek(BinaryDataFile, -4, SEEK_CUR);
      break;
    }
    int i = Header[3] - '0' - 1;
    std::cout << "Found timing calibration for channel " << i + 1 << "." << std::endl;
    fread(&TimeBinWidth[i][0], sizeof(float), 1024, BinaryDataFile);
  }

  return;
}

void DRS4::AccessEventInfo() {
  if (NumberOfEvents == 0) {
    std::cout << "Set the number of events you want to analyze." << std::endl;
    std::cout << "See DRS4::SetNumberOfEvents()." << std::endl;
    return;
  }

  if (UseAllEventsFlag) {
    std::cout << "You choose to use the whole dataset. All events will be analyzed." << std::endl;
  } else {
    std::cout << "You choose to use parts of dataset: " << NumberOfEvents <<  " events will be analyzed." << std::endl;
  }

  TFile * ROOTDataFile = new TFile(ROOTFileName.c_str(), "RECREATE");
  RAWWAVEFORM dummyWaveformStruct;
  DataTree = new TTree("DataTree", "DataTree");
  DataTree->Branch("TimeChannel1", dummyWaveformStruct.RawTime[0], "TimeChannel1[1024]/D");
  DataTree->Branch("TimeChannel2", dummyWaveformStruct.RawTime[1], "TimeChannel2[1024]/D");
  DataTree->Branch("TimeChannel3", dummyWaveformStruct.RawTime[2], "TimeChannel3[1024]/D");
  DataTree->Branch("TimeChannel4", dummyWaveformStruct.RawTime[3], "TimeChannel3[1024]/D");
  DataTree->Branch("WaveformChannel1", dummyWaveformStruct.RawWaveform[0], "WaveformChannel1[1024]/D");
  DataTree->Branch("WaveformChannel2", dummyWaveformStruct.RawWaveform[1], "WaveformChannel2[1024]/D");
  DataTree->Branch("WaveformChannel3", dummyWaveformStruct.RawWaveform[2], "WaveformChannel3[1024]/D");
  DataTree->Branch("WaveformChannel4", dummyWaveformStruct.RawWaveform[3], "WaveformChannel4[1024]/D");

  for (unsigned int EventNumber = 0; EventNumber < NumberOfEvents; EventNumber++) {
    // Read event header
    int i = fread(&EventHeader, sizeof(EventHeader), 1, BinaryDataFile);
    if (i < 1) {
      // Check if this is the end of file
      NumberOfEvents -= 1;
      break;
    } else {
      if (UseAllEventsFlag) {
        NumberOfEvents += 1;
      }
    }

    // Reach channel data
    for (int ChannelNumber = 0; ChannelNumber < 5; ChannelNumber++) {
      int k = fread(Header, sizeof(Header), 1, BinaryDataFile);
      if (k < 1) {
        break;
      }
      if (Header[0] != 'C') {
        // EveCryo100mV1kall.binnt header found
        fseek(BinaryDataFile, -4, SEEK_CUR);
        break;
      }
      int ChannelIndex = Header[3] - '0' - 1;
      fread(Voltage, sizeof(short), 1024, BinaryDataFile);

      for (int m = 0; m < 1024; m++) {
        // Convert data to volts
        Waveform[ChannelIndex][m] = (Voltage[m] / 65536. - 0.5);
        // Calculate time for this cell
        Time[ChannelIndex][m] = 0;
        for (int j = 0; j < m; j++) {
          Time[ChannelIndex][m] += TimeBinWidth[ChannelIndex][(j + EventHeader.TriggerCellValue) % 1024];
        }
      }
    }

    // Align cell #0 of all channels
    double T1 = Time[0][(1024 - EventHeader.TriggerCellValue) % 1024];
    for (int ChannelNumber = 1; ChannelNumber < 4; ChannelNumber++) {
      double T2 = Time[ChannelNumber][(1024 - EventHeader.TriggerCellValue) % 1024];
      double DT = T1 - T2;
      for (int l = 0; l < 1024; l++) {
        Time[ChannelNumber][l] += DT;
      }
    }

    for (unsigned int ChannelNumber = 0; ChannelNumber < 4; ChannelNumber++) {
      std::copy(std::begin(Waveform[ChannelNumber]), std::end(Waveform[ChannelNumber]), std::begin(dummyWaveformStruct.RawWaveform[ChannelNumber]));
      std::copy(std::begin(Time[ChannelNumber]), std::end(Time[ChannelNumber]), std::begin(dummyWaveformStruct.RawTime[ChannelNumber]));
    }

    //std::cout << "Found event " << EventHeader.EventSerialNumberValue << "." << std::endl;
    DataTree->Fill();
  }

  DataTree->Write(0, TObject::kOverwrite);
  ROOTDataFile->Close();

  return;
}
