#include "Bmaths/Bmaths.h"
#include "Circuit.h"
#include "fileParser.h"
#include "tokenParser.h"
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <numbers>

template<typename T>
matrix<complexNumber<double>> DFT(matrix<T> input) {
  if (input.rows != 1) {
    std::cerr << "ERROR: Rows of input must be 1." << std::endl;
  }
  int N = input.cols;

  matrix<complexNumber<double>> output;
  output.rows = 1;
  output.cols = N;
  std::vector<std::vector<complexNumber<double>>> outputData(output.rows, std::vector<complexNumber<double>>(output.cols, complexNumber<double>(0.0, 0.0)));
  output.data = outputData;
  for (int k = 0; k < N; k++) {
    for (int n = 0; n < N; n++) {
      auto data = input.data[0][n];
      double angle = -2*std::numbers::pi*k*n/N;
      output.data[0][k] = output.data[0][k] + (data*makeComplexNumberFromPolar<double>(1, angle));
    }
  }
  return output;
}


int main() {
  std::vector<std::shared_ptr<fileParser::token>> tokens = fileParser::parseFile("../Examples/inductorCapacitorAC.circuit");
  Circuit<double, double, function> circuit = createCircuitFromTokens<double, double, function>(tokens);
  circuit.calculate();
  auto initalValues = circuit.initalValues;
  auto A = circuit.A;
  auto E = circuit.E;
  auto f = circuit.f;
  auto s = circuit.syms;
  double timeStep = 0.001;
  double endTime = 1.0;
  DifferentialAlgebraicEquation DAE = {A, E, f, s};
  auto output = DAESolve2(DAE, initalValues, timeStep, endTime);
  postProcess("plotData.m", output.first, output.second, s, tokens);
  matrix<complexNumber<double>> testDFT = {
    {{
        complexNumber<double>(1, 0),
        complexNumber<double>(0.5, 0),
        complexNumber<double>(0.0, 0),
        complexNumber<double>(-1, 0)
      }},
    4, 1
  };
  auto DFTdata1 = DFT(testDFT);
  
  matrix<double> e1;
  e1.data.push_back({});
  e1.rows = 1;
  e1.cols = 0;
  for (auto& m : output.second) {
    e1.data[0].push_back(m.data[1][0]);
    e1.cols++;
  }

  auto DFTdata = DFT(e1);
  matrix<double> magDFTdata;
  magDFTdata.cols = DFTdata.cols;
  magDFTdata.rows = DFTdata.rows;
  std::vector<std::vector<double>> magDFTdataData(magDFTdata.rows, std::vector<double>(magDFTdata.cols, 0.0));
  magDFTdata.data = magDFTdataData;
  for (int row = 0; row < DFTdata.rows; row++) {
    for (int col = 0; col < DFTdata.cols; col++) {
      magDFTdata.data[row][col] = DFTdata.data[row][col].magnitude();
    }
  }
  e1.print("e1:");
  magDFTdata.print("output:");

  return 0;
};
