#pragma once
#include "complexNumbers.h"
#include "matrix.h"
#include <cmath>
#include <numbers>

template<typename T>
class FourierTransform {
public:
  FourierTransform(std::vector<double> time, matrix<T> inputData) {
    if (inputData.rows != 1) { std::cerr << "ERROR: Rows of inputData must be 1." << std::endl; }
    fft(time, inputData);
    //DFT(time, inputData);
    magnitude();
    //magnitudes.print("mag");
    phase();
  };
  
  void fft(std::vector<double> time, matrix<T> inputData) {
    
    int N = std::pow(2, (ceil(log2(inputData.cols))));
    transformData.rows = 1;
    transformData.cols = N/2;
    std::vector<complexNumber<double>> transformVec(N, complexNumber<double>(0.0, 0.0));
    for (int col = 0; col < transformData.cols; col++) {
      if (col < inputData.cols) {
        transformVec[col] = complexNumber<double>(inputData.data[0][col], 0.0);
      }
    }

    transformVec = ditfft2(transformVec, N);
    transformVec.resize(N/2);
    transformData.data = {transformVec};

    double fs = 1/time[time.size() - 1];
    auto f = std::vector<double>(transformData.cols, 0.0);
    for (int k = 0; k < N/2; k++) {
      f[k] = k*fs;
    }
    frequency = matrix<double>{{f}, N/2, 1};
  }
  
  std::vector<complexNumber<double>> ditfft2(std::vector<complexNumber<double>>& x, int N) {
    if (N == 1) {
        return {x[0]};
    } 

    auto even = std::vector<complexNumber<T>>(N / 2);
    auto odd = std::vector<complexNumber<T>>(N / 2);

    for (int i = 0; i < N / 2; i++) {
        even[i] = x[i * 2];
        odd[i] = x[i * 2 + 1];
    }
    even = ditfft2(even, N / 2);
    odd = ditfft2(odd, N / 2);

    std::vector<complexNumber<T>> X(N);
    for (int k = 0; k < N / 2; k++) {
      auto w = makeComplexNumberFromPolar<T>(1.0, -2*M_PI*k/N);
      X[k] = even[k] + w*odd[k];
      X[k+N/2] = even[k] - w*odd[k];
        
    }

    return X;
  }

  void DFT(std::vector<double> time, matrix<T> inputData) {
    // Choosing N to be the lenght of the data is fine
    // If we were to encounter the Nyquist sample rate we would have problems earlier
    double fs = 1/time[time.size() - 1];
    int N = inputData.cols;
    
    frequency.rows = 1;
    frequency.data.push_back({});
    frequency.cols = N/2;
    transformData.rows = 1;
    transformData.cols = N/2;
    std::vector<std::vector<complexNumber<double>>> transformVec(transformData.rows, std::vector<complexNumber<double>>(transformData.cols, complexNumber<double>(0.0, 0.0)));
    transformData.data = transformVec;
    for (int k = 0; k < N/2; k++) {
      for (int n = 0; n < N; n++) {
        auto data = inputData.data[0][n];
        double angle = -2*M_PI*k*n/N;
        transformData.data[0][k] += (data*makeComplexNumberFromPolar<double>(1, angle));
      }
      frequency.data[0].push_back(k*fs);

    }
  };

  void magnitude() {
    magnitudes.rows = transformData.rows;
    magnitudes.cols = transformData.cols;
    std::vector<std::vector<double>> magnitudeVec(magnitudes.rows, std::vector<double>(magnitudes.cols, 0.0));
    magnitudes.data = magnitudeVec;
    for (int row = 0; row < magnitudes.rows; row++) {
      for (int col = 0; col < magnitudes.cols; col++) {
        magnitudes.data[row][col] = transformData.data[row][col].magnitude();
      }
    }
  };

  void phase() {
    phases.rows = transformData.rows;
    phases.cols = transformData.cols;
    std::vector<std::vector<double>> phaseVec(phases.rows, std::vector<double>(phases.cols, 0.0));
    phases.data = phaseVec;
    for (int row = 0; row < phases.rows; row++) {
      for (int col = 0; col < phases.cols; col++) {
        phases.data[row][col] = transformData.data[row][col].phase();
      }
    }
  };

  matrix<complexNumber<double>> transformData;
  matrix<double> frequency;
  matrix<double> magnitudes;
  matrix<double> phases;
  
  double sampleFrequency;
};



