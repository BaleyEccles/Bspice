#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include "matrix.h"
#include "DAESolve.h"
#include "AlgebraicEquationSolver.h"

struct symbol {
  std::string name;
  symbol(const std::string& n);
  symbol() : name("NULL") {}
  bool operator==(const symbol& other) const {
    return other.name == name;
  }
};


struct equation {
  std::vector<std::pair<double, symbol>> equation;

  double solve(symbol sym, const std::vector<std::pair<symbol, double>>& values);
};
