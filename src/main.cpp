#include "Circuit.h"
#include "fileParser.h"
#include "tokenParser.h"
#include <cstdio>
#include <string>
#include <vector>


int main() {
  fileParser parsedFile("../Examples/twoCapacitors.circuit");
  auto tokens = parsedFile.tokens;
  Circuit<double, double, function> circuit = createCircuitFromTokens<double, double, function>(tokens);
  circuit.calculate();
  auto& initalValues = circuit.initalValues;
  auto& A = circuit.A;
  auto& E = circuit.E;
  auto& f = circuit.f;
  auto& s = circuit.syms;
  double& stopTime = circuit.stopTime;
  double& timeStep = circuit.timeStep;
  DifferentialAlgebraicEquation<double, double, function> DAE = {A, E, f, s};
  auto output = DAESolve2(DAE, initalValues, timeStep, stopTime);
  postProcess("plotData.m", output.first, output.second, s, tokens);
  
  return 0;
};
