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
  Circuit circuit = createCircuitFromTokens(tokens);
  circuit.calculate();
  auto initalValues = circuit.initalValues;
  auto A = circuit.A;
  auto E = circuit.E;
  auto f = circuit.f;
  auto s = circuit.syms;

  double timeStep = 0.001;
  double endTime = 1.0;
  auto output = DAESolve(A, E, f, initalValues, timeStep, endTime);

  postProcess("plotData.m", output.first, output.second, s, tokens);
  //createOctavePlotFile(output.first, output.second, s);
  
  if (false) {
    Node *Node1 = new Node("e1");
    Node *Node2 = new Node("e2");
    Node *Node3 = new Node("e3");
    Node *Node4 = new Node("e4");
    Node *GND = new Node("GND");

    auto Vcc = std::make_shared<VoltageSource>("Vcc", 5.0);
    auto R1 = std::make_shared<Resistor>("R1", 10);
    auto L1 = std::make_shared<Inductor>("L1", 1);

    Node1->addComponent(Vcc);
    Node1->addComponent(R1);

    Node2->addComponent(R1);
    Node2->addComponent(L1);

    GND->addComponent(L1);

    std::cout << "From Circ" << std::endl;
    Circuit circuit;
    circuit.addNode(Node1);
    circuit.addNode(Node2);
    circuit.addNode(GND);
    circuit.calculate();

    auto initalValues = circuit.initalValues;
    auto A = circuit.A;
    auto E = circuit.E;
    auto f = circuit.f;
    auto s = circuit.syms;

    double timeStep = 0.001;
    double endTime = 1.0;
    auto output = DAESolve(A, E, f, initalValues, timeStep, endTime);
    createOctavePlotFile(output.first, output.second, s);
    delete Node1;
    delete Node2;
    delete GND;
    return 0;
  }
};
