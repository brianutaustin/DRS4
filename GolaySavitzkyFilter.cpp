#include "GolaySavitzkyFilter.h"

GolaySavitzkyFilter::GolaySavitzkyFilter() {
  NPoints = 7;
  MDegree = 4;

  this->CalculateJMatrix();
  this->CalculateCMatrix();
}

GolaySavitzkyFilter::GolaySavitzkyFilter(unsigned int N, unsigned int M) {
  this->SetMDegree(M);
  this->SetNPoints(N);

  this->CalculateJMatrix();
  this->CalculateCMatrix();
}

GolaySavitzkyFilter::~GolaySavitzkyFilter() {
}

void GolaySavitzkyFilter::SetNPoints(unsigned int N) {
  if (N % 2 == 1) {
    this->NPoints = N;
  } else {
    std::cout << "\t WARNING: Number of interpolation points has to be an odd integer." << std::endl;
    std::cout << "\t WARNING: Filter algorithm automatically sets number of points to " << N + 1 << "." << std::endl;
    this->NPoints = N + 1;
  }

  return;
}

void GolaySavitzkyFilter::SetMDegree(unsigned int M) {
  MDegree = M;
  return;
}

void GolaySavitzkyFilter::CalculateJMatrix() {
  double* JArray = (double*) malloc(NPoints * MDegree * sizeof(double));
  for (int i = 0; i < NPoints; i++) {
    for (int j = 0; j < MDegree; j++) {
      if ((- ((NPoints - 1) / 2) + i == 0) && (j == 0)) {
        *(JArray + MDegree * i + j) = 1;
      } else {
        *(JArray + MDegree * i + j) = TMath::Power(- (((double) NPoints - 1) / 2) + (double) i, (double) j);
      }
    }
  }

  JMatrix = new TMatrixD(NPoints, MDegree, JArray);
#ifdef DebugMode
  std::cout << "\t The J matrix: " << std::endl;
  JMatrix->Print();
#endif

  JTransposeMatrix = new TMatrixD(MDegree, NPoints);
  JTransposeMatrix->Transpose(*JMatrix);
#ifdef DebugMode
  std::cout << "\t The J-transposed matrix: " << std::endl;
  JTransposeMatrix->Print();
#endif

  JJTransposeMatrix = new TMatrixD(MDegree, MDegree);
  JJTransposeMatrix->Mult(*JTransposeMatrix, *JMatrix);
#ifdef DebugMode
  std::cout << "\t The product of J and J-transposed matrices: " << std::endl;
  JJTransposeMatrix->Print();
#endif

  return;
}

void GolaySavitzkyFilter::CalculateCMatrix() {
  double Determinant;
  JJTransposeMatrix->InvertFast(&Determinant);

  CMatrix = new TMatrixD(MDegree, NPoints);
  CMatrix->Mult(*JJTransposeMatrix, *JTransposeMatrix);
#ifdef DebugMode
  std::cout << "The C matrix: " << std::endl;
  CMatrix->Print();
#endif

  return;
}

void GolaySavitzkyFilter::SetData(double* XArray, double* YArray, unsigned int NSample) {
  this->DataXaxis = (double*) malloc(NSample * sizeof(double));
  std::copy(XArray, XArray + NSample, this->DataXaxis);

  this->SamplingInterval = (*(YArray + NSample) - *(YArray)) / (NSample - 1);

  return;
}

void GolaySavitzkyFilter::SmoothingWaveform(double* XArray, double* YArray, unsigned int NSample) {
  this->SetData(XArray, YArray, NSample);

  double* ConvolutionCoeffs = (double*) malloc(NPoints * sizeof(double));


  return;
}
