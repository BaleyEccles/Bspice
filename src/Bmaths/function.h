#pragma once
#include <vector>
#include <iostream>
#include <ostream>
#include <memory>
#include <numbers>
#include <cmath>
#include <functional>

// for now this is intended to be used as a function that takes in one double and returns a double
// for example Vcc = A*sin(w*t + theta)
typedef std::function<double(double)> operationPtr;
typedef std::function<double(double, double)> branchPtr;

// The way we are going to reprsent a function is like a tree.
// To deal with having more than one instance of a varible we must have a recursive definition
// Using a simple list of opperations would make it 'imposible' to rempresent functions like:
// y = x + sin(x)

class function {
 public:
  inline void addOpperation(operationPtr f) {opperations.push_back(f); };
  inline void addBranches(branchPtr b, std::shared_ptr<function> f1, std::shared_ptr<function> f2) {
    isBranch = true;
    brachOpperation = b;
    functions = std::pair<std::shared_ptr<function>, std::shared_ptr<function>>(f1, f2);
  };
  bool isBranch = false;

  std::vector<operationPtr> opperations;
  std::pair<std::shared_ptr<function>, std::shared_ptr<function>> functions;
  branchPtr brachOpperation = nullptr;
  double evaluate(double t);
};


namespace Opperation {
  operationPtr add(double arg);

  operationPtr multiply(double arg);
  
  operationPtr divide(double arg);

  operationPtr sin(double A = 1.0, double frequency = 1/(2*std::numbers::pi), double theta = 0);
  
  operationPtr cos(double A = 1.0, double frequency = 1/(2*std::numbers::pi), double theta = 0);
  // TODO: add more functions
  
};
namespace branchOpperation {
  double add(double branch1, double branch2);
  
  double multiply(double branch1, double branch2);
  
  double divide(double branch1, double branch2);
  // TODO: add more functions
};



