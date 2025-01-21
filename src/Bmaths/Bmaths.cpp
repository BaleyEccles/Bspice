#include "Bmaths.h"
// Of the form 0 = a*x1 + b*x2 + c*x3 + ...
double equation::solve(symbol sym, const std::vector<std::pair<symbol, double>>& values) {
  double solution = 0.0;

  for (const auto& equ : equation) {
    auto it = std::find_if(values.begin(), values.end(), [&equ](const std::pair<symbol, double>& p) {
      return p.first.name == equ.second.name;
    });

    if (it != values.end()) {
      double subValue = it->second;
      solution -= equ.first * subValue;
    }
  }

  auto it = std::find_if(equation.begin(), equation.end(), [&sym](const std::pair<double, symbol>& p) {
    return p.second.name == sym.name;
  });

  if (it != equation.end()) {
    double divValue = it->first;
    solution = solution / divValue;
  }

  return solution;
}


symbol::symbol(const std::string& name) : name(name) {}



