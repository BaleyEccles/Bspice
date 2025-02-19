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
  OPAMP,
  DIODE
};

enum connectionType {
  UNDEFINED = 0,
  OPAMP_N,
  OPAMP_P,
  OPAMP_OUT,
  DIODE_P,
  DIODE_N,
  BJT_BASE,
  BJT_COLLECTOR,
  BJT_EMITTER,
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

class Opamp : public Component {
 public:
  Opamp(const std::string& Name);
};

class Diode : public Component {
 public:
  Diode(const std::string& Name, double Value);
  double voltageDrop = 0.0;
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
  void addComponent(std::shared_ptr<Component> component, connectionType cType);
  std::string nodeName;
  std::vector<std::pair<std::shared_ptr<Component>, connectionType>> components;
};

// Templated definitions // 

template<typename T1, typename T2, typename T3>
class Circuit{
 public:
  Circuit();
  // Main things
  void addNode(Node *node);
  void addComponent(std::shared_ptr<Component> component);
  void calculate();
  std::vector<Node *> nodes;
  std::vector<std::shared_ptr<Component>> components;

  // Data
  double stopTime, timeStep;
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
  int findEquationLocationFromSymbol(std::string symName);
  void generateComponentConections();

  function createVoltageFunction(VoltageSource::functionType& type, std::vector<double>& values);

};


template<typename T1, typename T2, typename T3>
Circuit<T1, T2, T3>::Circuit() {}

template<typename T1, typename T2, typename T3>
void Circuit<T1, T2, T3>::addNode(Node *node) {
  nodes.push_back(node);
}
template<typename T1, typename T2, typename T3>
void Circuit<T1, T2, T3>::calculate() {
  generateSymbols();
  preAllocateMatrixData();

  generateComponentConections();
  generateMatrices();

  A.print("A:");
  E.print("E:");
  f.print("f:");
  syms.print("syms:");
  initalValues.print("Inital Values:");
}

template<typename T1, typename T2, typename T3>
void Circuit<T1, T2, T3>::generateMatrices() {
  
  for (auto node : nodes) {
    int equationNumber = findEquationLocationFromSymbol(node->nodeName);
    if (node->nodeName == "GND") {
      A.data[equationNumber][equationNumber] = 1;
    }
    for (auto c : node->components) {
      switch (c.first->Type) {
      case ComponentType::VOLTAGESOURCE: {
        // Add 1 to the current location for this node/equation
        // and set the voltage for this node to be Vcc
        auto comp = dynamic_cast<VoltageSource *>(c.first.get());
        if (comp->Connections.size() != 2) {
          std::cerr << "ERROR: Voltage sources must have two connections" << std::endl;
        }
        if (comp->Connections[0]->nodeName != "GND" && comp->Connections[1]->nodeName != "GND") {
        std::cerr << "ERROR: Voltage sources must connect to GND" << std::endl;
        }
        int componentCurrentLocation = findEquationLocationFromSymbol("i_" + comp->ComponentName);
        A.data[equationNumber][componentCurrentLocation] += 1;

        int componentNodeConnection1 = findNodeLocationFromNode(comp->Connections[0]);
        int componentNodeConnection2 = findNodeLocationFromNode(comp->Connections[1]);

        //if (node == c.first->Connections[0]) {
          A.data[componentCurrentLocation][componentNodeConnection1] += 1;
          A.data[componentCurrentLocation][componentNodeConnection2] -= 1;
          //}
        if constexpr (std::is_arithmetic<T3>::value) {
          f.data[componentCurrentLocation][0] += comp->Values[0];
        } else if constexpr (std::is_same<T3, function>::value) {
          f.data[componentCurrentLocation][0] = f.data[componentCurrentLocation][0] + createVoltageFunction(comp->fType, comp->Values);;
        }
        break;
      }
      case ComponentType::RESISTOR: {
        // Add +- G to the specific matrix location
        auto comp = dynamic_cast<Resistor *>(c.first.get());
        if (comp->Connections.size() != 2) std::cerr << "ERROR: Resistors must have two connections" << std::endl;
        int componentNodeConnection1 = findNodeLocationFromNode(comp->Connections[0]);
        int componentNodeConnection2 = findNodeLocationFromNode(comp->Connections[1]);
        if (node == c.first->Connections[0]) {
          A.data[equationNumber][componentNodeConnection1] += 1/comp->Resistance;
          A.data[equationNumber][componentNodeConnection2] -= 1/comp->Resistance;
        } else {
          A.data[equationNumber][componentNodeConnection1] -= 1/comp->Resistance;
          A.data[equationNumber][componentNodeConnection2] += 1/comp->Resistance;
        }
        break;
      }
      case ComponentType::CAPACITOR: {
        // Add +- C to the specific matrix location
        auto comp = dynamic_cast<Capacitor *>(c.first.get());
        if (comp->Connections.size() != 2) std::cerr << "ERROR: Capacitors must have two connections" << std::endl;
        int componentNodeConnection1 = findNodeLocationFromNode(comp->Connections[0]);
        int componentNodeConnection2 = findNodeLocationFromNode(comp->Connections[1]);
        E.data[equationNumber][componentNodeConnection1] += comp->Capacitance;
        E.data[equationNumber][componentNodeConnection2] -= comp->Capacitance;
        break;
      }
      default: {
        std::cerr << "ERROR: Component of type: " << c.first->Type << " and name: " << c.first->ComponentName << " was not handled" << std::endl;
        break;
      }
      }
    }
  }
}

template<typename T1, typename T2, typename T3>
function Circuit<T1, T2, T3>::createVoltageFunction(VoltageSource::functionType& type, std::vector<double>& values) {
  function f;
  switch (type) {
  case VoltageSource::functionType::NONE: {
    f.addOperation(Operation::multiply(0.0));
    f.addOperation(Operation::add(values[0]));
    break;
  }
  case VoltageSource::functionType::AC: {
    if (values.size() != 3) {
      std::cerr << "ERROR: AC must have three arguments: magnitude, frequency and phase shift." << std::endl;
    }
    f.addOperation(Operation::sin(values[0], values[1], values[2]));
    break;
  }
  case VoltageSource::functionType::SQUARE_WAVE: {
    if (values.size() != 3) {
      std::cerr << "ERROR: SQUARE waves must have three arguments: magnitude, frequency and phase shift." << std::endl;
    }
    f.addOperation(Operation::sin(1.0, values[1], values[2]));
    f.addOperation(Operation::scaleToOne());
    f.addOperation(Operation::multiply(values[0]));
    break;
  }
  default: {
    std::cerr << "ERROR: Unable to create function, maybe not done yet?" << std::endl;
    break;
  }
  }
  return f;
}


template<typename T1, typename T2, typename T3>
std::vector<Node *> Circuit<T1, T2, T3>::findNodeFromComponent(std::shared_ptr<Component> comp1) {
  std::vector<Node *> nodesWithComponent;
  for (auto node : nodes) {
    for (auto comp2 : node->components) {
      if (comp1->ComponentName == comp2.first->ComponentName) {
        nodesWithComponent.push_back(node);
      }
    }
  }
  return nodesWithComponent;
}
template<typename T1, typename T2, typename T3>
int Circuit<T1, T2, T3>::findNodeLocationFromNode(Node *node) {
  for (int i = 0; i < syms.rows; i++) {
    for (int j = 0; j < syms.cols; j++) {
      if (syms.data[i][j].name == node->nodeName) {
        return i;
      }
    }
  }
  std::cerr << "ERROR: Location Not Found" << std::endl;
  return -1;
}
template<typename T1, typename T2, typename T3>
int Circuit<T1, T2, T3>::findEquationLocationFromSymbol(std::string symName) {
  for (int i = 0; i < syms.rows; i++) {
    for (int j = 0; j < syms.cols; j++) {
      if (syms.data[i][j].name == symName) {
        return i;
      }
    }
  }
  //std::cerr << "ERROR: Location Not Found" << std::endl;
  return -1;
}
template<typename T1, typename T2, typename T3>
void Circuit<T1, T2, T3>::generateSymbols() {
  bool hasGround = false;
  for (auto node : nodes) {
    syms.data.push_back({symbol(node->nodeName)});
    if (node->nodeName == "GND") hasGround = true;
  }
  if (hasGround == false) {
    std::cerr << "ERROR: Circuit has no ground connection, please define a node with the name \"GND\"" << std::endl;
  }
  syms.rows = syms.data.size();
  syms.cols = 1;
  for (auto node : nodes) {
    for (auto c : node->components) {
      if (c.first->Type == ComponentType::VOLTAGESOURCE || c.first->Type == ComponentType::INDUCTOR) {
        symbol componetCurrent = symbol("i_" + c.first->ComponentName);
        auto symsTransposed = syms.transpose();
        auto it = std::find(symsTransposed.data[0].begin(), symsTransposed.data[0].end(), componetCurrent);

        if (it == symsTransposed.data[0].end()) {
          syms.data.push_back({componetCurrent});
          syms.rows++;
        }
      } else if (c.first->Type == ComponentType::OPAMP) {
        symbol componetCurrentP = symbol("i_" + c.first->ComponentName + "P");
        symbol componetCurrentN = symbol("i_" + c.first->ComponentName + "N");
        auto symsTransposed = syms.transpose();
        auto it1 = std::find(symsTransposed.data[0].begin(), symsTransposed.data[0].end(), componetCurrentP);
        if (it1 == symsTransposed.data[0].end()) {
          syms.data.push_back({componetCurrentP});
          syms.rows++;
        }
        auto it2 = std::find(symsTransposed.data[0].begin(), symsTransposed.data[0].end(), componetCurrentN);
        if (it2 == symsTransposed.data[0].end()) {
          syms.data.push_back({componetCurrentN});
          syms.rows++;
        }

      }
    }
  }
  syms.rows = syms.data.size();
  syms.cols = 1;
}
template<typename T1, typename T2, typename T3>
void Circuit<T1, T2, T3>::preAllocateMatrixData() {
  int matrixSize = syms.rows;
  A.rows = matrixSize;
  A.cols = matrixSize;
  std::vector<std::vector<T1>> Adata(A.rows,
                                     std::vector<T1>(A.cols, 0.0));
  A.data = Adata;
  E.rows = matrixSize;
  E.cols = matrixSize;
  std::vector<std::vector<T2>> Edata(E.rows,
                                     std::vector<T2>(E.cols, 0.0));
  E.data = Edata;
  f.rows = matrixSize;
  f.cols = 1;
  std::vector<std::vector<T3>> fdata;
  if constexpr (std::is_arithmetic<T3>::value) {
    std::vector<std::vector<T3>> fArithmetic(f.rows,
                                             std::vector<T3>(f.cols, 0.0));
    fdata = fArithmetic;
  } else if constexpr (std::is_same<T3, function>::value) {
    function f0;
    f0.addOperation(Operation::multiply(0.0));
    std::vector<std::vector<T3>> fFunction(f.rows,
                                           std::vector<T3>(f.cols, f0));
    fdata = fFunction;
    
  }
  f.data = fdata;
  initalValues.rows = matrixSize;
  initalValues.cols = 1;
  std::vector<std::vector<double>> initalValuesdata(
                                                    initalValues.rows, std::vector<double>(initalValues.cols, 0.0));
  initalValues.data = initalValuesdata;
}

// This defines the current direction
template<typename T1, typename T2, typename T3>
void Circuit<T1, T2, T3>::generateComponentConections() {
  for (auto node : nodes) {
    for (auto c : node->components) {
      if (c.first->Connections.size() == 0) {
        c.first->Connections = findNodeFromComponent(c.first);
      }
    }
  }
};

