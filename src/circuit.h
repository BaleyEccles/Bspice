#pragma once
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include "BMaths/BMaths.h"
#include "component.h"

template<typename T1, typename T2, typename T3, std::size_t rowsT, std::size_t colsT>
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
  matrix<T1, colsT, rowsT> A;
  matrix<T2, colsT, rowsT> E;
  matrix<T3, 1, rowsT> f;
  matrix<double, 1, rowsT> initalValues;
  matrix<symbol, 1, rowsT> syms;

  // Helper functions
  matrix<symbol, 1, rowsT> removeGroundSym();
private:
  void generateMatrices();
  std::vector<Node*> findNodeFromComponent(std::shared_ptr<Component> comp);
  void generateSymbols();
  void preAllocateMatrixData();
  int findNodeLocationFromNode(Node* node);
  int findNodeLocationFromSymbol(std::string symName);
  void generateComponentConections();
  int findEquationLocationFromSymbol(std::string s);
  bool isInSymbols(symbol sym);

  function createVoltageFunction(VoltageSource::functionType& type, std::vector<double>& values);

};


template<typename T1, typename T2, typename T3, std::size_t colsT, std::size_t rowsT>
Circuit<T1, T2, T3, colsT, rowsT>::Circuit() {}
template<typename T1, typename T2, typename T3, std::size_t colsT, std::size_t rowsT>
void Circuit<T1, T2, T3, colsT, rowsT>::addNode(Node *node) { nodes.push_back(node); }
template<typename T1, typename T2, typename T3, std::size_t colsT, std::size_t rowsT>
void Circuit<T1, T2, T3, colsT, rowsT>::calculate() {
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
template<typename T1, typename T2, typename T3, std::size_t colsT, std::size_t rowsT>
void Circuit<T1, T2, T3, colsT, rowsT>::generateMatrices() {
  
  int equationNumber = 0;
  for (auto node : nodes) {
    if (node->nodeName == "GND") {
      int GNDLocation = findNodeLocationFromNode(node); // node == GND
      A.data[equationNumber][GNDLocation] = 1;
    } else {
      for (auto c : node->components) {
        switch (c.first->Type) {
        case Component::ComponentType::RESISTOR: {
          if (c.first->Connections.size() > 2) {
            std::cerr << "ERROR: Resistor " << c.first->ComponentName << " has too many connections" << std::endl;
          } else if (c.first->Connections.size() < 2) {
            std::cerr << "ERROR: Resistor " << c.first->ComponentName << " has not got enough connections" << std::endl;
          }
          
          int componentConnectionIdx1 = findNodeLocationFromNode(c.first->Connections[0]);
          int componentConnectionIdx2 = findNodeLocationFromNode(c.first->Connections[1]);
          auto resistor = dynamic_cast<Resistor *>(c.first.get());

          if (node == c.first->Connections[0]) {
            A.data[equationNumber][componentConnectionIdx1] += 1 / resistor->Resistance;
            A.data[equationNumber][componentConnectionIdx2] -= 1 / resistor->Resistance;
          } else {
            A.data[equationNumber][componentConnectionIdx1] -= 1 / resistor->Resistance;
            A.data[equationNumber][componentConnectionIdx2] += 1 / resistor->Resistance;
          }
          break;
        }
        case Component::ComponentType::CAPACITOR: {
          if (c.first->Connections.size() > 2) {
            std::cerr << "ERROR: Capacitor " << c.first->ComponentName << " has too many connections" << std::endl;
          } else if (c.first->Connections.size() < 2) {
            std::cerr << "ERROR: Capacitor " << c.first->ComponentName << " has not got enough connections" << std::endl;
          }
          
          int componentConnectionIdx1 = findNodeLocationFromNode(c.first->Connections[0]);
          int componentConnectionIdx2 = findNodeLocationFromNode(c.first->Connections[1]);
          auto capacitor = dynamic_cast<Capacitor *>(c.first.get());

          if (node == c.first->Connections[0]) {
            if (c.first->Connections[0]->nodeName != "GND") {
              E.data[equationNumber][componentConnectionIdx1] += capacitor->Capacitance;
            }
            if (c.first->Connections[1]->nodeName != "GND") {
              E.data[equationNumber][componentConnectionIdx2] += -capacitor->Capacitance;
            }
          } else {
            if (c.first->Connections[0]->nodeName != "GND") {
              E.data[equationNumber][componentConnectionIdx1] -= capacitor->Capacitance;
            }
            if (c.first->Connections[1]->nodeName != "GND") {
              E.data[equationNumber][componentConnectionIdx2] -= -capacitor->Capacitance;
            }
          }
          break;
        }
        case Component::ComponentType::INDUCTOR: {
          if (c.first->Connections.size() > 2) {
            std::cerr << "ERROR: Inductor " << c.first->ComponentName << " has too many connections" << std::endl;
          } else if (c.first->Connections.size() < 2) {
            std::cerr << "ERROR: Inductor " << c.first->ComponentName << " has not got enough connections" << std::endl;
          }
          
          int componentCurrentIdx = findNodeLocationFromSymbol("i_" + c.first->ComponentName);
          int componentConnectionIdx1 = findNodeLocationFromNode(c.first->Connections[0]);
          int componentConnectionIdx2 = findNodeLocationFromNode(c.first->Connections[1]);
          auto inductor = dynamic_cast<Inductor *>(c.first.get());

          if (node == c.first->Connections[0]) {
            if (c.first->Connections[0]->nodeName != "GND") {
              A.data[equationNumber][componentCurrentIdx] += 1;
            }
          } else {
            if (c.first->Connections[0]->nodeName != "GND") {
              A.data[equationNumber][componentCurrentIdx] -= 1;
            }
          }
          if (node == c.first->Connections[0]) {
            if (c.first->Connections[0]->nodeName != "GND") {
              A.data[componentCurrentIdx][componentConnectionIdx1] += 1;
              E.data[componentCurrentIdx][componentCurrentIdx] -= inductor->Inductance;
            }
          } else {
            if (c.first->Connections[0]->nodeName != "GND") {
              A.data[componentCurrentIdx][componentConnectionIdx2] -= 1;
            }
          }
          break;
        }
        case Component::ComponentType::VOLTAGESOURCE: {
          int componentConnectionIdx1 = findNodeLocationFromNode(c.first->Connections[0]);
          int componentCurrentIdx = findNodeLocationFromSymbol("i_" + c.first->ComponentName);
          auto voltageSource = dynamic_cast<VoltageSource *>(c.first.get());
          A.data[equationNumber][componentCurrentIdx] = 1;
          A.data[componentCurrentIdx][equationNumber] = 1;
          if constexpr (std::is_arithmetic<T3>::value) {
            f.data[componentCurrentIdx][0] += voltageSource->Values[0];
          }  else if constexpr (std::is_same<T3, function>::value) {
            f.data[componentCurrentIdx][0] = f.data[componentCurrentIdx][0] + createVoltageFunction(voltageSource->fType, voltageSource->Values);
          }
          break;
        }
        case Component::ComponentType::OPAMP: {
          // using:
          // I_- = I_+ = 0A
          // Vout = A * (V_+ - V_-)
          
          int nodeLocationCurrentP = findNodeLocationFromSymbol("i_" + c.first->ComponentName + "P");
          int nodeLocationCurrentN = findNodeLocationFromSymbol("i_" + c.first->ComponentName + "N");
          A.data[nodeLocationCurrentP][nodeLocationCurrentP] = 1;
          A.data[nodeLocationCurrentN][nodeLocationCurrentN] = 1;
          
          auto opamp = dynamic_cast<Opamp *>(c.first.get());
          auto amp = 100e3; // FIXME: This should be user controlled
          int nodeLocationVout = -1;
          for (auto& n : findNodeFromComponent(c.first)) {
            for (auto& com : n->components) {
              if (com.second == Component::OPAMP_OUT && c.first->ComponentName == com.first->ComponentName) {
                nodeLocationVout = findNodeLocationFromNode(n);
              }
            }
          }

          switch (c.second) {
          case Component::OPAMP_P: {
            int pos = findNodeLocationFromNode(node);
            A.data[nodeLocationVout][pos] = -amp;
            break;
          }
          case Component::OPAMP_N: {
            int pos = findNodeLocationFromNode(node);
            A.data[nodeLocationVout][pos] = amp;
            break;
          }
          case Component::OPAMP_OUT:{
            int pos = findNodeLocationFromNode(node);
            A.data[nodeLocationVout][pos] = 1;
            break;
          }
          default: {
            std::cerr << "ERROR: Opamp contains the wrong connection type." << std::endl;
            break;
          }
          }
          break;
        }
        case Component::ComponentType::DIODE: {
          std::cerr << "ERROR: Diodes not done yet" << std::endl;
          break;
        }
        default: {
          std::cerr << "ERROR: Component of type: " << c.first->Type << " and name: " << c.first->ComponentName << " was not handled" << std::endl;
        }
        }
      }

    }
    equationNumber++;
  }
}

template<typename T1, typename T2, typename T3, std::size_t colsT, std::size_t rowsT>
function Circuit<T1, T2, T3, colsT, rowsT>::createVoltageFunction(VoltageSource::functionType& type, std::vector<double>& values) {
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


template<typename T1, typename T2, typename T3, std::size_t colsT, std::size_t rowsT>
std::vector<Node *> Circuit<T1, T2, T3, colsT, rowsT>::findNodeFromComponent(std::shared_ptr<Component> comp1) {
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

template<typename T1, typename T2, typename T3, std::size_t colsT, std::size_t rowsT>
int Circuit<T1, T2, T3, colsT, rowsT>::findNodeLocationFromNode(Node *node) {
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
template<typename T1, typename T2, typename T3, std::size_t colsT, std::size_t rowsT>
int Circuit<T1, T2, T3, colsT, rowsT>::findNodeLocationFromSymbol(std::string symName) {
  for (int i = 0; i < syms.rows; i++) {
    for (int j = 0; j < syms.cols; j++) {
      if (syms.data[i][j].name == symName) {
        return i;
      }
    }
  }
  std::cerr << "ERROR: Location Not Found" << std::endl;
  return -1;
}

template<typename T1, typename T2, typename T3, std::size_t colsT, std::size_t rowsT>
void Circuit<T1, T2, T3, colsT, rowsT>::generateSymbols() {
  bool hasGround = false;
  syms.rows = 0;
  syms.cols = 1;
  for (auto node : nodes) {
    syms.data[syms.rows][0] = node->nodeName;
    syms.rows++;
  }

  for (auto node : nodes) {
    for (auto c : node->components) {
      if (c.first->Type == Component::ComponentType::VOLTAGESOURCE || c.first->Type == Component::ComponentType::INDUCTOR) {
        symbol componetCurrent = symbol("i_" + c.first->ComponentName);
        
        if (!isInSymbols(componetCurrent)) {
          syms.data[syms.rows][0] = componetCurrent;
          syms.rows++;
        }
      } else if (c.first->Type == Component::ComponentType::OPAMP) {
        symbol componetCurrentP = symbol("i_" + c.first->ComponentName + "P");
        symbol componetCurrentN = symbol("i_" + c.first->ComponentName + "N");
        
        if (!isInSymbols(componetCurrentP)) {
          syms.data[syms.rows][0] = componetCurrentP;
          syms.rows++;
        }
        if (!isInSymbols(componetCurrentN)) {
          syms.data[syms.rows][0] = componetCurrentN;
          syms.rows++;
        }

      }
    }
  }
  syms.rows = syms.data.size();
}

template<typename T1, typename T2, typename T3, std::size_t colsT, std::size_t rowsT>
bool Circuit<T1, T2, T3, colsT, rowsT>::isInSymbols(symbol sym) {
  auto symsTransposed = syms.transpose();
  auto it = std::find(symsTransposed.data[0].begin(), symsTransposed.data[0].end(), sym);
  if (it == symsTransposed.data[0].end()) {
    return false;
  }
  return true;
}

template<typename T1, typename T2, typename T3, std::size_t colsT, std::size_t rowsT>
void Circuit<T1, T2, T3, colsT, rowsT>::preAllocateMatrixData() {
  int matrixSize = syms.rows;
  A.rows = matrixSize;
  A.cols = matrixSize;
  A.createData();
  
  E.rows = matrixSize;
  E.cols = matrixSize;
  E.createData();
  
  f.rows = matrixSize;
  f.cols = 1;
  f.createData();
  
  initalValues.rows = matrixSize;
  initalValues.cols = 1;
  initalValues.createData();
}

// This defines the current direction
template<typename T1, typename T2, typename T3, std::size_t colsT, std::size_t rowsT>
void Circuit<T1, T2, T3, colsT, rowsT>::generateComponentConections() {
  for (auto node : nodes) {
    for (auto c : node->components) {
      if (c.first->Connections.size() == 0) {
        c.first->Connections = findNodeFromComponent(c.first);
      }
    }
  }
};
