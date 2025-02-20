#include "circuit.h"
#include "fileParser.h"
#include "tokenParser.h"
#include <cstdio>
#include <string>
#include <vector>


int main() {
  
  fileParser parsedFile("../Examples/diode.circuit");
  auto tokens = parsedFile.tokens;
  
  if (parsedFile.isLinear) {
    auto circuit = createCircuitFromTokens<double, double, function>(tokens);
    circuit.calculate();
    
    double& stopTime = circuit.stopTime;
    double& timeStep = circuit.timeStep;
    DifferentialAlgebraicEquation<double, double, function> DAE = {circuit.A, circuit.E, circuit.f, circuit.symbols};
    
    auto output = DAESolveLinear(DAE, circuit.initalValues, timeStep, stopTime);
    postProcess("plotData.m", output.first, output.second, circuit.symbols, tokens);
  } else {
    auto circuit = createCircuitFromTokens<multiVaribleFunction, multiVaribleFunction, function>(tokens);
    circuit.calculate();
    
    double& stopTime = circuit.stopTime;
    double& timeStep = circuit.timeStep;
    DifferentialAlgebraicEquation<multiVaribleFunction, multiVaribleFunction, function> DAE = {circuit.A, circuit.E, circuit.f, circuit.symbols};

    auto output = DAESolveNonLinear(DAE, circuit.initalValues, timeStep, stopTime);
    postProcess("plotData.m", output.first, output.second, circuit.symbols, tokens);
  }

  return 0;
};
