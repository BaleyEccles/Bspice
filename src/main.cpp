#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <iomanip>
#include "Bmaths.h"
#include "Circuit.h"


matrix<double> DESolveNextState(matrix<double> E, matrix<double> A, matrix<double> f, matrix<double> yn, double h) {
  matrix<double> Einv = E.pseudoInvert(); // This is wrong, but works for now
  
  std::vector<std::vector<double>> d_yn_dtData;
  std::vector<std::vector<double>> yn1Data;
  auto EinvMulA = multiply(Einv, A.transpose());
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

std::vector<int> getDifferentialEquationsIndex(matrix<double> E) {
  std::vector<int> output;
  
  const double eps = 1e-300;
  for (int row = 0; row < E.rows; row++) {
    bool IsDE = false;
    for (int col = 0; col < E.cols; col++) {
      if (std::abs(E.data[row][col]) > eps) {
        IsDE = true;
      }
    }
    
    if (IsDE) {
      output.push_back(row);
    }
  }

  return output;
};

template<typename T>
matrix<T> getRows(matrix<T> M, std::vector<int> idxs) {
  matrix<T> Mout {
    std::vector<std::vector<T>>{},
    M.cols, (int)idxs.size()
  };
  for (auto idx : idxs) {
    matrix<T> row = M.getRow(idx).transpose();
    Mout.data.push_back(row.data[0]);
  }
  return Mout;
}

std::vector<int> getAlgebraicEquationsIndex(matrix<double> E) {
  std::vector<int> output;
  bool IsDE = true;
  for (int row = 0; row < E.rows; row++) {
    IsDE = true;
    for (int col = 0; col < E.cols; col++) {
      if (E.data[row][col] != 0.0) {
        IsDE = false;
      }
    }
    if (IsDE) {
      output.push_back(row);
    }
  }
  return output;
};

// now we need to solve
// Ex' + Ax = f
// where we have E = [0]
// => x = A(-1)f
// We would expect that there arent enough equations to solve this,
// but we have solved t he DE component, so we can subsitute those values in to A and f
matrix<double> AlgSolveNextState(matrix<double> DEsol, std::vector<int> DEIdx, matrix<double> A, matrix<double> f, matrix<double> yn, double h) {
  // change A and f
  //f = f - A(DEsol)
  for (auto idx : DEIdx) {
    for (int row = 0; row < f.rows; row++) {
      double Aval = A.data[row][idx];
      double DEval = DEsol.data[idx][0];
      f.data[row][0] -= DEval*Aval;
    }
  }
  // A = remove DEIdx vals
  A.cols -= DEIdx.size();;
  for (int row = 0; row < A.rows; row++) {
    for (auto it : DEIdx) {
      A.data[row].erase(A.data[row].begin() + it - 1);
    }
  }

  // Now we do x = A(-1)f
  matrix<double> Ainvf = multiply(A.invert(), f);
  return Ainvf;
    
}


matrix<double> y(matrix<symbol> syms, matrix<double> E, matrix<double> A, matrix<double> f, matrix<double> yn, double h) {
  // The DAE can be broken down into a set of differentail equations and a set of algerbraic equations.
  // first we solve the differential components, then the algerbraic parts.
  std::vector<int> DEIdx = getDifferentialEquationsIndex(E);
  matrix<symbol> DEsSyms = getRows(syms, DEIdx);
  matrix<double> DEsA = getRows(A, DEIdx);
  matrix<double> DEsE = getRows(E, DEIdx);
  matrix<double> DEsf = getRows(f, DEIdx);

  std::vector<int> AlgIdx = getAlgebraicEquationsIndex(E);
  matrix<symbol> AlgSyms = getRows(syms, AlgIdx);
  matrix<double> AlgA = getRows(A, AlgIdx);
  matrix<double> AlgE = getRows(E, AlgIdx);
  matrix<double> Algf = getRows(f, AlgIdx);
  //auto nextState = DEsolveNextState(E, A, f, yn, h);

  //E.print();
  auto DESolution = DESolveNextState(E, A, f, yn, h);

  auto AlgSolution = AlgSolveNextState(DESolution, DEIdx, AlgA, Algf, yn, h);

  for (auto i : DEIdx) {
    AlgSolution.data.insert(AlgSolution.data.begin() + i, {0.0});
    AlgSolution.rows += 1;
  }
  // 5.00000 
  // -0.49950 
  // 0.00000 
  // 0.50000

  //auto nextState = add(AlgSolution, DESolution);
  //auto nextState = AlgSolution;
  // This is wrong
  auto nextState = AlgSolution;
  for (auto idx : DEIdx) {
    nextState.data[idx][0] += DESolution.data[idx][0];
  }  
  //std::cout << "ALG:" << std::endl;
  //AlgSolution.print();
  //std::cout << "DE:" << std::endl;
  //DESolution.print();
  //std::cout << "NS:" << std::endl;
  //nextState.print();

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

  for (int row = 0; row < names.rows; row++) {
    file << ("]\n" + names.data[row][0].name + " = [");
    for (int i = 0; i < data.size(); i++) {
      file << std::scientific << std::setprecision(5) << data[i].data[row][0] << " ";
    }
  }

  file << ("]\n");
  for (int j = 0; j < names.rows; j++) {
    file << ("figure(" + std::to_string(j + 1) + ");\n");
    file << ("plot(t, " + names.data[j][0].name + ")\n");
    file << ("xlabel(\"t\");\n");
    file << ("ylabel(\"" + names.data[j][0].name + "\");\n");
  }

  // Print voltage over component
  //file << ("figure(" + std::to_string(names.rows + 1) + ");\n");
  //file << ("plot(t, e2 - e3)\n");
  //file << ("xlabel \"t\")\n;");
  //file << ("ylabel (\"e2-e3\")\n;");
  

  file << ("pause;");
  file.close();
};


int main() {
  double L = 1.0;
  double R = 10.0;
  double G = 1/R;
  double Vs = 5;
  
  matrix<double> A = {
    std::vector<std::vector<double>> {
      {G, -G, 1, 0},
      {-G, 0, 0, 1},
      {0 , 1, 0, 0},
      {1, 0, 0, 0}},
    4, 4
  };

  matrix<double> E = {
    std::vector<std::vector<double>>{
      {0, 0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, -L},
      {0, 0, 0, 0}},
    4, 4
  };
  
  matrix<double> f = {
    std::vector<std::vector<double>> {
      {0.0},
      {0.0},
      {0.0},
      {Vs}},
    1, 4
  };

  symbol e1 = {"e1"};
  symbol e2 = {"e2"};
  symbol iVs = {"iVs"};
  symbol iL = {"iL"};
  matrix<symbol> syms = {
    std::vector<std::vector<symbol>> {
      {e1},
      {e2},
      {iVs},
      {iL}
    },
    1, 4
  };
  std::vector<double> time;
  std::vector<matrix<double>> output;
  matrix<double> initalValues = {
    std::vector<std::vector<double>> {
      {0.0},
      {0.0},
      {0.0},
      {0.0}},
    1, 4
  };
  output.push_back(initalValues);
  double start_time = 0.0;
  time.push_back(start_time);
  double time_step = 0.001;
  double h = time_step;

  for (int i = 0; i < 5000; i++) {
    double tn = i*h - h;
    matrix<double> yn = output[output.size() - 1];
    auto outputs = y(syms, E, A, f, yn, h);
    output.push_back(outputs);
    time.push_back(tn);
  
  };
  createOctavePlotFile(time, output, syms);
  A.print();
  E.print();
  f.print();
  initalValues.print();
  bool doNext = false;
  if (doNext) {
    Node* Node1 = new Node("e1");
    Node* Node2 = new Node("e2");
    Node* Node3 = new Node("e3");
    Node* Node4 = new Node("GND");


    auto Vcc = std::make_shared<VoltageSource>("Vcc", 5.0);
    auto R1 = std::make_shared<Resistor>("R1", 10e3);
    auto R2 = std::make_shared<Resistor>("R2", 10e3);
    auto R3 = std::make_shared<Resistor>("R3", 10e3);
    auto C1 = std::make_shared<Capacitor>("C1", 10e-6);
    auto C2 = std::make_shared<Capacitor>("C2", 10e-6);


    Node1->addComponent(Vcc);
    Node1->addComponent(R1);
    Node1->addComponent(R3);
    Node2->addComponent(R1);
    Node2->addComponent(C1);
    Node2->addComponent(R2);
    Node3->addComponent(R2);
    Node3->addComponent(C2);

    Node4->addComponent(C1);
    Node4->addComponent(C2);
    Node4->addComponent(R3);


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
    auto s = circuit.syms.data;
    matrix<symbol> syms = {{{s[0]}, {s[1]}, {s[2]}}, circuit.syms.cols, circuit.syms.rows - 1};
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
    delete Node1;
    delete Node2;
    delete Node3;
    delete Node4;
  }
  return 0;
  
};

