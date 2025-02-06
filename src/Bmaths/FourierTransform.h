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
    //fft(inputData);
    DFT(time, inputData);
    magnitude();
    //magnitudes.print("mag");
    phase();
  };
  
  void fft(matrix<T> inputData) {
    int N = inputData.cols;
    transformData.rows = 1;
    transformData.cols = N;
    std::vector<std::vector<complexNumber<double>>> transformVec(transformData.rows, std::vector<complexNumber<double>>(transformData.cols, complexNumber<double>(0.0, 0.0)));
    transformData.data = transformVec;
    for (int col = 0; col < inputData.cols; col++) {
      transformData.data[0][col] = complexNumber<double>(inputData.data[0][col], 0.0);
    }

    ditfft2(transformData, N);
    //transformData.print("td");
    frequency.data.push_back({});
    for (int col = 0; col < transformData.cols; col++) {
      frequency.data[0].push_back((double)col);
    }
    frequency.rows = transformData.rows;
    frequency.cols = transformData.cols;
  }
  
  void ditfft2(matrix<complexNumber<double>>& x, int N) {
	if (N <= 1) {
      return;
    } 
    matrix<complexNumber<double>> even;
    even.data = std::vector<std::vector<complexNumber<double>>>(1, std::vector<complexNumber<double>>(N/2, complexNumber<double>(0.0, 0.0)));
    even.rows = 1;
    even.cols = N/2;
    matrix<complexNumber<double>> odd;
    odd.data = std::vector<std::vector<complexNumber<double>>>(1, std::vector<complexNumber<double>>(N/2, complexNumber<double>(0.0, 0.0)));
    odd.rows = 1;
    odd.cols = N/2;
    for (int i = 0; i < N/2; i++) {
      even.data[0][i] = x.data[0][i*2];
      odd.data[0][i] = x.data[0][i*2+1];
    }
	ditfft2(even, N/2);
	ditfft2(odd, N/2);
	for (int k = 0; k < N / 2; k++) {
      complexNumber<double> t = makeComplexNumberFromPolar<double>(1.0, -2*std::numbers::pi*k/N) + odd.data[0][k];
      x.data[0][k] = even.data[0][k] + t;
      x.data[0][N / 2 + k] = even.data[0][k] - t;
	}
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
        double angle = -2*std::numbers::pi*k*n/N;
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



