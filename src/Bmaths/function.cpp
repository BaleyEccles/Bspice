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
    isBranch = false;
    t = this->evaluate(t);
    isBranch = true;
  }
  return t;
};

function createConstantFunction(double val) {
  function f;
  f.addOpperation(Opperation::multiply(0.0));
  f.addOpperation(Opperation::add(val));
  return f;
}



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
    return [A, frequency, theta](double t) { return A*std::sin(frequency*2*std::numbers::pi*t + theta); };
  }
  
  operationPtr cos(double A, double frequency, double theta) {
    return [A, frequency, theta](double t) { return A*std::cos(frequency*2*std::numbers::pi + theta); };
  }
  // TODO: add more functions
  
};
namespace branchOpperation {
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


