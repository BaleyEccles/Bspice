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
  generateSymbols();
  preAllocateMatrixData();

  generateComponentConections();
  generateMatrices();
  bool doThing = false;
  if (doThing == true){
    A = {
      {{0.00010, -0.00010, 1.00000},
       {-0.00010, 0.00020, 0.0},
       {1.00000, 0.00000, 0.00000}},
      3, 3
    };

    E = {
      {{0, 0, 0},
       {0, 0, 0},
       {0, 0, 0}},
      3, 3
    };
    f = {
      {{0},
       {0},
       {5}
      },
      1, 3
    };

    initalValues = {
      {{5},
       {0},
       {0}
      },
      1, 3
    };
    syms = {
      {{symbol("e1")},
       {symbol("e2")},
       {symbol("GND")}
      },
      1, 3
    };
  }

  std::cout << "A:" << std::endl;
  A.print();
  std::cout << "E:" << std::endl;
  E.print();
  std::cout << "f:" << std::endl;
  f.print();
  std::cout << "Inital Values:" << std::endl;
  initalValues.print();
}


void Circuit::generateMatrices() {
  
  int equationNumber = 0; 
  for (auto node : nodes) {
    if (node->nodeName != "GND") {
      //A.print();
      for (auto c : node->components) {
        switch (c->Type) {
        case ComponentType::RESISTOR: {
          if (c->Connections.size() > 2) {
            std::cerr << "ERROR: Resistor " << c->ComponentName << " has too many connections" << std::endl;
          } else if (c->Connections.size() < 2) {
            std::cerr << "ERROR: Resistor " << c->ComponentName << " has not got enough connections" << std::endl;
          }
          int nodeLocation1 = findNodeLocationFromNode(c->Connections[0]);
          int nodeLocation2 = findNodeLocationFromNode(c->Connections[1]);
          auto resistor = dynamic_cast<Resistor*>(c.get());
          if (node == c->Connections[0]) {      
            A.data[equationNumber][nodeLocation1] +=  1/resistor->Resistance;
            A.data[equationNumber][nodeLocation2] += -1/resistor->Resistance;
          } else {
            A.data[equationNumber][nodeLocation1] -=  1/resistor->Resistance;
            A.data[equationNumber][nodeLocation2] -= -1/resistor->Resistance;
          }
        
          break;
        }
        case ComponentType::CAPACITOR: {
          if (c->Connections.size() > 2) {
            std::cerr << "ERROR: Capacitor " << c->ComponentName << " has too many connections" << std::endl;
          } else if (c->Connections.size() < 2) {
            std::cerr << "ERROR: Capacitor " << c->ComponentName << " has not got enough connections" << std::endl;
          }
          int nodeLocation1 = findNodeLocationFromNode(c->Connections[0]);
          int nodeLocation2 = findNodeLocationFromNode(c->Connections[1]);
          auto capacitor = dynamic_cast<Capacitor*>(c.get());

          if (node == c->Connections[0]) {
            E.data[equationNumber][nodeLocation1] +=  capacitor->Capacitance;
            E.data[equationNumber][nodeLocation2] += -capacitor->Capacitance;
          } else {
            E.data[equationNumber][nodeLocation1] -=  capacitor->Capacitance;
            E.data[equationNumber][nodeLocation2] -= -capacitor->Capacitance;
          }
          break;
        }
        case ComponentType::VOLTAGESOURCE: {
          // FIXME: deal with more than one node per voltage
          int nodeLocation1 = findNodeLocationFromNode(c->Connections[0]);
          int nodeLocationCurrent = findNodeLocationFromSymbol("i_" + c->ComponentName);
          auto voltageSource = dynamic_cast<VoltageSource*>(c.get());
          initalValues.data[nodeLocation1][0] += voltageSource->Voltage;
          // FIXME?: This might break on diff circuits
          A.data[equationNumber][nodeLocationCurrent] = 1;
          A.data[nodeLocationCurrent][equationNumber] = 1;
          f.data[nodeLocationCurrent][0] += voltageSource->Voltage;
          
          break;
        } // TODO: add other components
        default: {
          std::cerr << "ERROR: Component of type: " << c->Type << " and name: " << c->ComponentName << " was not handled" << std::endl;
        }}
      }
    } else { // node == GND
      int GNDLocation = findNodeLocationFromNode(node);
      A.data[equationNumber][GNDLocation] = 1;
    }
    equationNumber++;
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
  std::cerr << "ERROR: Location Not Found" << std::endl;
  return -1;
}

int Circuit::findNodeLocationFromSymbol(std::string symName) {
  for (int i = 0 ; i < syms.rows; i++) {
    for (int j = 0 ; j < syms.cols; j++) {
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
    //if (node->nodeName != "GND") {
    syms.data.push_back({symbol(node->nodeName)});
      //}
    if (node->nodeName == "GND") hasGround = true;
  }
  if (hasGround == false) {
    std::cerr << "ERROR: Circuit has no ground connection, please define a node with the name \"GND\"" << std::endl;
  }
  // FIXME: We need to deal with voltage sources in a more elegent way
  for (auto node : nodes) {
    for (auto c :node->components) {
      if (c->Type == ComponentType::VOLTAGESOURCE) {
        syms.data.push_back({symbol("i_" + c->ComponentName)});
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


