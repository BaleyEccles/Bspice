#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <cmath>
#include <iomanip>

struct symbol {
  std::string name;
  symbol(const std::string& n);
  symbol() : name("NULL") {}
};

template<typename T>
struct matrix {
  std::vector<std::vector<T>> data;
  int cols, rows;

  matrix<T> scale(double scale) {
    std::vector<std::vector<T>> outputData(rows, std::vector<T>(cols, 0));
    for (int i = 0; i < rows; ++i) {
      for (int j = 0; j < cols; ++j) {
        outputData[j][i] = scale*data[j][i];
      }
    }
    matrix<T> output = {
      outputData,
      cols, rows
    };
    return output;
  }

  matrix<T> getColumn(int col) {
    std::vector<std::vector<T>> output;
    for (int i = 0; i < rows; i++) {
      output.push_back({data[i][col]});
    }
    return matrix<T>{output, 1, rows};
  }
  matrix<T> getRow(int row) {
    std::vector<std::vector<T>> output;
    for (int i = 0; i < cols; i++) {
      output.push_back({data[row][i]});
    }
    return matrix<T>{output, 1, cols};
  }
  
  void print() {
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        if constexpr (std::is_arithmetic<T>::value) {
          // If T is an arithmetic type (int, float, double, etc.)
          std::cout << std::fixed << std::setprecision(5) << data[i][j] << " ";
        } else if constexpr (std::is_same<T, symbol>::value) {
          // If T is a string
          std::cout << data[i][j].name << " ";
        }
      }
      std::cout << "\n";
    }
    std::cout << "\n";
  }

  matrix<T> transpose() {
    std::vector<std::vector<T>> outputVec(cols, std::vector<T>(rows));
    for (int row = 0; row < rows; row++) {
      for (int col = 0; col < cols; col++) {
        outputVec[col][row] = data[row][col];
      }
    }
    matrix<T> output = {
      outputVec,
      rows, cols
    };
    return output;
  };

  double norm(double Ln) {
    if (cols > 1) {
      std::cerr << "TODO: Norm with more than one col is not implemented" << std::endl;
    }
    double norm = 0.0;
    for (int row = 0; row < rows; ++row) {
      norm += std::pow(data[row][0], Ln);
    }
    norm = std::pow(norm, 1/Ln);
    return norm;
  };
  
  double max() {
    double max = 0.0;
    for (int row = 0; row < rows; ++row) {
      for (int col = 0; col < cols; ++col) {
        if (data[row][col] > max) {
          max = data[row][col];
        }
      }
    }
    return max;
  }
  matrix<T> pseudoInvert() {
    // Step 1: Compute SVD (this is a simplified version)
    // For a full implementation, you would need to implement or use an SVD algorithm.
    // Here, we will assume you have U, S, and V matrices from SVD.

    // Placeholder for SVD results
    std::vector<std::vector<T>> U(rows, std::vector<T>(rows, 0));
    std::vector<T> S(std::min(rows, cols), 0);
    std::vector<std::vector<T>> V(cols, std::vector<T>(cols, 0));

    // For demonstration, let's assume we have the following SVD results:
    // U = identity matrix, S = [1, 2, 0], V = identity matrix (for a 3x3 matrix)
    for (int i = 0; i < std::min(rows, cols); ++i) {
      U[i][i] = 1; // Identity matrix for U
      S[i] = (i < 2) ? (i + 1) : 0; // Singular values
      V[i][i] = 1; // Identity matrix for V
    }

    // Step 2: Compute the pseudo-inverse of S
    std::vector<std::vector<T>> S_inv(cols, std::vector<T>(rows, 0));
    for (int i = 0; i < S.size(); ++i) {
      if (std::abs(S[i]) > 1e-10) { // Avoid division by zero
        S_inv[i][i] = 1.0 / S[i];
      }
    }

    // Step 3: Compute the pseudo-inverse: A^+ = V * S^+ * U^T
    matrix<T> S_inv_matrix = {
      S_inv,
      cols, rows
    };
    matrix<T> U_transpose = {
      U,
      cols, rows
    };
    matrix<T> V_matrix = {
      V,
      cols, rows
    };

    // Compute V * S_inv
    std::vector<std::vector<T>> VS_inv(V_matrix.rows, std::vector<T>(S_inv_matrix.cols, 0));
    for (int i = 0; i < V_matrix.rows; ++i) {
      for (int j = 0; j < S_inv_matrix.cols; ++j) {
        for (int k = 0; k < V_matrix.cols; ++k) {
          VS_inv[i][j] += V_matrix.data[i][k] * S_inv_matrix.data[k][j];
        }
      }
    }

    // Compute (V * S_inv) * U^T
    std::vector<std::vector<T>> pseudo_inv(VS_inv.size(), std::vector<T>(U_transpose.cols, 0));
    for (int i = 0; i < VS_inv.size(); ++i) {
      for (int j = 0; j < U_transpose.cols; ++j) {
        for (int k = 0; k < U_transpose.rows; ++k) {
          pseudo_inv[i][j] += VS_inv[i][k] * U_transpose.data[k][j];
        }
      }
    }
    matrix<T> pinv = {
      pseudo_inv,
      cols, rows
    };
    return pinv;
  }
  matrix<T> pseudoInvert1() {
    std::vector<std::vector<T>> outputData(rows, std::vector<T>(cols, 0));

    for (int row = 0; row < rows; ++row) {
      for (int col = 0; col < cols; ++col) {
        if (data[row][col] != 0) {
          outputData[row][col] = 1 / data[row][col];
        } else {
          outputData[row][col] = 0;
        }
      }
    }
    matrix<T> output = {
      outputData,
      cols, rows
    };
    return output;
  };
  matrix<T> invert() {
    int n = rows;
    std::vector<std::vector<double>> inverse = std::vector<std::vector<double>>(n, std::vector<double>(n, 0));
    std::vector<std::vector<double>> augmented(n, std::vector<double>(2 * n, 0));

    // Create the augmented matrix [input | I]
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        augmented[i][j] = data[i][j];
      }
      augmented[i][i + n] = 1; // Identity matrix
    }

    // Apply Gaussian elimination
    for (int i = 0; i < n; i++) {
      // Search for maximum in this column
      double maxEl = abs(augmented[i][i]);
      int maxRow = i;
      for (int k = i + 1; k < n; k++) {
        if (abs(augmented[k][i]) > maxEl) {
          maxEl = abs(augmented[k][i]);
          maxRow = k;
        }
      }

      // Swap maximum row with current row
      for (int k = i; k < 2 * n; k++) {
        std::swap(augmented[maxRow][k], augmented[i][k]);
      }

      // Make the diagonal contain all 1s
      double diag = augmented[i][i];
      if (diag == 0) {
        //print();
        //std::cerr << "ERROR: matrix is singular" << std::endl;
      }
      for (int k = 0; k < 2 * n; k++) {
        augmented[i][k] /= diag;
      }

      // Make the other rows contain 0s in this column
      for (int k = 0; k < n; k++) {
        if (k != i) {
          double factor = augmented[k][i];
          for (int j = 0; j < 2 * n; j++) {
            augmented[k][j] -= factor * augmented[i][j];
          }
        }
      }
    }

    // Extract the inverse matrix from the augmented matrix
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        inverse[i][j] = augmented[i][j + n];
      }
    }
    matrix<T> inv = {inverse, cols, rows};
    return inv;
  }
};

template<typename T>
matrix<T> multiply(matrix<T> A, matrix<T> B) {
  if (A.cols != B.rows) {
    std::cerr << "ERROR: cols of A must be equal to rows of B" << std::endl;
    return matrix<T>();
  }

  std::vector<std::vector<T>> outputData(A.rows, std::vector<T>(B.cols, 0));

  for (int row = 0; row < A.rows; ++row) {
    for (int col = 0; col < B.cols; ++col) {
      for (int k = 0; k < A.cols; ++k) {
        outputData[row][col] += A.data[row][k] * B.data[k][col];
      }
    }
  }
  matrix<T> output = {
    outputData,
    B.cols, A.rows
  };

  return output;
}

template<typename T>
matrix<T> subtract(matrix<T> A, matrix<T> B) {
  if (A.cols != B.cols || A.rows != B.rows) { std::cerr << "ERROR: Mismatched matrix sizes" << std::endl; }
  std::vector<std::vector<T>> outputData(A.rows, std::vector<T>(A.cols, 0));
  for (int row = 0; row < A.rows; ++row) {
    for (int col = 0; col < A.cols; ++col) {
      outputData[row][col] = A.data[row][col] - B.data[row][col];
    }
  }
  matrix<T> output = {
    outputData,
    A.cols, A.rows
  };
  return output;
}

template<typename T>
matrix<T> add(matrix<T> A, matrix<T> B) {
  if (A.cols != B.cols || A.rows != B.rows) { std::cerr << "ERROR: Mismatched matrix sizes" << std::endl; }
  std::vector<std::vector<T>> outputData(A.rows, std::vector<T>(A.cols, 0));
  for (int row = 0; row < A.rows; ++row) {
    for (int col = 0; col < A.cols; ++col) {
      outputData[row][col] = A.data[row][col] + B.data[row][col];
    }
  }
  matrix<T> output = {
    outputData,
    A.cols, A.rows
  };
  return output;
}


struct equation {
  std::vector<std::pair<double, symbol>> equation;

  double solve(symbol sym, const std::vector<std::pair<symbol, double>>& values);
};
