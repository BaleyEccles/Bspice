#include "algebraicEquationSolver.h"
// solve matrix equtations of the form A x = f
template <std::size_t colsT, std::size_t rowsT>
matrix<double, colsT, rowsT> NewtonsMethod(matrix<double, colsT, rowsT> A, matrix<double, colsT, rowsT> f, matrix<double, colsT, rowsT> guess) {
  
  // Jacobian
  // For now we are only dealing with first order polynomials
  // So the Jacobian will be the same as A
  int maxIt = 100000;
  const double eps = 1e-4;
  for (int i = 0; i < maxIt; i++) {
    // FIXME: Jacobian == A, as we are dealing with first order polynomials
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
