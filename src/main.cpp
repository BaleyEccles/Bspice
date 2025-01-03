#include <boost/numeric/odeint.hpp>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "Bmaths.h"



matrix<double> DAESolveNextState(matrix<double> E, matrix<double> A, matrix<double> f, matrix<double> yn, double h) {
  matrix<double> Einv = E.psudoInvert();
  
  std::vector<std::vector<double>> d_yn_dtData;
  std::vector<std::vector<double>> yn1Data;
  auto EinvMulA = Einv.multiply(Einv, A);
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
  file << "graphics_toolkit(\"gnuplot\")\n";
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
    file << ("xlabel \"t\");");
    file << ("ylabel (\"" + names.data[j][0].name + "\");");
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
  for (int i = 0; i < 2000; i++) {
    double tn = i*h - h;
    matrix<double> d_yn_dt = derivatives[derivatives.size() - 1];
    matrix<double> yn = output[output.size() - 1];
    auto outputs = y(syms, E, A, f, yn, h);
    output.push_back(outputs);
    time.push_back(tn);


  };
  createOctavePlotFile(time, output, syms);

  
  
  //// y''' = sin(t) + cos(t)y +t2 y' - 4 t y''
  //// t0 = 0
  //// y0 = y(t0) = 2
  //// y'0 = y'(t0) = -1
  //// y''0 = y''(t0) = 3
  //// h = 0.5
  //std::vector<double> time;
  //std::vector<std::vector<double>> output;
  //output.push_back(std::vector<double>{2.0, -1.0, 3.0});
  //time.push_back(0.0);
  //double time_step = 0.01;
  //for (int i = 1; i < 1000; i++) {
  //
  //  double h = time_step;
  //  double tn = i*h - h;
  //
  //  std::vector<double> yn = output[output.size() - 1];
  //  std::vector<double> yn1 = y(yn, tn, h);
  //  output.push_back(yn1);
  //  time.push_back(tn);
  //};
  //printf("t = [");
  //for (int i = 0; i < output.size(); i++) {
  //  printf("%.5e ", time[i]);
  //}
  //printf("]\ny = [");
  //for (int i = 0; i < output.size(); i++) {
  //  printf("%.5e ", output[i][0]);
  //}
  //printf("]\ny1 = [");
  //for (int i = 0; i < output.size(); i++) {
  //  printf("%.5e ", output[i][1]);
  //}
  //printf("]\ny2 = [");
  //for (int i = 0; i < output.size(); i++) {
  //  printf("%.5e ", output[i][2]);
  //}
  //printf("]\n");


  
  //  Node* Node1 = new Node("N1");
  //  Node* Node2 = new Node("N2");
  //  Node* Node3 = new Node("N3");
  //  
  //  VoltageSource* Vcc = new VoltageSource("Vcc", 5);
  //  Resistor* R1 = new Resistor("R1", 100);
  //  Resistor* R2 = new Resistor("R2", 100);
  //  
  //  Node1->addComponent(Vcc);
  //  Node1->addComponent(R1);
  //  Node2->addComponent(R1);
  //  Node2->addComponent(R2);
  //  Node3->addComponent(R2);
  //  Node3->addComponent(Vcc);
  //
  //  Circuit circuit;
  //  circuit.addNode(Node1);
  //  circuit.addNode(Node2);
  //  circuit.addNode(Node3);
  //
  //  circuit.calculate();
  
  return 0;
  
};

