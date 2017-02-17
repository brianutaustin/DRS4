//////////////////////////////////////////////////////////
// Thu Feb 16 17:32:31 2017 by Dung Phan
//
// Dung Phan
// The University of Texas at Austin
//////////////////////////////////////////////////////////

#ifndef GolaySavitzkyFilter_h
#define GolaySavitzkyFilter_h

#include <cmath>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <iostream>

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TMatrixD.h>
#include <TMath.h>

class GolaySavitzkyFilter {
public:
  GolaySavitzkyFilter();
  GolaySavitzkyFilter(unsigned int, unsigned int);
  virtual  ~GolaySavitzkyFilter();

private:
  virtual void CalculateJMatrix();
  virtual void CalculateCMatrix();
  virtual void SetNPoints(unsigned int);
  virtual void SetMDegree(unsigned int);
  virtual void SetData(double*, double*, unsigned int);

  unsigned int NPoints;
  unsigned int MDegree;
  TMatrixD* JMatrix;
  TMatrixD* JTransposeMatrix;
  TMatrixD* JJTransposeMatrix;
  TMatrixD* CMatrix;

  double  SamplingInterval;
  double* DataXaxis;
  double* DataXaxisSmoothedWaveform;
  double* DataXaxisSmoothedFirstDerivative;
};

#endif
