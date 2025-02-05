#pragma once
#include <vector>
#include <iostream>
#include <ostream>
#include <memory>
#include <numbers>
#include <cmath>
#include <functional>

namespace branchOperation {
  double add(double branch1, double branch2);
  
  double multiply(double branch1, double branch2);
  
  double divide(double branch1, double branch2);
  // TODO: add more functions
};

// for now this is intended to be used as a function that takes in one double and returns a double
// for example Vcc = A*sin(w*t + theta)
typedef std::function<double(double)> operationPtr;
typedef std::function<double(double, double)> branchPtr;

// The way we are going to reprsent a function is like a tree.
// To deal with having more than one instance of a varible we must have a recursive definition
// Using a simple list of operations would make it 'imposible' to rempresent functions like:
// y = x + sin(x)

class function {
public:
  inline void addOperation(operationPtr f) {operations.push_back(f); };
  inline void addBranches(branchPtr b, std::shared_ptr<function> f1, std::shared_ptr<function> f2) {
    if (f1 == nullptr) {
      std::cerr << "ERROR: Unable to add branches, f1 is nullptr." << std::endl;
    }
    if (f2 == nullptr) {
      std::cerr << "ERROR: Unable to add branches, f2 is nullptr." << std::endl;
    }
    std::cout << "added branch" << std::endl;
    isBranch = true;
    brachOperation = b;
    functions = std::make_pair(f1, f2);
  };
  bool isBranch = false;

  std::vector<operationPtr> operations;
  std::pair<std::shared_ptr<function>, std::shared_ptr<function>> functions;
  branchPtr brachOperation = nullptr;
  double evaluate(double t) const;
  double evaluateBranches(double t) const;
  
  function operator+(const function& other) {
    std::shared_ptr<function> f1 = std::make_shared<function>(*this);
    std::shared_ptr<function> f2 = std::make_shared<function>(other);
    function f;
    f.addBranches(branchOperation::add, f1, f2);
    return f;
  }

};

function createConstantFunction(double val);


namespace Operation {
  operationPtr add(double arg);

  operationPtr multiply(double arg);
  
  operationPtr divide(double arg);

  operationPtr sin(double A = 1.0, double frequency = 1/(2*std::numbers::pi), double theta = 0);
  
  operationPtr cos(double A = 1.0, double frequency = 1/(2*std::numbers::pi), double theta = 0);
  
  operationPtr scaleToOne();
  // TODO: add more functions
  
};

