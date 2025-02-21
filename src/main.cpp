#include "circuit.h"
#include "fileParser.h"
#include "tokenParser.h"
#include <cstdio>
#include <string>
#include <vector>


int main() {
  fileParser parsedFile("../Examples/capacitor.circuit"); // this example does not work
  auto tokens = parsedFile.tokens;
  auto circuit = createCircuitFromTokens<double, double, function, 1, 1>(tokens);
  circuit.calculate();
  auto& initalValues = circuit.initalValues;
  auto& A = circuit.A;
  auto& E = circuit.E;
  auto& f = circuit.f;
  auto& s = circuit.syms;
  double& stopTime = circuit.stopTime;
  double& timeStep = circuit.timeStep;
  DifferentialAlgebraicEquation<double, double, function, 1, 1> DAE = {A, E, f, s};
  auto output = DAESolve2(DAE, initalValues, timeStep, stopTime);
  postProcess<1, 1>("plotData.m", output.first, output.second, s, tokens);
  
  return 0;
};
