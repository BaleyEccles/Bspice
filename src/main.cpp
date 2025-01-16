#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <iomanip>
#include "Bmaths.h"
#include "Circuit.h"


matrix<double> DAESolveNextState(matrix<double> E, matrix<double> A, matrix<double> f, matrix<double> yn, double h) {
  matrix<double> Einv = E.pseudoInvert();
  
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

matrix<double> AESolveNextState(matrix<double> E, matrix<double> A, matrix<double> f, matrix<double> yn, double h) {
  // Jacobian
  // For now we are only dealing with first order polynomials
  // So the Jacobian will be the same as A
  int maxIt = 1000;
  const double eps = 1e-6;
  for (int i = 0; i < maxIt; i++) {
    auto F = subtract(multiply(A, yn), f);
    auto J = A;
    // d = -J(-1)*F
    auto delta = multiply(J.invert().scale(-1),F);
    yn = add(yn, delta);
    if (delta.norm(2) < eps) {

      //std::cerr << "Newtons method did converge" << std::endl;
      //yn.print();
      break;
    }
    if (i >= maxIt - 3) {
      //      std::cerr << "Newtons method did not converge" << std::endl;
      //      yn.print();
    }
  }
  return yn;
};
matrix<double> y(matrix<symbol> syms, matrix<double> E, matrix<double> A, matrix<double> f, matrix<double> yn, double h) {
  auto nextState = AESolveNextState(E, A, f, yn, h);  
  //auto nextState = DAESolveNextState(E, A, f, yn, h);
  return nextState;
};

void createOctavePlotFile(std::vector<double>& time, std::vector<matrix<double>>& data, matrix<symbol>& names) {
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
    file << ("xlabel(\"t\");\n");
    file << ("ylabel(\"" + names.data[j][0].name + "\");\n");
  }

  /// Print voltage over component
  //file << ("figure(" + std::to_string(names.rows + 1) + ");\n");
  //file << ("plot(t, e2 - e3)\n");
  //file << ("xlabel \"t\");");
  //file << ("ylabel (\"e2-e3\");");
  

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
  //A.print();
  //E.print();
  //f.print();
  //initalValues.print();
    
  {
    matrix<symbol> syms;
    syms = {
      {{symbol("e1")},
       {symbol("e2")},
       {symbol("iVs")},
      },
      1, 3
    };
    Node* Node1 = new Node("e1");
    Node* Node2 = new Node("e2");
    //Node* Node3 = new Node("e3");
    Node* GND = new Node("GND");


    auto Vcc = std::make_shared<VoltageSource>("Vcc", 5.0);
    auto R1 = std::make_shared<Resistor>("R1", 10e3);
    auto R2 = std::make_shared<Resistor>("R2", 10e3);
    //auto C1 = std::make_shared<Capacitor>("C1", 10e-6);
    //auto C2 = std::make_shared<Capacitor>("C2", 10e-6);


    Node1->addComponent(Vcc);
    Node1->addComponent(R1);
    Node2->addComponent(R1);
    //Node2->addComponent(C1);
    Node2->addComponent(R2);
    GND->addComponent(R2);
    //Node3->addComponent(C2);

    //Node4->addComponent(C1);
    //Node4->addComponent(C2);


    std::cout << "From Circ" << std::endl;
    Circuit circuit;
    circuit.addNode(Node1);
    circuit.addNode(Node2);
    //circuit.addNode(Node3);
    circuit.addNode(GND);
    circuit.calculate();

    std::vector<double> time;
    std::vector<matrix<double>> output;
    auto initalValues = circuit.initalValues;
    auto A = circuit.A;
    auto E = circuit.E;
    auto f = circuit.f;
    auto s = circuit.syms;
    circuit.syms.print();

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
    createOctavePlotFile(time, output, s);
    delete Node1;
    delete Node2;
    //delete Node3;
    delete GND;
  }
  return 0;
  
};
