#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Bmaths.h"

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

class VoltageSource : public Component {
 public:
  VoltageSource(const std::string& Name, double Value);
  double Voltage;
};

class Node {
 public:
  Node(const std::string& name);
  void addComponent(std::shared_ptr<Component> component);
  std::string nodeName;
  std::vector<std::shared_ptr<Component>> components;
};

class Circuit {
 public:
  Circuit();
  // Main things
  void addNode(Node* node);
  void calculate();
  std::vector<Node*> nodes;

  // Data 
  std::vector<double> time;
  matrix<double> A, E, f, initalValues;
  matrix<symbol> syms;

  // Helper functions
  matrix<symbol> removeGroundSym();
private:
  void generateMatrices();
  std::vector<Node*> findNodeFromComponent(std::shared_ptr<Component> comp);
  void generateSymbols();
  void preAllocateMatrixData();
  int findNodeLocationFromNode(Node* node);
  void generateComponentConections();

};
