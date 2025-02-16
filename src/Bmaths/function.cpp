#include "function.h"

symbol::symbol(const std::string& name) : name(name) {}

double function::evaluate(double t) const {

  if (brachOperation == nullptr && isBranch) {
    std::cerr << "ERROR: Branch has no operation defined." << std::endl;
  }
  if (brachOperation != nullptr && isBranch) {
    t = brachOperation(functions.first->evaluate(t), functions.second->evaluate(t));
  }
  for (auto& operation : operations) {
    t = operation(t);
  }
  return t;
};

function createConstantFunction(double val) {
  function f;
  f.addOperation(Operation::constant(val));
  return f;
}



namespace Operation {
  operationPtr constant(double arg) {
    return [arg](double t) { return arg; };
  }
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
    return [A, frequency, theta](double t) { return A*std::sin(frequency*2*M_PI*t + theta); };
  }
  
  operationPtr cos(double A, double frequency, double theta) {
    return [A, frequency, theta](double t) { return A*std::cos(frequency*2*M_PI + theta); };
  }

  operationPtr scaleToOne() {
    return [](double t) {
      if (t > 0) return 1;
      if (t < 0) return -1;
      return 0;
    };
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


