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



int main() {
  std::vector<std::shared_ptr<fileParser::token>> tokens = fileParser::parseFile("../Examples/inductorCapacitor.circuit");
  Circuit<double> circuit = createCircuitFromTokens<double>(tokens);
  circuit.calculate();
  auto initalValues = circuit.initalValues;
  auto A = circuit.A;
  auto E = circuit.E;
  //auto f = circuit.f;
  auto s = circuit.syms;
  function f0;
  f0.addOpperation(Opperation::multiply(0.0));
  auto f1 = std::make_shared<function>();
  f1->addOpperation(Opperation::multiply(1.0));
  auto f2 = std::make_shared<function>();
  f2->addOpperation(Opperation::multiply(1.0));
  function freqIncrease;
  freqIncrease.addBranches(branchOpperation::multiply, f1, f2);
  freqIncrease.addOpperation(Opperation::sin(5.0, 1.0, 0.0));
  matrix<function> f = {
    std::vector<std::vector<function>>{
      {f0},
      {f0},
      {f0},
      {f0},
      {freqIncrease},
      {f0}},
    1, 6
  };
  double timeStep = 0.0001;
  double endTime = 10.0;

    

  DifferentialAlgebraicEquation DAE = {A, E, f, s};
  auto output = DAESolve2(DAE, initalValues, timeStep, endTime);
  //auto output = DAESolve(A, E, f, initalValues, timeStep, endTime);

  postProcess("plotData.m", output.first, output.second, s, tokens);
  //createOctavePlotFile(output.first, output.second, s);
  return 0;
};
