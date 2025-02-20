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

function createConstantFunction(double val, symbol sym) {
  function f;
  f.functionSymbol = sym;
  f.addOperation(Operation::constant(val));
  return f;
}


double multiVaribleFunction::evaluate(std::vector<std::pair<symbol, double>> inputs) {
  double output = 0.0;
  if (brachOperation == nullptr && isBranch) {
    std::cerr << "ERROR: Branch has no operation defined." << std::endl;
  }
  if (brachOperation != nullptr && isBranch) {
    double b1 = branchEquations.first->evaluate(inputs);
    double b2 = branchEquations.second->evaluate(inputs);
    double output = brachOperation(b1, b2);
  } else {
    for (auto& input : inputs) {
      if (input.first.name == nodeData.functionSymbol.name) {
        output = nodeData.evaluate(input.second);
      }
    }
  }
  return output;
};

double multiVaribleFunction::differentiate(symbol varible, std::vector<std::pair<symbol, double>> inputs, double h) {
  double fx = this->evaluate(inputs);
  int varIdx = 0;
  auto heInputs = inputs;
  for (int i = 0; i < inputs.size(); i++) {
    if (inputs[i].first.name == varible.name) {
      heInputs[i].second += h;
    }
  }
  double fxhe = this->evaluate(heInputs);
  return (fxhe-fx)/h;
}

std::vector<symbol> multiVaribleFunction::getVaribles() {
  if (variblesAreKnown) {
    return varibles;
  }
  if (brachOperation == nullptr && isBranch) {
    std::cerr << "ERROR: Branch has no operation defined." << std::endl;
  }
  if (isBranch) {
    auto b1 = branchEquations.first->getVaribles();
    auto b2 = branchEquations.second->getVaribles();
    varibles.insert(varibles.end(), b1.begin(), b1.end());
    varibles.insert(varibles.end(), b2.begin(), b2.end());
    std::sort(varibles.begin(), varibles.end());
    auto last = std::unique(varibles.begin(), varibles.end());
    varibles.erase(last, varibles.end());
  } else {
    varibles.push_back(nodeData.functionSymbol);
  }
  variblesAreKnown = true;
  std::sort(varibles.begin(), varibles.end());
  return varibles;
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

  operationPtr exp(double arg1, double arg2) {
    return [arg1, arg2](double t) { return pow(arg1, arg2*t); };
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


