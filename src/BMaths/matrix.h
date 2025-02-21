#pragma once
#include <array>
#include <string>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include "complexNumbers.h"
#include "function.h"
#include "matrix.h"

class function;
class symbol;

template <typename T, std::size_t colsT, std::size_t rowsT>
class matrix {
 public:
  matrix();
  std::array<std::array<T, colsT>, rowsT> data;
  std::size_t cols, rows;
  void createData();  
  void print(const std::string name = "") const;
  
  matrix<T, colsT, rowsT> scale(double scale);
  matrix<T, colsT, rowsT> getColumn(int col);
  matrix<T, colsT, rowsT> getRow(int row);
  matrix<T, colsT, rowsT> eliminateRow(int row);
  matrix<T, colsT, rowsT> eliminateCol(int col);
  matrix<double, colsT, rowsT> evaluate(double t);
  matrix<double, colsT, rowsT> evaluate(std::vector<std::pair<symbol, double>> inputs);
  matrix<T, colsT, rowsT> transpose();
  double norm(double Ln);
  double max();
  matrix<T, colsT, rowsT> invert();
  
  template<typename U>
  matrix<T, colsT, rowsT> operator*(const matrix<U, rowsT, colsT>& other);
  matrix<T, colsT, rowsT> operator+(const matrix<T, colsT, rowsT>& other);
  matrix<T, colsT, rowsT> operator-(const matrix<T, colsT, rowsT>& other);
};

template <typename T, std::size_t colsT, std::size_t rowsT>
matrix<T, colsT, rowsT>::matrix()
  : cols(colsT), rows(rowsT) {
  createData();
}

//template <typename T, std::size_t colsT, std::size_t rowsT> TODO
//void matrix<T, colsT, rowsT>::createData() {
//  if constexpr (std::is_arithmetic<T>::value) {
//    data = std::vector<std::vector<T>>(rows, std::vector<T>(cols, 0));
//    
//  } else if constexpr (std::is_same<T, symbol>::value) {
//    data = std::vector<std::vector<T>>(rows, std::vector<T>(cols, symbol("")));
//    
//  } else if constexpr (std::is_same<T, function>::value) {
//    function f0 = createConstantFunction(0.0);
//    data = std::vector<std::vector<T>>(rows, std::vector<T>(cols, f0));
//    
//  } else if constexpr (std::is_same<T, complexNumber<double>>::value) {
//    data = std::vector<std::vector<T>>(rows, std::vector<T>(cols, complexNumber<double>(0.0, 0.0)));
//  }
//}

template <typename T, std::size_t colsT, std::size_t rowsT>
void matrix<T, colsT, rowsT>::print(const std::string name) const {
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

template <typename T, std::size_t colsT, std::size_t rowsT>
matrix<T, colsT, rowsT> matrix<T, colsT, rowsT>::scale(double scale) {
  matrix<T, colsT, rowsT> output;
  for (int row = 0; row < rows; ++row) {
    for (int col = 0; col < cols; ++col) {
      output.data[row][col] = scale*data[row][col];
    }
  }

  return output;
}

template <typename T, std::size_t colsT, std::size_t rowsT>
matrix<T, colsT, rowsT> matrix<T, colsT, rowsT>::getColumn(int col) {
  std::vector<std::vector<T>> output;
  output.reserve(rows);
  for (int i = 0; i < rows; i++) {
    output.push_back({data[i][col]});
  }
  return matrix<T, colsT, rowsT>{output, 1, rows};
}

template <typename T, std::size_t colsT, std::size_t rowsT>
matrix<T, colsT, rowsT> matrix<T, colsT, rowsT>::getRow(int row) {
  matrix<T, colsT, rowsT> output;
  
  for (int i = 0; i < cols; i++) {
    output.data[i][0] = (data[row][i]);
  }

  return output;
}

template <typename T, std::size_t colsT, std::size_t rowsT>
matrix<T, colsT, rowsT> matrix<T, colsT, rowsT>::eliminateRow(int row) {
  //data.erase(data.begin() + row); TODO
  rows -= 1;
  return *this;
}
template <typename T, std::size_t colsT, std::size_t rowsT>
matrix<T, colsT, rowsT> matrix<T, colsT, rowsT>::eliminateCol(int col) {
  for (int row = 0; row < rows; row++) {
    //data[row].erase(data[row].begin() + col); TODO
  }
  cols -= 1;
  return *this;
}

// For multivarible functions
template <typename T, std::size_t colsT, std::size_t rowsT>
matrix<double, colsT, rowsT> matrix<T, colsT, rowsT>::evaluate(std::vector<std::pair<symbol, double>> inputs) {
  matrix<double, colsT, rowsT> output;
  for (int row = 0; row < output.rows; row++) {
    for (int col = 0; col < output.cols; col++) {
      output.data[row][col] = data[row][col].evaluate(inputs);
    }
  }
  return output;
  std::cerr << "ERROR: Unreachable" << std::endl;
}

template <typename T, std::size_t colsT, std::size_t rowsT>
matrix<double, colsT, rowsT> matrix<T, colsT, rowsT>::evaluate(double t) {
  if constexpr (std::is_arithmetic<T>::value) {
    return *this;
  } else if constexpr (std::is_same<T, function>::value) {
    matrix<double, colsT, rowsT> output;
    for (int row = 0; row < output.rows; row++) {
      for (int col = 0; col < output.cols; col++) {
        output.data[row][col] = data[row][col].evaluate(t);
      }
    }
    return output;
  }
  std::cerr << "ERROR: Unreachable" << std::endl;
}


template <typename T, std::size_t colsT, std::size_t rowsT>
matrix<T, colsT, rowsT> matrix<T, colsT, rowsT>::transpose() {
  matrix<T, colsT, rowsT> output;
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      output.data[col][row] = data[row][col];
    }
  }

  return output;
};

template <typename T, std::size_t colsT, std::size_t rowsT>
double matrix<T, colsT, rowsT>::norm(double Ln) {
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

template <typename T, std::size_t colsT, std::size_t rowsT>
double matrix<T, colsT, rowsT>::max() {
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

template <typename T, std::size_t colsT, std::size_t rowsT>
matrix<T, colsT, rowsT> matrix<T, colsT, rowsT>::invert() {
  matrix<double, colsT, rowsT> inverse;
  matrix<double, colsT, rowsT> augmented;

    // Create the augmented matrix [input | I]
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < rows; j++) {
        augmented.data[i][j] = data[i][j];
      }
      augmented.data[i][i + rows] = 1; // Identity matrix
    }

    // Apply Gaussian elimination
    for (int i = 0; i < rows; i++) {
      // Search for maximum in this column
      double maxEl = abs(augmented.data[i][i]);
      int maxRow = i;
      for (int k = i + 1; k < rows; k++) {
        if (abs(augmented.data[k][i]) > maxEl) {
          maxEl = abs(augmented.data[k][i]);
          maxRow = k;
        }
      }

      // Swap maximum row with current row
      for (int k = i; k < 2 * rows; k++) {
        std::swap(augmented.data[maxRow][k], augmented.data[i][k]);
      }

      // Make the diagonal contain all 1s
      double diag = augmented.data[i][i];
      if (diag == 0) {
        print();
        std::cerr << "ERROR: matrix is singular" << std::endl;
      }
      for (int k = 0; k < 2 * rows; k++) {
        augmented.data[i][k] /= diag;
      }

      // Make the other rows contain 0s in this column
      for (int k = 0; k < rows; k++) {
        if (k != i) {
          double factor = augmented.data[k][i];
          for (int j = 0; j < 2 * rows; j++) {
            augmented.data[k][j] -= factor * augmented.data[i][j];
          }
        }
      }
    }

    // Extract the inverse matrix from the augmented.data matrix
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < rows; j++) {
        inverse.data[i][j] = augmented.data[i][j + rows];
      }
    }
    return inverse;
  }


template <typename T, std::size_t colsT, std::size_t rowsT>
template <typename U>
matrix<T, colsT, rowsT> matrix<T, colsT, rowsT>::operator*(const matrix<U, rowsT, colsT>& other) {
  if (this->cols != other.rows) {
    std::cerr << "ERROR: For multiply cols of A must be equal to rows of B" << std::endl;
    this->print("A:");
    other.print("B:");
  }
  matrix<T, colsT, rowsT> output;//(this->rows, std::vector<T>(other.cols));


  for (int row = 0; row < this->rows; ++row) {
    for (int col = 0; col < other.cols; ++col) {
      for (int k = 0; k < this->cols; ++k) {
        output.data[row][col] = output.data[row][col] + (this->data[row][k] * other.data[k][col]);
      }
    }
  }

  return output;
}


template <typename T, std::size_t colsT, std::size_t rowsT>
matrix<T, colsT, rowsT> matrix<T, colsT, rowsT>::operator-(const matrix<T, colsT, rowsT>& other) {
  if (this->cols != other.cols || this->rows != other.rows) {
    std::cerr << "ERROR: Mismatched matrix sizes" << std::endl;
    this->print("A");
    other.print("B");
  }
  matrix<T, colsT, rowsT> output;
  for (int row = 0; row < this->rows; ++row) {
    for (int col = 0; col < this->cols; ++col) {
      output.data[row][col] = this->data[row][col] - other.data[row][col];
    }
  }

  return output;
}



template <typename T, std::size_t colsT, std::size_t rowsT>
matrix<T, colsT, rowsT> matrix<T, colsT, rowsT>::operator+(const matrix<T, colsT, rowsT>& other) {
  if (this->cols != other.cols || this->rows != other.rows) { std::cerr << "ERROR: Mismatched matrix sizes" << std::endl; }
  matrix<T, colsT, rowsT> output;
  for (int row = 0; row < this->rows; ++row) {
    for (int col = 0; col < this->cols; ++col) {
      output.data[row][col] = this->data[row][col] + other.data[row][col];
    }
  }
  return output;
}

