#include "function.h"

double function::evaluate(double t) const {

  if (!isBranch) {
    for (auto& operation : operations) {
      t = operation(t);
    }
  } else {
    if (brachOperation == nullptr) {
      std::cerr << "ERROR: Branch has no operation defined." << std::endl;
    }
    t = brachOperation(functions.first->evaluate(t), functions.second->evaluate(t));
    t = this->evaluateBranches(t);
  }
  return t;
};

double function::evaluateBranches(double t) const {
  for (auto& operation : operations) {
    t = operation(t);
  }
  return t;
}

function createConstantFunction(double val) {
  function f;
  f.addOperation(Operation::multiply(0.0));
  f.addOperation(Operation::add(val));
  return f;
}



namespace Operation {
  operationPtr add(double arg) {
    return [arg](double t) { return arg + t; };
  }

  operationPtr multiply(double arg) {
    return [arg](double t) { return arg * t; };
  }
  
  operationPtr divide(double arg) {
    return [arg](double t) { return t / arg; };
  }

  operationPtr sin(double A, double frequency, double theta) {
    return [A, frequency, theta](double t) { return A*std::sin(frequency*2*std::numbers::pi*t + theta); };
  }
  
  operationPtr cos(double A, double frequency, double theta) {
    return [A, frequency, theta](double t) { return A*std::cos(frequency*2*std::numbers::pi + theta); };
  }
  // TODO: add more functions
  
};
namespace branchOperation {
  double add(double branch1, double branch2) {
    return branch1 + branch2;
  }
  
  double multiply(double branch1, double branch2) {
    return branch1 * branch2;
  }
  
  double divide(double branch1, double branch2) {
    return branch1 / branch2;
  }
  // TODO: add more functions
};


