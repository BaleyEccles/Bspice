#pragma once
#include <vector>
#include <iostream>
#include <ostream>
#include <memory>
#include <numbers>
#include <math.h>
#include <functional>
#include <algorithm>
#include <map>

struct symbol {

  symbol(const std::string& n);
  symbol() : name("NULL") {};
  std::string name;
  std::shared_ptr<symbol> parentVarible = nullptr; // used if this varible is a derivative of parent varible
  
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

  double subtract(double branch1, double branch2);
  
  double multiply(double branch1, double branch2);
  
  double divide(double branch1, double branch2);
  
  // TODO: add more functions
};

// for now this is intended to be used as a function that takes in one double and returns a double
// for example Vcc = A*sin(w*t + theta)
typedef std::function<double(double)> operationPtr;
typedef std::function<double(double, double)> branchPtr;

class function;
function createConstantFunction(double val, symbol sym = symbol(""));

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

  function operator=(const double& other) {
    return createConstantFunction(other);
  }
  
  function operator+(const function& other) {
    std::shared_ptr<function> f1 = std::make_shared<function>(*this);
    std::shared_ptr<function> f2 = std::make_shared<function>(other);
    function f;
    f.addBranches(branchOperation::add, f1, f2);
    return f;
  }

  function operator-(const function& other) {
    std::shared_ptr<function> f1 = std::make_shared<function>(*this);
    std::shared_ptr<function> f2 = std::make_shared<function>(other);
    function f;
    f.addBranches(branchOperation::subtract, f1, f2);
    return f;
  }

  function operator+=(const function& other) {
    std::shared_ptr<function> f1 = std::make_shared<function>(*this);
    std::shared_ptr<function> f2 = std::make_shared<function>(other);
    this->addBranches(branchOperation::add, f1, f2);
    return *this;
  }
  
  function operator-=(const function& other) {
    std::shared_ptr<function> f1 = std::make_shared<function>(*this);
    std::shared_ptr<function> f2 = std::make_shared<function>(other);
    this->addBranches(branchOperation::subtract, f1, f2);
    return *this;
  }

};



typedef std::map<symbol, double> values;


// f(x1, x2, x3, ...)
class multiVaribleFunction {
public:
  function nodeData;
  bool isBranch = false;
  branchPtr brachOperation = nullptr;
  std::pair<std::shared_ptr<multiVaribleFunction>, std::shared_ptr<multiVaribleFunction>> branchEquations;
  
  double evaluate(values inputs);

  double differentiate(symbol varible, values inputs, double h);

  bool variblesAreKnown = false;
  std::vector<symbol> varibles;
  std::vector<symbol> getVaribles();

  multiVaribleFunction operator=(const double& other) {
    multiVaribleFunction parent;
    parent.nodeData = createConstantFunction(other);
    return parent;
  };
  
  multiVaribleFunction operator+(const function& other) {
    std::shared_ptr<multiVaribleFunction> fMultiVarible = std::make_shared<multiVaribleFunction>(*this);
    multiVaribleFunction parent;
    parent.branchEquations = std::make_pair(fMultiVarible, nullptr);
    parent.isBranch = true;
    parent.brachOperation = branchOperation::add;
    parent.nodeData = other;
    return parent;
  };

  multiVaribleFunction operator-(const function& other) {
    std::shared_ptr<multiVaribleFunction> fMultiVarible = std::make_shared<multiVaribleFunction>(*this);
    multiVaribleFunction parent;
    parent.branchEquations = std::make_pair(fMultiVarible, nullptr);
    parent.isBranch = true;
    parent.brachOperation = branchOperation::subtract;
    parent.nodeData = other;
    return parent;
  }
  
  multiVaribleFunction operator+=(const function& other) {
    std::shared_ptr<multiVaribleFunction> fMultiVarible = std::make_shared<multiVaribleFunction>(*this);
    multiVaribleFunction parent;
    parent.branchEquations = std::make_pair(fMultiVarible, nullptr);
    parent.isBranch = true;
    parent.brachOperation = branchOperation::add;
    parent.nodeData = other;
    return parent;
  };

  multiVaribleFunction operator-=(const function& other) {
    std::shared_ptr<multiVaribleFunction> fMultiVarible = std::make_shared<multiVaribleFunction>(*this);
    multiVaribleFunction parent;
    parent.branchEquations = std::make_pair(fMultiVarible, nullptr);
    parent.isBranch = true;
    parent.brachOperation = branchOperation::subtract;
    parent.nodeData = other;
    return parent;
  };

  multiVaribleFunction operator+=(const double& other) {
    std::shared_ptr<multiVaribleFunction> fMultiVarible = std::make_shared<multiVaribleFunction>(*this);
    multiVaribleFunction parent;
    parent.branchEquations = std::make_pair(fMultiVarible, nullptr);
    parent.isBranch = true;
    parent.brachOperation = branchOperation::add;
    parent.nodeData = createConstantFunction(other);
    return parent;
  };

  multiVaribleFunction operator-=(const double& other) {
    std::shared_ptr<multiVaribleFunction> fMultiVarible = std::make_shared<multiVaribleFunction>(*this);
    multiVaribleFunction parent;
    parent.branchEquations = std::make_pair(fMultiVarible, nullptr);
    parent.isBranch = true;
    parent.brachOperation = branchOperation::subtract;
    parent.nodeData = createConstantFunction(-other);
    return parent;
  };
};


  
namespace Operation {
  operationPtr constant(double arg);
  
  operationPtr add(double arg);

  operationPtr multiply(double arg);
  
  operationPtr divide(double arg);

  operationPtr exp(double arg1, double arg2);

  operationPtr sin(double A = 1.0, double frequency = 1/(2*M_PI), double theta = 0);
  
  operationPtr cos(double A = 1.0, double frequency = 1/(2*M_PI), double theta = 0);
  
  operationPtr scaleToOne();
  // TODO: add more functions
  
};
