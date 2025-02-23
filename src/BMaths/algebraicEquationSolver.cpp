#include "algebraicEquationSolver.h"
#include "function.h"
#include "matrix.h"

// solve matrix equtations of the form A x = f
values NewtonsMethod(matrix<multiVaribleFunction> A, matrix<double> f, matrix<symbol> syms, values guess) {
  
  int maxIt = 100000;
  const double eps = 1e-4;
  matrix<double> inputs = valuesToMatrix<double>(guess, syms);
  for (int i = 0; i < maxIt; i++) {
    auto J = getJacobian(A, guess, syms, 0.0001);
    auto AEval = A.evaluate(guess);
    auto F = (AEval * inputs) - f;
    // d = -J(-1)*F
    auto delta = J.invert().scale(-1) * F;
    inputs = inputs + delta;
    
    guess = matrixToValues(inputs, syms);
    // This feels wrong
    // I am unsure if it will keep order
    
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



matrix<double> getJacobian(matrix<multiVaribleFunction> funcMatrix, values inputs, matrix<symbol> syms, double h) {
  if (funcMatrix.cols != 1) {
    std::cerr << "ERROR: function matrix must have one column" << std::endl;
  }
  if (funcMatrix.rows != inputs.size()) {
    std::cerr << "ERROR: function matrix rows and inputs size must be the same" << std::endl;
  }
  matrix<double> J;
  J.cols = funcMatrix.rows;
  J.rows = funcMatrix.rows;
  J.createData();
  
  for (int row = 0; row < funcMatrix.rows; row++) {
    int col = 0;
    for (auto& [sym, val] : inputs) {
      J.data[row][col] = funcMatrix.data[row][0].differentiate(sym, inputs, h);
      col++;
    }
  }
  
  return J;
}


// Linear:
// solve matrix equtations of the form A x = f
matrix<double> NewtonsMethod(matrix<double> A, matrix<double> f,
                             matrix<double> guess) {
  
  // Jacobian
  // For now we are only dealing with first order polynomials
  // So the Jacobian will be the same as A
  int maxIt = 100000;
  const double eps = 1e-4;
  for (int i = 0; i < maxIt; i++) {
    auto J = A;
    auto F = (A * guess) - f;
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
