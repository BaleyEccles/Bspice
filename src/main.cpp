#include "Bmaths/Bmaths.h"
#include "Circuit.h"
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

void createOctavePlotFile(std::vector<double> &time,
                          std::vector<matrix<double>> &data,
                          matrix<symbol> &names) {
  if (data.size() != time.size()) {
    std::cerr << "ERROR: Time and data have different sizes" << std::endl;
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

  Node *Node1 = new Node("e1");
  Node *Node2 = new Node("e2");
  Node *Node3 = new Node("e3");
  Node *GND = new Node("GND");

  auto Vcc = std::make_shared<VoltageSource>("Vcc", 5.0);
  auto R1 = std::make_shared<Resistor>("R1", 10e3);
  auto R2 = std::make_shared<Resistor>("R2", 10);  
  auto C1 = std::make_shared<Capacitor>("C1", 10e-6);


  Node1->addComponent(Vcc);
  Node1->addComponent(R1);

  Node2->addComponent(R1);
  Node2->addComponent(C1);

  Node3->addComponent(C1);
  Node3->addComponent(R2);
  
  GND->addComponent(R2);

  std::cout << "From Circ" << std::endl;
  Circuit circuit;
  circuit.addComponent(Vcc);
  circuit.addComponent(R1);
  circuit.addComponent(R2);
  circuit.addComponent(C1);
  circuit.addNode(Node1);
  circuit.addNode(Node2);
  circuit.addNode(Node3);
  circuit.addNode(GND);
  circuit.calculate();


  auto initalValues = circuit.initalValues;
  auto A = circuit.A;
  auto E = circuit.E;
  auto f = circuit.f;
  auto s = circuit.syms;
  A.print("A");
  E.print("E");
  f.print("f");
  s.print("s");
   

  double timeStep = 0.001;
  double endTime = 5.0;
  auto output = DAESolve(A, E, f, initalValues, timeStep, endTime);
  createOctavePlotFile(output.first, output.second, s);
  delete Node1;
  delete Node2;
  delete GND;
  return 0;
};
