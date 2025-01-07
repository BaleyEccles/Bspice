#include "Circuit.h"



Component::Component(const std::string& Name, ComponentType Type)
  : ComponentName(Name), Type(Type) {}

Resistor::Resistor(const std::string &Name, double Value)
  : Component(Name, ComponentType::RESISTOR), Resistance(Value) {}

Capacitor::Capacitor(const std::string& Name, double Value)
  : Component(Name, ComponentType::CAPACITOR), Capacitance(Value) {}

VoltageSource::VoltageSource(const std::string& Name, double Value)
  : Component(Name, ComponentType::VOLTAGESOURCE), Voltage(Value) {}

Node::Node(const std::string& name)
  : nodeName(name) {}

void Node::addComponent(std::shared_ptr<Component> component) {
  components.push_back(component);
}

Circuit::Circuit() {}

void Circuit::addNode(Node* node) {
  nodes.push_back(node);
}


void Circuit::calculate() {
  generateMatrices();

  A.print();
  E.print();
  f.print();
  initalValues.print();
}


void Circuit::generateMatrices() {
  generateSymbols();
  preAllocateMatrixData();
  int i = 0;
  std::cout << "A:" << std::endl;
  for (auto node : nodes) {
    //A.print();
    for (auto c : node->components) {
      switch (c->Type) {
      case ComponentType::RESISTOR: {
        auto NodesWithCurrentComponent = findNodeFromComponent(c);
        if (NodesWithCurrentComponent.size() > 2) {
          std::cerr << "ERROR: Resistor " << c->ComponentName << " has too many connections" << std::endl;
        } else if (NodesWithCurrentComponent.size() < 2) {
          std::cerr << "ERROR: Resistor " << c->ComponentName << " has not enough connections" << std::endl;
        }
        int nodeLocation1 = findNodeLocationFromNode(NodesWithCurrentComponent[0]);
        int nodeLocation2 = findNodeLocationFromNode(NodesWithCurrentComponent[1]);
        auto resistor = dynamic_cast<Resistor*>(c.get());
        if (nodeLocation1 == 0) {
          A.data[i][nodeLocation1] +=  1/resistor->Resistance;
          A.data[i][nodeLocation2] += -1/resistor->Resistance;
        }
        if (nodeLocation1 == 1) {
          A.data[i][nodeLocation1] -=  1/resistor->Resistance;
          A.data[i][nodeLocation2] -= -1/resistor->Resistance;
        }
        break;
      }
      case ComponentType::CAPACITOR: {
        auto NodesWithCurrentComponent = findNodeFromComponent(c);
        if (NodesWithCurrentComponent.size() > 2) {
          std::cerr << "ERROR: Capacitor " << c->ComponentName << " has too many connections" << std::endl;
        } else if (NodesWithCurrentComponent.size() < 2) {
          std::cerr << "ERROR: Capacitor " << c->ComponentName << " has not enough connections" << std::endl;
        }
        int nodeLocation1 = findNodeLocationFromNode(NodesWithCurrentComponent[0]);
        int nodeLocation2 = findNodeLocationFromNode(NodesWithCurrentComponent[1]);
        auto capacitor = dynamic_cast<Capacitor*>(c.get());
        E.data[i][nodeLocation1] +=  1/capacitor->Capacitance;
        E.data[i][nodeLocation2] += -1/capacitor->Capacitance;
        break;
      }
      case ComponentType::VOLTAGESOURCE: {
        auto NodesWithCurrentComponent = findNodeFromComponent(c);
        // FIXME: deal with more than one node per voltage
        int nodeLocation1 = findNodeLocationFromNode(NodesWithCurrentComponent[0]);
        auto voltageSource = dynamic_cast<VoltageSource*>(c.get());
        initalValues.data[nodeLocation1][0] += voltageSource->Voltage;
        break;
      } // TODO: add other components
      default: {
        std::cerr << "ERROR: Component of type: " << c->Type << " and name: " << c->ComponentName << " was not handled" << std::endl;
      }

      }
    }
    i++;
  }
}



std::vector<Node*> Circuit::findNodeFromComponent(std::shared_ptr<Component> comp1) {
  std::vector<Node*> nodesWithComponent;
  for (auto node : nodes) {
    for (auto comp2 : node->components) {
      if (comp1->ComponentName == comp2->ComponentName) {
        nodesWithComponent.push_back(node);
      }
    }
  }
  return nodesWithComponent;
}

int Circuit::findNodeLocationFromNode(Node* node) {
  for (int i = 0 ; i < syms.rows; i++) {
    for (int j = 0 ; j < syms.cols; j++) {
      if (syms.data[i][j].name == node->nodeName) {
        return i;
      }
    }    
  }
  return -1;
}

void Circuit::generateSymbols() {
  for (auto node : nodes) {
    syms.data.push_back({symbol(node->nodeName)});
  }
  syms.rows = syms.data.size();
  syms.cols = 1;
}


void Circuit::preAllocateMatrixData() {
  int matrixSize = syms.rows;
  A.rows = matrixSize;
  A.cols = matrixSize;
  std::vector<std::vector<double>> Adata(A.rows, std::vector<double>(A.cols, 0.0));
  A.data = Adata;
  E.rows = matrixSize;
  E.cols = matrixSize;
  std::vector<std::vector<double>> Edata(E.rows, std::vector<double>(E.cols, 0.0));
  E.data = Edata;
  f.rows = matrixSize;
  f.cols = 1;
  std::vector<std::vector<double>> fdata(f.rows, std::vector<double>(f.cols, 0.0));
  f.data = fdata;
  initalValues.rows = matrixSize;
  initalValues.cols = 1;
  std::vector<std::vector<double>> initalValuesdata(initalValues.rows, std::vector<double>(initalValues.cols, 0.0));
  initalValues.data = initalValuesdata;
                   
}
