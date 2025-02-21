#include "algebraicEquationSolver.h"
#include "function.h"
#include <utility>

// solve matrix equtations of the form A x = f
matrix<double> NewtonsMethod(matrix<multiVaribleFunction> A, matrix<symbol> syms, matrix<double> f, matrix<double> guess) {
  
  int maxIt = 100000;
  const double eps = 1e-4;
  
  for (int i = 0; i < maxIt; i++) {
    auto inputs = getInputs(syms, guess); // I dont thing using guess works for the first step?
    auto J = getJacobian(A, inputs, 0.0001);
    //auto J = A;
    auto AEval = A.evaluate(inputs);
    auto F = (AEval * guess) - f;
    // d = -J(-1)*F
    auto delta = J.invert().scale(-1) * F;
    guess = guess + delta;

    
    if (delta.norm(2) < eps) {
      break;
    }
    if (i >= maxIt - 1) {
      std::cerr << "Newtons method did not converge" << std::endl;
      std::cout << "||delta||_2: " << std::scientific << std::setprecision(4) << delta.norm(2) << std::endl;
      //A.print("A = ");
      //F.print("F = ");
      //guess.print("Result = ");
    }
  }
  return guess;
};


std::vector<std::pair<symbol, double>> getInputs(matrix<symbol> syms, matrix<double> inputs) {
  std::vector<std::pair<symbol, double>> output;
  for (int row = 0; row < syms.rows; row++) {
    output.push_back(std::make_pair(syms.data[row][0], inputs.data[row][0]));
  }
  return output;
}

matrix<double> getJacobian(matrix<multiVaribleFunction> funcMatrix, std::vector<std::pair<symbol, double>> inputs, double h) {
  if (funcMatrix.cols != 1) {
    std::cerr << "ERROR: function matrix must have one column" << std::endl;
  }
  if (funcMatrix.rows != inputs.size()) {
    std::cerr << "ERROR: function matrix rows and inputs size must be the same" << std::endl;
  }
  for (int row = 0; row < funcMatrix.rows; row++) {
    funcMatrix.data[row][0].getVaribles();
  }
  auto outputData = std::vector<std::vector<double>>(funcMatrix.rows, std::vector<double>(funcMatrix.cols, 0.0));
  matrix<double> output = {
    outputData,
    funcMatrix.rows, funcMatrix.rows
  };

  for (int row = 0; row < output.rows; row++) {
    for (int col = 0; col < output.cols; col++) {
      output.data[row][col] = funcMatrix.data[row][0].differentiate(inputs[col].first, inputs, h);
    }
  }
  return output;
}
