#include "Bmaths/Bmaths.h"
#include "Circuit.h"
#include "fileParser.h"
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

void createOctavePlotFile(std::vector<double> &time,
                          std::vector<matrix<double>> &data,
                          matrix<symbol> &names) {
  if (data.size() != time.size()) {
    std::cerr << "Time and data have different sizes" << std::endl;
  }

  std::string filename = "plotData.m";
  std::ofstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error opening file" << std::endl;
    return;
  }
  file << "graphics_toolkit(\"gnuplot\")\n";
  file << "set(0, 'DefaultTextFontSize', 25);";
  file << "set(0, 'DefaultAxesFontSize', 25);";
  file << ("t = [");
  for (int i = 0; i < data.size(); i++) {
    file << std::scientific << std::setprecision(5) << time[i] << " ";
  }
  for (int j = 0; j < names.rows; j++) {
    file << ("];\n" + names.data[j][0].name + " = [");
    for (int i = 0; i < data.size(); i++) {
      file << std::scientific << std::setprecision(5) << data[i].data[j][0]
           << " ";
    }
  }
  file << ("];\n");

  for (int j = 0; j < names.rows; j++) {
    file << ("figure(" + std::to_string(j + 1) + ");\n");
    file << ("plot(t, " + names.data[j][0].name + ");\n");
    file << ("xlabel(\"t\");\n");
    file << ("ylabel(\"" + names.data[j][0].name + "\");\n");
  }

  /// Print voltage over component
  // file << ("figure(" + std::to_string(names.rows + 1) + ");\n");
  // file << ("plot(t, e2 - e3)\n");
  // file << ("xlabel \"t\");");
  // file << ("ylabel (\"e2-e3\");");

  file << ("pause;");
  file.close();
};


int main() {
  std::vector<std::shared_ptr<token>> tokens = fileParser::parseFile("example.circuit");

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
