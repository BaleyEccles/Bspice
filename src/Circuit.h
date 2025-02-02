#pragma once
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include "Bmaths/Bmaths.h"

enum ComponentType {
  VOLTAGESOURCE = 0,
  CURRENTSOURCE,
  RESISTOR,
  CAPACITOR,
  INDUCTOR,
};

class Node;
class Component {
 public:
  Component(const std::string& name, ComponentType Type);
  virtual ~Component() = default;
  std::string ComponentName;
  ComponentType Type;
  std::vector<Node*> Connections;
  
};

class Resistor : public Component {
 public:
  Resistor(const std::string& Name, double Value);
  double Resistance;
};

class Capacitor : public Component {
 public:
  Capacitor(const std::string& Name, double Value);
  double Capacitance;
};

class Inductor : public Component {
 public:
  Inductor(const std::string& Name, double Value);
  double Inductance;
};



class VoltageSource : public Component {
 public:
  enum functionType {
    NONE,
    AC,
    SQUARE_WAVE
  };
  
  VoltageSource(const std::string& Name, functionType type, std::vector<double> Values);
  std::vector<double> Values;
  functionType fType;
};

class Node {
 public:
  Node(const std::string& name);
  void addComponent(std::shared_ptr<Component> component);
  std::string nodeName;
  std::vector<std::shared_ptr<Component>> components;
};

template<typename T1, typename T2, typename T3>
class Circuit {
 public:
  Circuit();
  // Main things
  void addNode(Node *node);
  void addComponent(std::shared_ptr<Component> component);
  void calculate();
  std::vector<Node *> nodes;
  std::vector<std::shared_ptr<Component>> components;

  // Data 
  std::vector<double> time;
  matrix<T1> A;
  matrix<T2> E;
  matrix<T3> f;
  matrix<double> initalValues;
  matrix<symbol> syms;

  // Helper functions
  matrix<symbol> removeGroundSym();
private:
  void generateMatrices();
  std::vector<Node*> findNodeFromComponent(std::shared_ptr<Component> comp);
  void generateSymbols();
  void preAllocateMatrixData();
  int findNodeLocationFromNode(Node* node);
  int findNodeLocationFromSymbol(std::string symName);
  void generateComponentConections();
  int findEquationLocationFromSymbol(std::string s);

  function createVoltageFunction(VoltageSource::functionType& type, std::vector<double>& values);

};
#include "Circuit_impl.h"
