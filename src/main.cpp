#include "Bmaths/Bmaths.h"
#include "Circuit.h"
#include "fileParser.h"
#include "tokenParser.h"
#include <cstdio>
#include <string>
#include <vector>


int main() {
  fileParser parsedFile("../Examples/diode.circuit");
  auto tokens = parsedFile.tokens;
  Circuit<double, double, function> circuit = createCircuitFromTokens<double, double, function>(tokens);
  circuit.calculate();
  auto initalValues = circuit.initalValues;
  auto A = circuit.A;
  auto E = circuit.E;
  auto f = circuit.f;
  auto s = circuit.syms;
  double timeStep = 0.0001;
  double endTime = 1;
  DifferentialAlgebraicEquation<double, double, function> DAE = {A, E, f, s};
  auto output = DAESolve2(DAE, initalValues, timeStep, endTime);
  postProcess("plotData.m", output.first, output.second, s, tokens);
  
  return 0;
};
