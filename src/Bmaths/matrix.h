#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include "complexNumbers.h"

class function;
class symbol;

template <typename T>
class matrix {
 public:
  std::vector<std::vector<T>> data;
  int cols, rows;
  void print(const std::string name = "") const;
  matrix<T> scale(double scale);

  matrix<T> getColumn(int col);
  matrix<T> getRow(int row);
  matrix<T> eliminateRow(int row);
  matrix<T> eliminateCol(int col);
  matrix<double> evaluate(double t);
  matrix<double> evaluate(std::vector<std::pair<symbol, double>> inputs);
  matrix<T> transpose();
  double norm(double Ln);
  double max();
  
  matrix<T> pseudoInvert();
  matrix<T> invert();
  template<typename U>
  matrix<T> operator*(const matrix<U>& other);
  matrix<T> operator+(const matrix<T>& other);
  matrix<T> operator-(const matrix<T>& other);
};

//template<typename T>
 //matrix<T> multiply(matrix<T> A, matrix<T> B);

//template<typename T>
 //matrix<T> subtract(matrix<T> A, matrix<T> B);

//template <typename T> matrix<T> add(matrix<T> A, matrix<T> B);
#include "matrix_impl.h"
