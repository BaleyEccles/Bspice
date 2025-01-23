#include "Circuit.h"

Component::Component(const std::string &Name, ComponentType Type)
  : ComponentName(Name), Type(Type) {}

Resistor::Resistor(const std::string &Name, double Value)
  : Component(Name, ComponentType::RESISTOR), Resistance(Value) {}

Capacitor::Capacitor(const std::string &Name, double Value)
  : Component(Name, ComponentType::CAPACITOR), Capacitance(Value) {}

Inductor::Inductor(const std::string &Name, double Value)
  : Component(Name, ComponentType::INDUCTOR), Inductance(Value) {}

VoltageSource::VoltageSource(const std::string &Name, double Value)
  : Component(Name, ComponentType::VOLTAGESOURCE), Voltage(Value) {}

Node::Node(const std::string &name) : nodeName(name) {}

void Node::addComponent(std::shared_ptr<Component> component) {
  components.push_back(component);
}

Circuit::Circuit() {}

void Circuit::addNode(Node *node) { nodes.push_back(node); }
void Circuit::addComponent(std::shared_ptr<Component> component) { components.push_back(component); }

void Circuit::calculate() {
  generateSymbols();
  preAllocateMatrixData();

  generateComponentConections();
  generateMatrices();

  //  A = {
  //    {{0.00010, -0.00010, 1.00000},
  //     {-0.00010, 0.00020, 0.0},
  //     {1.00000, 0.00000, 0.00000}},
  //    3, 3
  //  };
  //  f = {
  //    {{0},
  //     {0},
  //     {5}
  //    },
  //    1, 3
  //  };

}

void Circuit::generateMatrices() {
  // Node equations (algebraic)
  int equationNumber = 0;
  for (auto node : nodes) {
    if (node->nodeName != "GND") {
      // A.print();
      for (auto c : node->components) {
        int equationLocation =
          findEquationLocationFromSymbol("i_" + c->ComponentName);
        if(c->Connections[0] == node) {
          A.data[equationNumber][equationLocation] = 1;
        } else {
          A.data[equationNumber][equationLocation] = -1;
        }
      }
    } else {
      int equationLocation = findEquationLocationFromSymbol("GND");
      A.data[equationNumber][equationLocation] = 1;
    }
    equationNumber++;
  }

  
  for (auto c : components) {
    switch (c->Type) {
    case ComponentType::RESISTOR: { // G*V_R - i_R = 0
      if (c->Connections.size() > 2) {
        std::cerr << "ERROR: Resistor " << c->ComponentName
                  << " has too many connections" << std::endl;
      } else if (c->Connections.size() < 2) {
        std::cerr << "ERROR: Resistor " << c->ComponentName
                  << " has not got enough connections" << std::endl;
      }
      int nodeLocation1 = findNodeLocationFromNode(c->Connections[0]);
      int nodeLocation2 = findNodeLocationFromNode(c->Connections[1]);
      auto resistor = dynamic_cast<Resistor *>(c.get());
      A.data[equationNumber][nodeLocation1] += 1 / resistor->Resistance;
      A.data[equationNumber][nodeLocation2] += -1 / resistor->Resistance;
      
      int equationLocation = findEquationLocationFromSymbol("i_" + c->ComponentName);
      A.data[equationNumber][equationLocation] = -1;
      break;
    }
    case ComponentType::CAPACITOR: {
      if (c->Connections.size() > 2) {
        std::cerr << "ERROR: Capacitor " << c->ComponentName
                  << " has too many connections" << std::endl;
      } else if (c->Connections.size() < 2) {
        std::cerr << "ERROR: Capacitor " << c->ComponentName
                  << " has not got enough connections" << std::endl;
      }
      int nodeLocation1 = findNodeLocationFromNode(c->Connections[0]);
      int nodeLocation2 = findNodeLocationFromNode(c->Connections[1]);
      auto capacitor = dynamic_cast<Capacitor *>(c.get());

      if (c->Connections[0]->nodeName != "GND") {
        E.data[equationNumber][nodeLocation1] += capacitor->Capacitance;
      } if (c->Connections[1]->nodeName != "GND") {
        E.data[equationNumber][nodeLocation2] -= capacitor->Capacitance;
      }

      int equationLocation = findEquationLocationFromSymbol("i_" + c->ComponentName);
      A.data[equationNumber][equationLocation] = -1;

      break;
    }
    case ComponentType::VOLTAGESOURCE: {
      // FIXME: deal with more than one node per voltage
      int nodeLocation1 = findNodeLocationFromNode(c->Connections[0]);
      int nodeVoltageLocation =
        findEquationLocationFromSymbol(c->Connections[0]->nodeName);
      auto voltageSource = dynamic_cast<VoltageSource *>(c.get());
      initalValues.data[nodeLocation1][0] += voltageSource->Voltage;
      // FIXME?: This might break on diff circuits
      A.data[equationNumber][nodeVoltageLocation] = 1;
      f.data[equationNumber][0] = voltageSource->Voltage;

      break;
    }
    }
    equationNumber++;
  }
}

int Circuit::findEquationLocationFromSymbol(std::string s) {
  for (int row = 0; row < syms.rows; row++) {
    if (s == syms.data[row][0].name) {
      return row;
    }
  }
  std::cerr << "ERROR: EquationLocation not found" << std::endl;
  return -1;
};


std::vector<Node *>
Circuit::findNodeFromComponent(std::shared_ptr<Component> comp1) {
  std::vector<Node *> nodesWithComponent;
  for (auto node : nodes) {
    for (auto comp2 : node->components) {
      if (comp1->ComponentName == comp2->ComponentName) {
        nodesWithComponent.push_back(node);
      }
    }
  }
  return nodesWithComponent;
}

int Circuit::findNodeLocationFromNode(Node *node) {
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

int Circuit::findNodeLocationFromSymbol(std::string symName) {
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

void Circuit::generateSymbols() {
  // Node equations
  bool hasGround = false;
  for (auto node : nodes) {
    syms.data.push_back({symbol(node->nodeName)});
    if (node->nodeName == "GND")
      hasGround = true;
  }
  if (hasGround == false) {
    std::cerr << "ERROR: Circuit has no ground connection, please define a "
      "node with the name \"GND\""
              << std::endl;
  }
  syms.rows = syms.data.size();
  syms.cols = 1;
  // current through component equations
  for (auto node : nodes) {
    for (auto c : node->components) {
      symbol componetCurrent = symbol("i_" + c->ComponentName);
      auto symsTransposed = syms.transpose();
      auto it = std::find(symsTransposed.data[0].begin(),
                          symsTransposed.data[0].end(), componetCurrent);

      if (it == symsTransposed.data[0].end()) {
        syms.data.push_back({componetCurrent});
        syms.rows++;
      }
    }
  }

}

void Circuit::preAllocateMatrixData() {
  // Equation for each node voltage and current through each component
  int matrixSize = syms.rows;
  A.rows = matrixSize;
  A.cols = matrixSize;
  std::vector<std::vector<double>> Adata(A.rows,
                                         std::vector<double>(A.cols, 0.0));
  A.data = Adata;
  E.rows = matrixSize;
  E.cols = matrixSize;
  std::vector<std::vector<double>> Edata(E.rows,
                                         std::vector<double>(E.cols, 0.0));
  E.data = Edata;
  f.rows = matrixSize;
  f.cols = 1;
  std::vector<std::vector<double>> fdata(f.rows,
                                         std::vector<double>(f.cols, 0.0));
  f.data = fdata;
  initalValues.rows = matrixSize;
  initalValues.cols = 1;
  std::vector<std::vector<double>> initalValuesdata(
                                                    initalValues.rows, std::vector<double>(initalValues.cols, 0.0));
  initalValues.data = initalValuesdata;
}

// This defines the current direction
void Circuit::generateComponentConections() {
  for (auto node : nodes) {
    for (auto c : node->components) {
      if (c->Connections.size() == 0) {
        c->Connections = findNodeFromComponent(c);
      }
    }
  }
};
