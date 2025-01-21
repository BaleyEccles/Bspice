#include "Bmaths.h"
#include "Circuit.h"
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

matrix<double> DESolveNextState(matrix<double> E, matrix<double> A,
                                matrix<double> f, matrix<double> yn, double h) {
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
      equSol1 += equEinv.data[i][0] * f.data[i][0];
    }
    double equSol2 = 0.0;
    for (int i = 0; i < f.data.size(); i++) {
      equSol2 += equEinvMulA.data[i][0] * yn.data[i][0];
    }
    double sol = equSol1 - equSol2;
    double yn1 = h * sol + yn.data[row][0];

    yn1Data.push_back({yn1});
  }

  matrix<double> output = {yn1Data, 1, (int)yn1Data.size()};
  return output;
};

// solve matrix equtations of the form A x = F
matrix<double> AESolveNextState(matrix<double> A, matrix<double> f, matrix<double> guess) {
  // Jacobian
  // For now we are only dealing with first order polynomials
  // So the Jacobian will be the same as A
  //matrix<double> DESolveNextState(matrix<double> E, matrix<double> A,
  //                                matrix<double> f, matrix<double> yn, double h) {
  int maxIt = 100000;
  const double eps = 1e-4;
  for (int i = 0; i < maxIt; i++) {
    // Jacobian == A, as we are dealing with first order polynomials
    auto J = A;
    auto F = subtract(multiply(A, guess), f);
      // d = -J(-1)*F
    auto delta = multiply(J.invert().scale(-1), F);
    guess = add(guess, delta);

    
    if (delta.norm(2) < eps) {
      break;
    }
    if (i >= maxIt - 1) {
      std::cerr << "Newtons method did not converge" << std::endl;
      std::cout << "||delta||_2: " << std::scientific << std::setprecision(4) << delta.norm(2) << std::endl;
      A.print("A = ");
      F.print("F = ");
      guess.print("Result = ");
    }
  }
  return guess;
};

std::vector<int> getDifferentialEquations(matrix<double> E) {
  std::vector<int> DEIdx;

  for (int row = 0; row < E.rows; ++row) {
    bool isDE = false;
    for (int col = 0; col < E.cols; ++col) {
      if (E.data[row][col] != 0.0) {
        isDE = true;
      }
    }
    if (isDE) {
      DEIdx.push_back(row);
    }
  }
  return DEIdx;
}

std::vector<int> getAlgebraicEquations(matrix<double> E) {
  std::vector<int> AEIdx;

  for (int row = 0; row < E.rows; ++row) {
    bool isAE = true;
    for (int col = 0; col < E.cols; ++col) {
      if (E.data[row][col] != 0.0) {
        isAE = false;
      }
    }
    if (isAE) {
      AEIdx.push_back(row);
    }
  }
  return AEIdx;
}


matrix<double> y(matrix<symbol> syms, matrix<double> E, matrix<double> A,
                 matrix<double> f, matrix<double> yn, double h) {

  auto DERowIdx = getDifferentialEquations(E);
  int i = 0;
  matrix<double> EDE = {std::vector<std::vector<double>>{}, 0, 0};
  matrix<double> ADE = {std::vector<std::vector<double>>{}, 0, 0};
  matrix<double> fDE = {std::vector<std::vector<double>>{}, 0, 0};
  matrix<double> ynDE = {std::vector<std::vector<double>>{}, 0, 0};
  matrix<symbol> symsDE = {std::vector<std::vector<symbol>>{}, 0, 0};
  for (auto row : DERowIdx) {
    EDE.data.push_back(E.getRow(row).transpose().data[0]);
    EDE.rows++;
    ADE.data.push_back(A.getRow(row).transpose().data[0]);
    ADE.rows++;
    fDE.data.push_back(f.getRow(row).transpose().data[0]);
    fDE.rows++;
    ynDE.data.push_back(yn.getRow(row).transpose().data[0]);
    ynDE.rows++;
    symsDE.data.push_back(syms.getRow(row).transpose().data[0]);
    symsDE.rows++;
    i++;
  }
  EDE.cols = E.cols;
  ADE.cols = A.cols;
  fDE.cols = f.cols;
  ynDE.cols = yn.cols;

  std::vector<int> AEColIdx;
  for (int col = 0; col < E.cols; col++) {
    bool isZero = true;
    for (int row = 0; row < E.rows; row++) {
      if (E.data[row][col] != 0.0) {
        isZero = false;
      }
    }
    if (isZero) {
      AEColIdx.push_back(col);
    }
  }
  i = 0;
  for (auto col : AEColIdx) {
    EDE.eliminateCol(col - i);
    i++;
  }
  //std::cout << "E:" << std::endl;
  //EDE.print();
  //std::cout << "A:" << std::endl;
  //ADE.print();
  //std::cout << "f:" << std::endl;
  //fDE.print();
  // Should be:
  // E = [ C 0
  //       0 C ]

  // xn1   = E(-1)(f - A xn) h
  //std::cout << "ynDE" << std::endl;
  //yn.print("yn");
  //ADE.print("ADE");
  auto a = multiply(EDE.invert(), subtract(fDE, multiply(ADE, yn))).scale(h);
  //multiply(EDE.invert(), subtract(fDE, multiply(ADE, yn))).print("multiply");
  //multiply(EDE.invert(), subtract(fDE, multiply(ADE, yn))).scale(h).print("mult inv");
  //a.print();
  auto xn1 = add(a, ynDE);
  //ynDE.print("ynDE");
  //xn1.print("xn1");
  // Now we know e2(n+1) and e3(n+1)
  // pad xn1 with zeros so we have it of the form [ 0 e2 e3 0 0]T
  matrix<double> xn1New = {std::vector<std::vector<double>>(
                               syms.rows, std::vector<double>(syms.cols, 0.0)),
                           syms.cols, syms.rows};
  i = 0;
  for (auto row : DERowIdx) {
    xn1New.data[row][0] = xn1.data[i][0];
    i++;
  }
  //xn1New.print();
  // AE Equations
  matrix<double> An1 = A;
  //A.print();
  i = 0;
  for (auto row : DERowIdx) {
    An1.eliminateRow(row - i);
    i++;
  }
  //An1.print();

  std::vector<int> DEColIdx;
  for (int col = 0; col < E.cols; col++) {
    bool isZero = false;
    for (int row = 0; row < E.rows; row++) {
      if (E.data[row][col] != 0.0) {
        isZero = true;
      }
    }
    if (isZero) {
      DEColIdx.push_back(col);
    }
  }
  
  auto An = An1;
  i = 0;
  for (auto col : DEColIdx) {
    An.eliminateCol(col - i);
    i++;
  }
  // Now we need to solve the equation:
  // An xn = f - An1 xn1
  //std::cout << "An:" << std::endl;
  //An.print();
  //std::cout << "f:" << std::endl;
  i = 0;
  for (auto row : DERowIdx) {
    f.eliminateRow(row - i);
    i++;
  }
  //f.print();
  //An1.print();
  //xn1New.print();
  auto An1xn1 = multiply(An1, xn1New);
  //An1xn1.print();
  auto newf = subtract(f, An1xn1);
  //newf.print();

  auto NewtonGuess = yn;
  i = 0;
  for (auto row : DERowIdx) {
    NewtonGuess.eliminateRow(row - i);
    i++;
  }
  auto AEsols = AESolveNextState(An, newf, NewtonGuess);
  //std::cout << "AE sols:" << std::endl;
  //AEsols.print();

  matrix<double> AEsolsNew = {
      std::vector<std::vector<double>>(syms.rows,
                                       std::vector<double>(syms.cols, 0.0)),
      syms.cols, syms.rows};
  auto AERowIdx = getAlgebraicEquations(E);
  i = 0;
  for (auto row : AERowIdx) {
    AEsolsNew.data[row][0] = AEsols.data[i][0];
    i++;
  }
  // AEsolsNew.print();
  auto output = add(AEsolsNew, xn1New);
  //output.print();
  return output;
  // Now we have both the DE and AE solutions in AEsols and xn1
  
  
  /*auto DEnextState = DESolveNextState(E, A, f, yn, h);
  //std::cout << "DE sols:" << std::endl;
  //DEnextState.print();


  auto AEIdx = getAlgebraicEquations(E);

  matrix<double> DEnewNextState = {
  std::vector<std::vector<double>>(f.rows, std::vector<double>(f.cols, 0)),
  f.cols,
  f.rows
  };

  for (auto varible : DEcols) {
  DEnewNextState.data[varible][0] = DEnextState.data[varible][0];
  }

  int i = 0;
  for (auto row : DEIdx) {
  A.eliminateRow(row - i);
  i++;
  }


  i = 0;
  for (auto col : DEcols) {
  A.eliminateCol(col - i);
  i++;
  }
  //A.print();



  auto newf = subtract(f, DEnewNextState);
  i = 0;
  for (auto row : DEIdx) {
  newf.eliminateRow(row - i);
  i++;
  }
  std::cout << "A" << std::endl;
  A.print();
  std::cout << "new f" << std::endl;
  newf.print();
  auto AEnextState = AESolveNextState(E, A, newf, newf, h);
  // std::cout << "AE sols:" << std::endl;
  // AEnextState.print();


  i = 0;
  for (auto row : AEIdx) {
  //std::cout << row << std::endl;
  DEnextState.data[row][0] = AEnextState.data[i][0];
  i++;
  }
  std::cout << "DE" << std::endl;
  DEnextState.print();
  std::cout << "AE" << std::endl;
  AEnextState.print();

  return DEnextState;*/
  return xn1;
};

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
    file << ("]\n" + names.data[j][0].name + " = [");
    for (int i = 0; i < data.size(); i++) {
      file << std::scientific << std::setprecision(5) << data[i].data[j][0]
           << " ";
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
  // file << ("figure(" + std::to_string(names.rows + 1) + ");\n");
  // file << ("plot(t, e2 - e3)\n");
  // file << ("xlabel \"t\");");
  // file << ("ylabel (\"e2-e3\");");

  file << ("pause;");
  file.close();
};

int main() {
  double C = 10e-6;
  double R = 10e3;
  double G = 1 / R;
  double Vs = 5;

  matrix<double> A = {std::vector<std::vector<double>>{{G, -G, 0.0, 1.0},
                                                       {-G, 2 * G, -G, 0.0},
                                                       {0.0, -G, G, 0.0},
                                                       {1.0, 0.0, 0.0, 0.0}},
                      4, 4};

  matrix<double> E = {std::vector<std::vector<double>>{{0.0, 0.0, 0.0, 0.0},
                                                       {0.0, C, 0.0, 0.0},
                                                       {0.0, 0.0, C, 0.0},
                                                       {0.0, 0.0, 0.0, 0.0}},
                      4, 4};

  matrix<double> f = {std::vector<std::vector<double>>{
                          {0.0},
                          {0.0},
                          {0.0},
                          {Vs},
                      },
                      1, 4};

  symbol e1 = {"e1"};
  symbol e2 = {"e2"};
  symbol e3 = {"e3"};
  symbol iVs = {"iVs"};
  matrix<symbol> syms = {
      std::vector<std::vector<symbol>>{{e1}, {e2}, {e3}, {iVs}}, 1, 4};

  std::vector<double> time;
  std::vector<matrix<double>> output;
  matrix<double> initalValues = {
      std::vector<std::vector<double>>{{Vs}, {0.0}, {0.0}, {0.0}}, 1, 4};

  //A.print();
  //E.print();
  //f.print();
  //initalValues.print();

  // output.push_back(initalValues);
  // double start_time = 0.0;
  // time.push_back(start_time);
  // double time_step = 0.001;
  // double h = time_step;
  // for (int i = 0; i < 2000; i++) {
  //   double tn = i*h - h;
  //   matrix<double> yn = output[output.size() - 1];
  //   auto outputs = y(syms, E, A, f, yn, h);
  //   output.push_back(outputs);
  //   time.push_back(tn);
  // };
  // createOctavePlotFile(time, output, syms);

  if (true) {
    matrix<symbol> syms;
    syms = {{
                {symbol("e1")},
                {symbol("e2")},
                {symbol("iVs")},
            },
            1,
            3};
    Node *Node1 = new Node("e1");
    Node *Node2 = new Node("e2");
    Node *Node3 = new Node("e3");
    Node *GND = new Node("GND");

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

    GND->addComponent(C1);
    GND->addComponent(C2);

    std::cout << "From Circ" << std::endl;
    Circuit circuit;
    circuit.addNode(Node1);
    circuit.addNode(Node2);
    circuit.addNode(Node3);
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
      double tn = i * h - h;
      matrix<double> yn = output[output.size() - 1];
      auto outputs = y(s, E, A, f, yn, h);
      output.push_back(outputs);
      time.push_back(tn);
    };
    createOctavePlotFile(time, output, s);
    delete Node1;
    delete Node2;
    delete GND;
  }
  return 0;
};
