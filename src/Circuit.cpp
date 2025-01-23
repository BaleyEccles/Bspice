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

void Circuit::calculate() {
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

void Circuit::generateMatrices() {

  int equationNumber = 0;
  for (auto node : nodes) {
    if (node->nodeName != "GND") {
      for (auto c : node->components) {
        switch (c->Type) {
        case ComponentType::RESISTOR: {
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

          if (node == c->Connections[0]) {
            A.data[equationNumber][nodeLocation1] += 1 / resistor->Resistance;
            A.data[equationNumber][nodeLocation2] += -1 / resistor->Resistance;
          } else {
            A.data[equationNumber][nodeLocation1] -= 1 / resistor->Resistance;
            A.data[equationNumber][nodeLocation2] -= -1 / resistor->Resistance;
          }

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

          if (node == c->Connections[0]) {
            if (c->Connections[0]->nodeName != "GND") {
              E.data[equationNumber][nodeLocation1] += capacitor->Capacitance;
            }
            if (c->Connections[1]->nodeName != "GND") {
              E.data[equationNumber][nodeLocation2] += -capacitor->Capacitance;
            }
          } else {
            if (c->Connections[0]->nodeName != "GND") {
              E.data[equationNumber][nodeLocation1] -= capacitor->Capacitance;
            }
            if (c->Connections[1]->nodeName != "GND") {
              E.data[equationNumber][nodeLocation2] -= -capacitor->Capacitance;
            }
          }
          break;
        }
        case ComponentType::INDUCTOR: {
          if (c->Connections.size() > 2) {
            std::cerr << "ERROR: Capacitor " << c->ComponentName
                      << " has too many connections" << std::endl;
          } else if (c->Connections.size() < 2) {
            std::cerr << "ERROR: Capacitor " << c->ComponentName
                      << " has not got enough connections" << std::endl;
          }
          int currentLocation =
              findNodeLocationFromSymbol("i_" + c->ComponentName);

          int nodeLocation1 = findNodeLocationFromNode(c->Connections[0]);
          int nodeLocation2 = findNodeLocationFromNode(c->Connections[1]);
          auto inductor = dynamic_cast<Inductor *>(c.get());

          // Unsure how right this is, but it works for the simple case
          if (node == c->Connections[0]) {
            if (c->Connections[0]->nodeName != "GND") {
              A.data[equationNumber][currentLocation] += 1;
            }
          } else {
            if (c->Connections[0]->nodeName != "GND") {
              A.data[equationNumber][currentLocation] -= 1;
            }
          }
          if (node == c->Connections[0]) {
            if (c->Connections[0]->nodeName != "GND") {
              A.data[currentLocation][nodeLocation1] += 1;
              E.data[currentLocation][currentLocation] -= inductor->Inductance;
            }
          } else {
            if (c->Connections[0]->nodeName != "GND") {
              A.data[currentLocation][nodeLocation1] -= 1;
              E.data[currentLocation][currentLocation] += inductor->Inductance;
            }
          }
          break;
        }
        case ComponentType::VOLTAGESOURCE: {
          int nodeLocation1 = findNodeLocationFromNode(c->Connections[0]);
          int nodeLocationCurrent =
              findNodeLocationFromSymbol("i_" + c->ComponentName);
          auto voltageSource = dynamic_cast<VoltageSource *>(c.get());
          initalValues.data[nodeLocation1][0] += voltageSource->Voltage;
          // FIXME?: This might break on diff circuits
          A.data[equationNumber][nodeLocationCurrent] = 1;
          A.data[nodeLocationCurrent][equationNumber] = 1;
          f.data[nodeLocationCurrent][0] += voltageSource->Voltage;

          break;
        }
        default: {
          std::cerr << "ERROR: Component of type: " << c->Type
                    << " and name: " << c->ComponentName << " was not handled"
                    << std::endl;
        }
        }
      }
    } else {
      int GNDLocation = findNodeLocationFromNode(node); // node == GND
      A.data[equationNumber][GNDLocation] = 1;
    }
    equationNumber++;
  }
}

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
  bool hasGround = false;
  for (auto node : nodes) {

    // if (node->nodeName != "GND") {
    syms.data.push_back({symbol(node->nodeName)});
    //}
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
  for (auto node : nodes) {
    for (auto c : node->components) {
      if (c->Type == ComponentType::VOLTAGESOURCE ||
          c->Type == ComponentType::INDUCTOR) {
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
  syms.rows = syms.data.size();
  syms.cols = 1;
}

void Circuit::preAllocateMatrixData() {
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
