#pragma once
#include "matrix.h"
#include <vector>
#include <iostream>
#include <ostream>
#include <memory>
#include <numbers>
#include <math.h>
#include <functional>
#include <algorithm>

struct symbol {
  std::string name;
  symbol(const std::string& n);
  symbol() : name("NULL") {};
  bool operator==(const symbol& other) const {
    return other.name == name;
  }
  bool operator<(const symbol& other) const {
    return other.name < name;
  }
  bool operator>(const symbol& other) const {
    return other.name > name;
  }
  
};

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
    //std::cout << "added branch" << std::endl;
    isBranch = true;
    brachOperation = b;
    functions = std::make_pair(f1, f2);
  };
  symbol functionSymbol;
  
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


// f(x1, x2, x3, ...)
class multiVaribleFunction {
public:
  function nodeData;
  bool isBranch = false;
  branchPtr brachOperation = nullptr;
  std::pair<std::shared_ptr<multiVaribleFunction>, std::shared_ptr<multiVaribleFunction>> branchEquations;

  double evaluate(std::vector<std::pair<symbol, double>> inputs) {
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

  double differentiate(symbol varible, std::vector<std::pair<symbol, double>> inputs, double h) {
    double fx = this->evaluate(inputs);
    int varIdx = 0;
    auto inputs_he = inputs;
    for (int i = 0; i < inputs.size(); i++) {
      if (inputs[i].first.name == varible.name) {
        inputs_he[i].second += h;
      }
    }
    double fx_he = this->evaluate(inputs_he);
    return (fx_he-fx)/h;
  }

  bool variblesAreKnown = false;
  std::vector<symbol> varibles;
  std::vector<symbol> getVaribles() {
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
};

function createConstantFunction(double val);


namespace Operation {
  operationPtr constant(double arg);
  
  operationPtr add(double arg);

  operationPtr multiply(double arg);
  
  operationPtr divide(double arg);

  operationPtr sin(double A = 1.0, double frequency = 1/(2*M_PI), double theta = 0);
  
  operationPtr cos(double A = 1.0, double frequency = 1/(2*M_PI), double theta = 0);
  
  operationPtr scaleToOne();
  // TODO: add more functions
  
};
