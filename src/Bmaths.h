#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <cmath>


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
    return matrix<T>{output, 3, 1};
  }
  matrix<T> getRow(int row) {
    std::vector<std::vector<T>> output;
    for (int i = 0; i < cols; i++) {
      output.push_back({data[row][i]});
    }
    return matrix<T>{output, 3, 1};
  }

  void print() {
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        printf("%.5e ", data[i][j]);
      }
      printf("\n");
    }
    printf("\n");
  }

  matrix<T> psudoInvert() {
    std::vector<std::vector<T>> outputData(rows, std::vector<T>(cols, 0));
    for (int i = 0; i < rows; ++i) {
      for (int j = 0; j < cols; ++j) {
        if (data[j][i] == 0) {
          outputData[j][i] = 0.0;
        } else { 
        outputData[j][i] = 1/data[j][i];
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
        print();
        std::cerr << "ERROR: Matrix is singular" << std::endl;
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
  int An = A.cols;
  int Bn = A.rows;
  if (An != Bn) { std::cerr << "ERROR: cols of A must be rows of B" << std::endl; }
  std::vector<std::vector<T>> outputData(A.rows, std::vector<T>(B.cols, 0));
  for (int i = 0; i < A.rows; ++i) {
    for (int j = 0; j < B.cols; ++j) {
      for (int k = 0; k < A.cols; ++k) {
        outputData[i][j] += A.data[i][k] * B.data[k][j];
      }
    }
  }
 
  matrix<T> output = {
    outputData,
    B.rows, A.cols
  };
  return output;
}

template<typename T>
matrix<T> subtract(matrix<T> A, matrix<T> B) {
  if (A.cols != B.cols || A.rows != B.rows) { std::cerr << "ERROR: Mismatched matrix sizes" << std::endl; }
  std::vector<std::vector<T>> outputData(A.rows, std::vector<T>(A.cols, 0));
  for (int i = 0; i < A.rows; ++i) {
    for (int j = 0; j < A.cols; ++j) {
      outputData[j][i] = A.data[j][i] - B.data[j][i];
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
  for (int i = 0; i < A.rows; ++i) {
    for (int j = 0; j < A.cols; ++j) {
      outputData[j][i] = A.data[j][i] + B.data[j][i];
    }
  }
  matrix<T> output = {
    outputData,
    A.cols, A.rows
  };
  return output;
}


struct symbol {
  std::string name;
  symbol(const std::string& n);
};

struct equation {
  std::vector<std::pair<double, symbol>> equation;

  double solve(symbol sym, const std::vector<std::pair<symbol, double>>& values);
};
