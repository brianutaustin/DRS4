#include "DRS4.h"

DRS4::DRS4() {
}

DRS4::DRS4(std::string datafilename) {
  DataFileName = datafilename;
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
  NumberOfEvents = n;
  return;
}

/*
void DRS4::SaveRawWaveformToROOTFile() {
  ROOTFileName = DataFileName;
  ROOTFileName.erase(ROOTFileName.length() - 3, ROOTFileName.length());
  ROOTFileName = ROOTFileName + ".root";

  ROOTDataFile = new TFile(ROOTFileName.c_str(), "RECREATE");

  // Define the raw waveform tree
  RawWaveformDataTree = new TTree("RawWaveformDataTree", "RawWaveformDataTree");
  RawWaveformDataTree->Branch("TimeChannel1", Time[0], "TimeChannel1[1024]/D");
  RawWaveformDataTree->Branch("TimeChannel2", Time[1], "TimeChannel2[1024]/D");
  RawWaveformDataTree->Branch("TimeChannel3", Time[2], "TimeChannel3[1024]/D");
  RawWaveformDataTree->Branch("TimeChannel4", Time[3], "TimeChannel3[1024]/D");
  RawWaveformDataTree->Branch("WaveformChannel1", Waveform[0], "WaveformChannel1[1024]/D");
  RawWaveformDataTree->Branch("WaveformChannel2", Waveform[1], "WaveformChannel2[1024]/D");
  RawWaveformDataTree->Branch("WaveformChannel3", Waveform[2], "WaveformChannel3[1024]/D");
  RawWaveformDataTree->Branch("WaveformChannel4", Waveform[3], "WaveformChannel4[1024]/D");

  //RawWaveformDataTree->Fill();
  RawWaveformDataTree->Write();
  ROOTDataFile->Close();

  return;
}
*/

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

  for (int EventNumber = 0; EventNumber < NumberOfEvents; EventNumber++) {
    // Read event header
    int i = fread(&EventHeader, sizeof(EventHeader), 1, BinaryDataFile);
    if (i < 1) {
      // Check if this is the end of file
      break;
    }

    std::cout << "Found event " << EventHeader.EventSerialNumberValue << "." << std::endl;

    // Reach channel data
    for (int ChannelNumber = 0; ChannelNumber < 5; ChannelNumber++) {
      int k = fread(Header, sizeof(Header), 1, BinaryDataFile);
      if (k < 1) {
        break;
      }
      if (Header[0] != 'C') {
        // Event header found
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
  }

  return;
}
