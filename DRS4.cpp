/*
   Original version created by:      Stefan Ritt <stefan.ritt@psi.ch>
   Current version developed by:     UTKL HEP group <The University of Texas at Austin>
 */

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

void DRS4::SetNumberOfEvents(unsigned int n) {
  NumberOfEvents = n;
  return;
}


void DRS4::SaveRawWaveformToROOTFile() {
  ROOTFileName = DataFileName;
  ROOTFileName.erase(ROOTFileName.length() - 3, ROOTFileName.length());
  ROOTFileName = ROOTFileName + "root";

  TFile * ROOTDataFile = new TFile(ROOTFileName.c_str(), "UPDATE");

  // Define the raw waveform tree
  RAWWAVEFORM dummyWaveformStruct;
  TTree * DataTree = new TTree("DataTree", "DataTree");
  DataTree->Branch("TimeChannel1", dummyWaveformStruct.RawTime[0], "TimeChannel1[1024]/D");
  DataTree->Branch("TimeChannel2", dummyWaveformStruct.RawTime[1], "TimeChannel2[1024]/D");
  DataTree->Branch("TimeChannel3", dummyWaveformStruct.RawTime[2], "TimeChannel3[1024]/D");
  DataTree->Branch("TimeChannel4", dummyWaveformStruct.RawTime[3], "TimeChannel3[1024]/D");
  DataTree->Branch("WaveformChannel1", dummyWaveformStruct.RawWaveform[0], "WaveformChannel1[1024]/D");
  DataTree->Branch("WaveformChannel2", dummyWaveformStruct.RawWaveform[1], "WaveformChannel2[1024]/D");
  DataTree->Branch("WaveformChannel3", dummyWaveformStruct.RawWaveform[2], "WaveformChannel3[1024]/D");
  DataTree->Branch("WaveformChannel4", dummyWaveformStruct.RawWaveform[3], "WaveformChannel4[1024]/D");

  for (unsigned int EventNumber = 0; EventNumber < NumberOfEvents; EventNumber++) {
    dummyWaveformStruct = RawWaveformDataVector.at(EventNumber);
    DataTree->Fill();
  }

  DataTree->Write(0, TObject::kOverwrite);
  ROOTDataFile->Close();

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

  RawWaveformDataVector.clear();
  RAWWAVEFORM dummyWaveformStruct;
  for (unsigned int EventNumber = 0; EventNumber < NumberOfEvents; EventNumber++) {
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

    for (unsigned int ChannelNumber = 0; ChannelNumber < 4; ChannelNumber++) {
      std::copy(std::begin(Waveform[ChannelNumber]), std::end(Waveform[ChannelNumber]), std::begin(dummyWaveformStruct.RawWaveform[ChannelNumber]));
      std::copy(std::begin(Time[ChannelNumber]), std::end(Time[ChannelNumber]), std::begin(dummyWaveformStruct.RawTime[ChannelNumber]));
    }
    RawWaveformDataVector.push_back(dummyWaveformStruct);
  }

  return;
}

void DRS4::SaveHighLevelDataToROOTFile(HighLevelVariableIndex index) {
  TFile * ROOTDataFile = new TFile(ROOTFileName.c_str(), "UPDATE");
  TTree * DataTree = (TTree *) ROOTDataFile->Get("DataTree");

  // Fake Data
  TRandom r;

  if (index == kPulseArea) {
    double PulseArea[4];
    TBranch * PulseAreaChannel1 = DataTree->Branch("PulseAreaChannel1", &PulseArea[0], "PulseAreaChannel1/D");
    TBranch * PulseAreaChannel2 = DataTree->Branch("PulseAreaChannel2", &PulseArea[1], "PulseAreaChannel2/D");
    TBranch * PulseAreaChannel3 = DataTree->Branch("PulseAreaChannel3", &PulseArea[2], "PulseAreaChannel3/D");
    TBranch * PulseAreaChannel4 = DataTree->Branch("PulseAreaChannel4", &PulseArea[3], "PulseAreaChannel4/D");

    for (Long64_t i = 0; i < NumberOfEvents; i++) {
      DataTree->GetEntry(i);
      // Do something here to calculate PulseArea
      // Fake Data
      for (int j = 0; j < 4; j++) {
        PulseArea[j] = r.Gaus(0, 1);
      }
      PulseAreaChannel1->Fill();
      PulseAreaChannel2->Fill();
      PulseAreaChannel3->Fill();
      PulseAreaChannel4->Fill();
    }
    DataTree->Write(0, TObject::kOverwrite);

  } else if (index == kPulseAmplitude) {
    double PulseAmp[4];
    TBranch * PulseAmplitudeChannel1 = DataTree->Branch("PulseAmplitudeChannel1", &PulseAmp[0], "PulseAmplitudeChannel1/D");
    TBranch * PulseAmplitudeChannel2 = DataTree->Branch("PulseAmplitudeChannel2", &PulseAmp[1], "PulseAmplitudeChannel2/D");
    TBranch * PulseAmplitudeChannel3 = DataTree->Branch("PulseAmplitudeChannel3", &PulseAmp[2], "PulseAmplitudeChannel3/D");
    TBranch * PulseAmplitudeChannel4 = DataTree->Branch("PulseAmplitudeChannel4", &PulseAmp[3], "PulseAmplitudeChannel4/D");

    for (Long64_t i = 0; i < NumberOfEvents; i++) {
      DataTree->GetEntry(i);
      // Do something here to calculate PulseAmp
      // Fake Data
      for (int j = 0; j < 4; j++) {
        PulseAmp[j] = r.Gaus(0, 1);
      }
      PulseAmplitudeChannel1->Fill();
      PulseAmplitudeChannel2->Fill();
      PulseAmplitudeChannel3->Fill();
      PulseAmplitudeChannel4->Fill();
    }
    DataTree->Write(0, TObject::kOverwrite);

  } else if (index == kRiseTime) {
    double RiseTime[4];
    TBranch * RiseTimeChannel1 = DataTree->Branch("RiseTimeChannel1", &RiseTime[0], "RiseTimeChannel1/D");
    TBranch * RiseTimeChannel2 = DataTree->Branch("RiseTimeChannel2", &RiseTime[1], "RiseTimeChannel2/D");
    TBranch * RiseTimeChannel3 = DataTree->Branch("RiseTimeChannel3", &RiseTime[2], "RiseTimeChannel3/D");
    TBranch * RiseTimeChannel4 = DataTree->Branch("RiseTimeChannel4", &RiseTime[4], "RiseTimeChannel4/D");

    for (Long64_t i = 0; i < NumberOfEvents; i++) {
      DataTree->GetEntry(i);
      // Do something here to calculate RiseTime
      // Fake Data
      for (int j = 0; j < 4; j++) {
        RiseTime[j] = r.Gaus(0, 1);
      }
      RiseTimeChannel1->Fill();
      RiseTimeChannel2->Fill();
      RiseTimeChannel3->Fill();
      RiseTimeChannel4->Fill();
    }
    DataTree->Write(0, TObject::kOverwrite);

  } else {
    ROOTDataFile->Close();
    return;
  }

  ROOTDataFile->Close();
  return;
}
