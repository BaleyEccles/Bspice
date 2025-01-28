#include "function.h"

double function::evaluate(double t) {
  if (!isBranch) {
    for (auto& opperation : opperations) {
      t = opperation(t);
    }
  } else {
    if (brachOpperation == nullptr) {
      std::cerr << "ERROR: Branch has no opperation defined." << std::endl;
    }
    t = brachOpperation(functions.first->evaluate(t), functions.second->evaluate(t));
  }
  return t;
};



namespace Opperation {
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
    return [A, frequency, theta](double t) { return A*std::sin(frequency*2*std::numbers::pi + theta); };
  }
  
  operationPtr cos(double A, double frequency, double theta) {
    return [A, frequency, theta](double t) { return A*std::cos(frequency*2*std::numbers::pi + theta); };
  }
  // TODO: add more functions
  
};
namespace branchOpperation {
  branchPtr add(double branch1, double branch2) {
    return [](double branch1, double branch2) { return branch1 + branch2; };
  }
  
  branchPtr multiply(double branch1, double branch2) {
    return [](double branch1, double branch2) { return branch1 * branch2; };
  }
  
  branchPtr divide(double branch1, double branch2) {
    return [](double branch1, double branch2) { return branch1 / branch2; };
  }
  // TODO: add more functions
};


