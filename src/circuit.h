#pragma once
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include "BMaths/BMaths.h"
#include "BMaths/function.h"
#include "component.h"

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
  values initalValues;
  matrix<symbol> symbols;
  bool isLinear = true;

  // Helper functions
  matrix<symbol> removeGroundSym();
private:
  void generateMatricesLinear();
  void generateMatricesNonLinear();
  void generateSymbols();
  void generateInitalValues();
  void preAllocateMatrixDataLinear();
  void preAllocateMatrixDataNonLinear();
  
  std::vector<Node*> findNodeFromComponent(std::shared_ptr<Component> comp);
  int findNodeLocationFromNode(Node* node);
  int findNodeLocationFromSymbol(std::string symName);
  void generateComponentConections();
  int findEquationLocationFromSymbol(std::string s);
  bool isInSymbols(symbol sym);

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
  generateInitalValues();
  
  generateComponentConections();
  if (isLinear) {
    preAllocateMatrixDataLinear();
    generateMatricesLinear();
  } else {
    preAllocateMatrixDataNonLinear();
    generateMatricesNonLinear();
  }

  A.print("A:");
  E.print("E:");
  f.print("f:");
  symbols.print("syms:");
  initalValues.print("Inital Values:");
}

template<typename T1, typename T2, typename T3>
void Circuit<T1, T2, T3>::generateMatricesNonLinear() {
  int equationNumber = 0;
  for (auto node : nodes) {
    if (node->nodeName == "GND") {
      int GNDLocation = findNodeLocationFromNode(node); // node == GND
      A.data[equationNumber][GNDLocation] = 1;
    } else {
      for (auto c : node->components) {
        switch (c.first->Type) {
        case Component::ComponentType::OPAMP:
        case Component::ComponentType::INDUCTOR:
        case Component::ComponentType::CURRENTSOURCE:
        case Component::ComponentType::CAPACITOR: {
          std::cerr << "TODO: Non linear component not done yet" << std::endl;
          break;
        }
        case Component::ComponentType::RESISTOR: {
          // i_R = G*V_node1 - G*V_node2
          int componentConnectionIdx1 = findNodeLocationFromNode(c.first->Connections[0]);
          int componentConnectionIdx2 = findNodeLocationFromNode(c.first->Connections[1]);
          auto resistor = dynamic_cast<Resistor *>(c.first.get());

          if (node == c.first->Connections[0]) {
            function fNode1 = createConstantFunction(1/resistor->Resistance, symbols.data[componentConnectionIdx1][0]);
            function fNode2 = createConstantFunction(-1/resistor->Resistance, symbols.data[componentConnectionIdx1][0]);
            A.data[equationNumber][0] += fNode1;
            A.data[equationNumber][0] -= fNode2;
          } else {
            function fNode1 = createConstantFunction(-1/resistor->Resistance, symbols.data[componentConnectionIdx1][0]);
            function fNode2 = createConstantFunction(1/resistor->Resistance, symbols.data[componentConnectionIdx1][0]);
            A.data[equationNumber][0] += fNode1;
            A.data[equationNumber][0] -= fNode2;
          }
          break;
        }
        case Component::ComponentType::VOLTAGESOURCE: {
          // f(X) + i_Vs = 0
          // AND
          // V_node1 - V_node2 = f(t)
          int componentConnectionIdx1 = findNodeLocationFromNode(c.first->Connections[0]);
          int componentCurrentIdx = findNodeLocationFromSymbol("i_" + c.first->ComponentName);
          auto voltageSource = dynamic_cast<VoltageSource *>(c.first.get());
          
          A.data[equationNumber][0] += createConstantFunction(1.0, symbols.data[componentCurrentIdx][0]);      // f(X) + i_Vs = 0
          A.data[componentCurrentIdx][0] += createConstantFunction(1.0, symbols.data[componentCurrentIdx][0]); // i_Vs = f(t)
          if constexpr (std::is_arithmetic<T3>::value) {
            f.data[componentCurrentIdx][0] += voltageSource->Values[0];
          }  else if constexpr (std::is_same<T3, function>::value) {
            f.data[componentCurrentIdx][0] += createVoltageFunction(voltageSource->fType, voltageSource->Values);
          }
          break;
        }
        case Component::ComponentType::DIODE: {
          // i_D = I_s* (e^{B*v_D} - 1) => I_s = I_s*e^{B*v_D} - i_D
          // AND
          // v_D = v_node1 - v_node2
          
          // FIXME: These should be user controlled
          // taken from wikipedia for now
          double I_s = 1e-9; // 1nA
          double n = 1.5; // Ideality factor
          double V_T = 25.852e-3; // 25.852mV
          double B = 1/(n*V_T);
            
          int componentConnectionIdx1 = findNodeLocationFromNode(c.first->Connections[0]);
          int componentConnectionIdx2 = findNodeLocationFromNode(c.first->Connections[1]);
          auto diode = dynamic_cast<Diode *>(c.first.get());
          
          // Shockleys equation
          int diodeCurrentLocation = findEquationLocationFromSymbol("i_" + diode->ComponentName);
          int diodeVoltageLocation = findEquationLocationFromSymbol("v_" + diode->ComponentName);
            
          function fExp;
          fExp.addOperation(Operation::exp(std::exp(1.0), B));
          fExp.functionSymbol = symbols.data[diodeVoltageLocation][0];
            
          function fCurrent;
          fCurrent.functionSymbol = symbols.data[diodeCurrentLocation][0];
          fCurrent.addOperation(Operation::multiply(-1/I_s));

          // Voltage accros diode
          function fVoltageNode1;
          fVoltageNode1.addOperation(Operation::multiply(1.0));
          fVoltageNode1.functionSymbol = symbols.data[componentConnectionIdx1][0];
          function fVoltageNode2;
          fVoltageNode2.addOperation(Operation::multiply(1.0));
          fVoltageNode2.functionSymbol = symbols.data[componentConnectionIdx2][0];
          function fVoltageDiode;
          fVoltageDiode.addOperation(Operation::multiply(1.0));
          fVoltageDiode.functionSymbol = symbols.data[diodeCurrentLocation][0];

          if (node == c.first->Connections[0]) {
            A.data[diodeCurrentLocation][0] += fExp;
            A.data[diodeCurrentLocation][0] += fCurrent;
            f.data[diodeCurrentLocation][0] += createConstantFunction(1.0, symbol("t"));
            
            A.data[diodeVoltageLocation][0] += fVoltageNode1;
            A.data[diodeVoltageLocation][0] -= fVoltageNode2;
            A.data[diodeVoltageLocation][0] -= fVoltageDiode;
          } else {
            A.data[diodeCurrentLocation][0] -= fExp;
            A.data[diodeCurrentLocation][0] -= fCurrent;
            f.data[diodeCurrentLocation][0] += createConstantFunction(1.0, symbol("t"));
            
            A.data[diodeVoltageLocation][0] += fVoltageNode1;
            A.data[diodeVoltageLocation][0] -= fVoltageNode2;
            A.data[diodeVoltageLocation][0] -= fVoltageDiode;
          }

          function fCurrentDiode;
          fCurrentDiode.addOperation(Operation::multiply(1.0));
          fCurrentDiode.functionSymbol = symbols.data[diodeCurrentLocation][0];
          A.data[equationNumber][0] += fCurrentDiode;
          break;
        }
        }
      }
    }
  }
}

template<typename T1, typename T2, typename T3>
void Circuit<T1, T2, T3>::generateMatricesLinear() {
  
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
        default: {
          std::cerr << "ERROR: Component of type: " << c.first->Type << " and name: " << c.first->ComponentName << " was not handled" << std::endl;
          break;
        }
        }
      }

    }
    equationNumber++;
  }
}

template<typename T1, typename T2, typename T3>
function Circuit<T1, T2, T3>::createVoltageFunction(VoltageSource::functionType& type, std::vector<double>& values) {
  function f;
  f.functionSymbol = symbol("t");
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
  for (int i = 0; i < symbols.rows; i++) {
    for (int j = 0; j < symbols.cols; j++) {
      if (symbols.data[i][j].name == node->nodeName) {
        return i;
      }
    }
  }
  std::cerr << "ERROR: Location Not Found" << std::endl;
  return -1;
}
template<typename T1, typename T2, typename T3>
int Circuit<T1, T2, T3>::findNodeLocationFromSymbol(std::string symName) {
  for (int i = 0; i < symbols.rows; i++) {
    for (int j = 0; j < symbols.cols; j++) {
      if (symbols.data[i][j].name == symName) {
        return i;
      }
    }
  }
  std::cerr << "ERROR: Location Not Found" << std::endl;
  return -1;
}

template<typename T1, typename T2, typename T3>
void Circuit<T1, T2, T3>::generateInitalValues() {
  for (int row = 0; row < symbols.rows; row++) {
    initalValues[symbols.data[row][0]] = 0.0;
  }
}
  
template<typename T1, typename T2, typename T3>
void Circuit<T1, T2, T3>::generateSymbols() {
  bool hasGround = false;
  symbols.rows = 0;
  symbols.cols = 1;
  for (auto node : nodes) {
    symbols.data.push_back({node->nodeName});
    symbols.rows++;
  }

  for (auto node : nodes) {
    for (auto c : node->components) {
      if (c.first->Type == Component::ComponentType::VOLTAGESOURCE || c.first->Type == Component::ComponentType::INDUCTOR) {
        symbol componetCurrent = symbol("i_" + c.first->ComponentName);
        
        if (!isInSymbols(componetCurrent)) {
          symbols.data.push_back({componetCurrent});
          symbols.rows++;
        }
      } else if (c.first->Type == Component::ComponentType::OPAMP) {
        symbol componetCurrentP = symbol("i_" + c.first->ComponentName + "P");
        symbol componetCurrentN = symbol("i_" + c.first->ComponentName + "N");
        
        if (!isInSymbols(componetCurrentP)) {
          symbols.data.push_back({componetCurrentP});
          symbols.rows++;
        }
        if (!isInSymbols(componetCurrentN)) {
          symbols.data.push_back({componetCurrentN});
          symbols.rows++;
        }
      } else if (c.first->Type == Component::ComponentType::OPAMP) {
        symbol componetVoltage = symbol("v_" + c.first->ComponentName);
        symbol componetCurrent = symbol("i_" + c.first->ComponentName);
        
        if (!isInSymbols(componetVoltage)) {
          symbols.data.push_back({componetVoltage});
          symbols.rows++;
        }
        if (!isInSymbols(componetCurrent)) {
          symbols.data.push_back({componetCurrent});
          symbols.rows++;
        }
      }
    }
  }
  symbols.rows = symbols.data.size();
}

template<typename T1, typename T2, typename T3>
bool Circuit<T1, T2, T3>::isInSymbols(symbol sym) {
  auto symsTransposed = symbols.transpose();
  auto it = std::find(symsTransposed.data[0].begin(), symsTransposed.data[0].end(), sym);
  if (it == symsTransposed.data[0].end()) {
    return false;
  }
  return true;
}

template<typename T1, typename T2, typename T3>
void Circuit<T1, T2, T3>::preAllocateMatrixDataNonLinear() {
  int matrixSize = symbols.rows;
  A.rows = matrixSize;
  A.cols = 1;
  A.createData();
  
  E.rows = matrixSize;
  E.cols = 1;
  E.createData();
  
  f.rows = matrixSize;
  f.cols = 1;
  f.createData();
  
  
}

template<typename T1, typename T2, typename T3>
void Circuit<T1, T2, T3>::preAllocateMatrixDataLinear() {
  int matrixSize = symbols.rows;
  A.rows = matrixSize;
  A.cols = matrixSize;
  A.createData();
  
  E.rows = matrixSize;
  E.cols = matrixSize;
  E.createData();
  
  f.rows = matrixSize;
  f.cols = 1;
  f.createData();
  

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
