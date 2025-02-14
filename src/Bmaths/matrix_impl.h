#pragma once
#include "function.h"
#include "matrix.h"
template <typename T>
void matrix<T>::print(const std::string name) const {
  if (name != "") std::cout << name << std::endl;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      if constexpr (std::is_arithmetic<T>::value) {
        std::cout << std::scientific << std::setprecision(5) << data[i][j] << " ";
      } else if constexpr (std::is_same<T, symbol>::value) {
        std::cout << data[i][j].name << " ";
      } else if constexpr (std::is_same<T, function>::value) {
        std::cout << "function evaluated at 0: " << data[i][j].evaluate(0.0) << " ";
      } else if constexpr (std::is_same<T, complexNumber<double>>::value) {
        data[i][j].print();
      }
    }
    std::cout << "\n";
  }
  std::cout << "\n";
}

template <typename T>
matrix<T> matrix<T>::scale(double scale) {
  std::vector<std::vector<T>> outputData(rows, std::vector<T>(cols, 0));
  for (int row = 0; row < rows; ++row) {
    for (int col = 0; col < cols; ++col) {
      outputData[row][col] = scale*data[row][col];
    }
  }
  matrix<T> output = {
    outputData,
    cols, rows
  };
  return output;
}

template <typename T>
matrix<T> matrix<T>::getColumn(int col) {
  std::vector<std::vector<T>> output;
  for (int i = 0; i < rows; i++) {
    output.push_back({data[i][col]});
  }
  return matrix<T>{output, 1, rows};
}

template <typename T>
matrix<T> matrix<T>::getRow(int row) {
  std::vector<std::vector<T>> output;
  for (int i = 0; i < cols; i++) {
    output.push_back({data[row][i]});
  }
  return matrix<T>{output, 1, cols};
}

template <typename T>
matrix<T> matrix<T>::eliminateRow(int row) {
  data.erase(data.begin() + row);
  rows -= 1;
  return matrix<T> {
    data,
    cols, rows
  };
}
template <typename T>
matrix<T> matrix<T>::eliminateCol(int col) {
  for (int row = 0; row < rows; row++) {
    data[row].erase(data[row].begin() + col);
  }
  cols -= 1;
  return matrix<T> {
    data,
    cols, rows
  };
}

// For multivarible functions
template<typename T>
matrix<double> matrix<T>::evaluate(std::vector<std::pair<symbol, double>> inputs) {
  matrix<double> output = {std::vector<std::vector<double>>(rows, std::vector<double>(cols, 0.0)), cols, rows};
  for (int row = 0; row < output.rows; row++) {
    for (int col = 0; col < output.cols; col++) {
      output.data[row][col] = data[row][col].evaluate(inputs);
    }
  }
  return output;
  std::cerr << "ERROR: Unreachable" << std::endl;
}

template<typename T>
matrix<double> matrix<T>::evaluate(double t) {
  if constexpr (std::is_arithmetic<T>::value) {
    return &this;
  } else if constexpr (std::is_same<T, function>::value) {
    matrix<double> output = {std::vector<std::vector<double>>(rows, std::vector<double>(cols, 0.0)), cols, rows};
    for (int row = 0; row < output.rows; row++) {
      for (int col = 0; col < output.cols; col++) {
        output.data[row][col] = data[row][col].evaluate(t);
      }
    }
    return output;
  }
  std::cerr << "ERROR: Unreachable" << std::endl;
}


template <typename T>
matrix<T> matrix<T>::transpose() {
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

template <typename T>
double matrix<T>::norm(double Ln) {
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
template <typename T>
double matrix<T>::max() {
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
template <typename T>
matrix<T> matrix<T>::pseudoInvert() { //FIXME: This is wrong but works for now
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
template <typename T>
matrix<T> matrix<T>::invert() {
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
        std::cerr << "ERROR: matrix is singular" << std::endl;
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


template<typename T>
template<typename U>
matrix<T> matrix<T>::operator*(const matrix<U>& other) {
  if (this->cols != other.rows) {
    std::cerr << "ERROR: For multiply cols of A must be equal to rows of B" << std::endl;
    this->print("A:");
    other.print("B:");
  }
  std::vector<std::vector<T>> outputData(this->rows, std::vector<T>(other.cols));


  for (int row = 0; row < this->rows; ++row) {
    for (int col = 0; col < other.cols; ++col) {
      for (int k = 0; k < this->cols; ++k) {
        outputData[row][col] = outputData[row][col] + (this->data[row][k] * other.data[k][col]);
      }
    }
  }
  matrix<T> output = {
    outputData,
    other.cols, this->rows
  };

  return output;
}


template<typename T>
matrix<T> matrix<T>::operator-(const matrix<T>& other) {
  if (this->cols != other.cols || this->rows != other.rows) {
    std::cerr << "ERROR: Mismatched matrix sizes" << std::endl;
    this->print("A");
    other.print("B");
  }
  std::vector<std::vector<T>> outputData(this->rows, std::vector<T>(this->cols, 0));
  for (int row = 0; row < this->rows; ++row) {
    for (int col = 0; col < this->cols; ++col) {
      outputData[row][col] = this->data[row][col] - other.data[row][col];
    }
  }
  matrix<T> output = {
    outputData,
    this->cols, this->rows
  };
  return output;
}



template<typename T>
matrix<T> matrix<T>::operator+(const matrix<T>& other) {
  if (this->cols != other.cols || this->rows != other.rows) { std::cerr << "ERROR: Mismatched matrix sizes" << std::endl; }
  std::vector<std::vector<T>> outputData(this->rows, std::vector<T>(this->cols, 0));
  for (int row = 0; row < this->rows; ++row) {
    for (int col = 0; col < this->cols; ++col) {
      outputData[row][col] = this->data[row][col] + other.data[row][col];
    }
  }
  matrix<T> output = {
    outputData,
    this->cols, this->rows
  };
  return output;
}
