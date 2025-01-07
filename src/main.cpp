#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <iomanip>
#include "Circuit.h"


matrix<double> DAESolveNextState(matrix<double> E, matrix<double> A, matrix<double> f, matrix<double> yn, double h) {
  matrix<double> Einv = E.psudoInvert();
  
  std::vector<std::vector<double>> d_yn_dtData;
  std::vector<std::vector<double>> yn1Data;
  auto EinvMulA = multiply(Einv, A);
  for (int row = 0; row < f.rows; row++) {
    auto equEinv = Einv.getRow(row);
    auto equA = A.getRow(row);
    auto equEinvMulA = EinvMulA.getRow(row);
    double equSol1 = 0.0;
    for (int i = 0; i < f.data.size(); i++) {
      equSol1 += equEinv.data[i][0]*f.data[i][0];
    }
    double equSol2 = 0.0;
    for (int i = 0; i < f.data.size(); i++) {
      equSol2 += equEinvMulA.data[i][0]*yn.data[i][0];
    }
    double sol = equSol1 - equSol2;
    double yn1 = h*sol + yn.data[row][0];
           
    yn1Data.push_back({yn1});
  }

  matrix<double> output = {
    yn1Data,
    1, (int)yn1Data.size()
  };
  return output;
};


matrix<double> y(matrix<symbol> syms, matrix<double> E, matrix<double> A, matrix<double> f, matrix<double> yn, double h) {  
  auto nextState = DAESolveNextState(E, A, f, yn, h);
  return nextState;
};

void createOctavePlotFile(std::vector<double> time, std::vector<matrix<double>> data, matrix<symbol> names) {
  if (data.size() != time.size()) {
    std::cerr << "Time and data have different sizes" << std::endl;
  }
  
  std::string filename = "plotData.m";
  std::ofstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error opening file" << std::endl;
    return;
  }
  //file << "graphics_toolkit(\"gnuplot\")\n";
  file << "set(0, 'DefaultTextFontSize', 25);";
  file << "set(0, 'DefaultAxesFontSize', 25);";
  file << ("t = [");
  for (int i = 0; i < data.size(); i++) {
    file << std::scientific << std::setprecision(5) << time[i] << " ";
  }
  for (int j = 0; j < names.rows; j++) {
    file << ("]\n" + names.data[j][0].name + " = [");
    for (int i = 0; i < data.size(); i++) {
      file << std::scientific << std::setprecision(5) << data[i].data[j][0] << " ";
    }
  }
  file << ("]\n");
  for (int j = 0; j < names.rows; j++) {
    file << ("figure(" + std::to_string(j + 1) + ");\n");
    file << ("plot(t, " + names.data[j][0].name + ")\n");
    file << ("xlabel(\"t\");\n");
    file << ("ylabel(\"" + names.data[j][0].name + "\");\n");
  }
  /* Print voltage over component
  file << ("figure(" + std::to_string(names.rows + 1) + ");\n");
  file << ("plot(t, e2 - e3)\n");
  file << ("xlabel \"t\");");
  file << ("ylabel (\"e2-e3\");");
  */
  
  file << ("pause;");
  file.close();
};


int main() {
  double C = 10e-6;
  double R = 10e3;
  double G = 1/R;
  double Vs = 5;
  
  /*
  matrix<double> A = {
    std::vector<std::vector<double>> {
      { G , -G , 0.0},
      {-G , 2*G, -G },
      {0.0, -G ,  G }},
    3, 3
  };
  
  matrix<double> E = {
    std::vector<std::vector<double>> {
      {0.0, 0.0, 0.0},
      {0.0,  C , 0.0},
      {0.0, 0.0, C  }},
    3, 3
  };

  matrix<double> f = {
    std::vector<std::vector<double>> {
      {0.0},
      {0.0},
      {0.0},
    },
    1, 3
  };


  symbol e1 = {"e1"};
  symbol e2 = {"e2"};
  symbol e3 = {"e3"};
  symbol iVs = {"iVs"};
  matrix<symbol> syms = {
    std::vector<std::vector<symbol>> {
      {e1},
      {e2},
      {e3},
    },
    1, 3
  };
  
  std::vector<matrix<double>> derivatives = {{
      std::vector<std::vector<double>> {
        {0.0},
        {0.0},
        {0.0},
      },
      1, 3
    }
  };

  // ODES:
  
  std::vector<double> time;
  std::vector<matrix<double>> output;
  matrix<double> initalValues = {
    std::vector<std::vector<double>> {{Vs}, {0.0}, {0.0}},
    1, 3
  };
  output.push_back(initalValues);
  double start_time = 0.0;
  time.push_back(start_time);
  double time_step = 0.001;
  double h = time_step;
  */
  //for (int i = 0; i < 2000; i++) {
  //  double tn = i*h - h;
  //  matrix<double> d_yn_dt = derivatives[derivatives.size() - 1];
  //  matrix<double> yn = output[output.size() - 1];
  //  auto outputs = y(syms, E, A, f, yn, h);
  //  output.push_back(outputs);
  //  time.push_back(tn);
  //
  //
  //};
  //createOctavePlotFile(time, output, syms);
  {
    Node* Node1 = new Node("e1");
    Node* Node2 = new Node("e2");
    Node* Node3 = new Node("e3");
    Node* Node4 = new Node("GND");


    auto Vcc = std::make_shared<VoltageSource>("Vcc", 5.0);
    auto R1 = std::make_shared<Resistor>("R1", 10e3);
    auto R2 = std::make_shared<Resistor>("R2", 10e3);
    auto C1 = std::make_shared<Capacitor>("C1", 10e-6);
    auto C2 = std::make_shared<Capacitor>("C2", 10e-6);


    Node1->addComponent(Vcc);
    Node1->addComponent(R1);
    Node2->addComponent(R1);
    Node2->addComponent(C1);
    Node2->addComponent(R2);
    Node3->addComponent(R2);
    Node3->addComponent(C2);

    Node4->addComponent(C1);
    Node4->addComponent(C2);




    std::cout << "From Circ" << std::endl;
    Circuit circuit;
    circuit.addNode(Node1);
    circuit.addNode(Node2);
    circuit.addNode(Node3);
    circuit.addNode(Node4);
    circuit.calculate();

    

    
    std::vector<double> time;
    std::vector<matrix<double>> output;
    auto initalValues = circuit.initalValues;
    auto A = circuit.A;
    auto E = circuit.E;
    auto f = circuit.f;
    auto syms = circuit.syms;
    output.push_back(initalValues);
    double start_time = 0.0;
    time.push_back(start_time);
    double time_step = 0.001;
    double h = time_step;
    for (int i = 0; i < 2000; i++) {
      double tn = i*h - h;
      matrix<double> yn = output[output.size() - 1];
      auto outputs = y(syms, E, A, f, yn, h);
      output.push_back(outputs);
      time.push_back(tn);
    
    
    };
    createOctavePlotFile(time, output, syms);
    
  }
  return 0;
  
};

