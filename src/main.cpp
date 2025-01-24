#include "Bmaths/Bmaths.h"
#include "Circuit.h"
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <memory>
#include <string>
#include <vector>

void createOctavePlotFile(std::vector<double> &time,
                          std::vector<matrix<double>> &data,
                          matrix<symbol> &names) {
  if (data.size() != time.size()) {
    std::cerr << "Time and data have different sizes" << std::endl;
  }

  std::string filename = "plotData.m";
  std::ofstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error opening file" << std::endl;
    return;
  }
  file << "graphics_toolkit(\"gnuplot\")\n";
  file << "set(0, 'DefaultTextFontSize', 25);";
  file << "set(0, 'DefaultAxesFontSize', 25);";
  file << ("t = [");
  for (int i = 0; i < data.size(); i++) {
    file << std::scientific << std::setprecision(5) << time[i] << " ";
  }
  for (int j = 0; j < names.rows; j++) {
    file << ("];\n" + names.data[j][0].name + " = [");
    for (int i = 0; i < data.size(); i++) {
      file << std::scientific << std::setprecision(5) << data[i].data[j][0]
           << " ";
    }
  }
  file << ("];\n");

  for (int j = 0; j < names.rows; j++) {
    file << ("figure(" + std::to_string(j + 1) + ");\n");
    file << ("plot(t, " + names.data[j][0].name + ");\n");
    file << ("xlabel(\"t\");\n");
    file << ("ylabel(\"" + names.data[j][0].name + "\");\n");
  }

  /// Print voltage over component
  // file << ("figure(" + std::to_string(names.rows + 1) + ");\n");
  // file << ("plot(t, e2 - e3)\n");
  // file << ("xlabel \"t\");");
  // file << ("ylabel (\"e2-e3\");");

  file << ("pause;");
  file.close();
};


class token {
public:
  enum tokenType {
    KEY_WORD = 0,
    COMPONENT,
    NODE,
    PLOT
  };
  
  token(tokenType type)
    :type(type) {};
  
  virtual ~token() = default;
  tokenType type;
};

class keyWordToken : public token {
public:
  enum KeyWordType {
    COMPONENTS,
    CIRCUIT,
    POST_PROCESS
  };
  keyWordToken(KeyWordType keyWordType)
    : token(token::KEY_WORD), keyWordType(keyWordType) {};

  KeyWordType keyWordType;

};

class componentToken : public token {
public:
  componentToken(ComponentType componentType)
    : token(token::COMPONENT), componentType(componentType) {};
  std::string name;
  double value;
  ComponentType componentType;
  inline void addName(std::string componentName) {name = componentName;};
  inline void addValue(double componentValue) {value = componentValue;};
};

class nodeToken : public token {
public:
  nodeToken()
    : token(token::NODE) {};
  std::string name;
  std::vector<std::string> components;
  inline void addName(std::string componentName) {name = componentName;};
  inline void addComponents(std::vector<std::string> connectedComponents) {components = connectedComponents;};
};

class plotToken : public token {
public:
  plotToken()
    : token(token::PLOT){};
  std::string name;
  inline void addPlot(std::string plotName) {name = plotName;};
};



std::vector<std::string> getInputs(const std::string &line) {
  //std::cout << line << std::endl;
  std::regex regexPattern(R"(\{.*?\})");
  std::vector<std::string> results;

  auto begin = std::sregex_iterator(line.begin(), line.end(), regexPattern);
  auto end = std::sregex_iterator();

  for (std::sregex_iterator i = begin; i != end; ++i) {
    results.push_back((*i)[0]);
  }
  
  // Remove curly braces
  for (auto& str : results) {
    str.erase(0, 1);
    str.pop_back();
  }
  
  //std::cout << "Extracted values:" << std::endl;
  //for (const auto& value : results) {
  //std::cout << value << std::endl;
  //}

  return results;

}

std::string getNameToken(const std::string &name) {
  // FIXME: Deal with characters that break things ie spaces, curly braces etc.
  return name;
}

double getValueToken(const std::string &value) {
  // FIXME: Deal with scientific notation and error handiling
  return std::stod(value);
}

std::shared_ptr<componentToken> getComponent(const std::string &line) {
  std::string resistor = "resistor";
  if (line.substr(0, resistor.size()) == resistor) {
    return std::make_shared<componentToken>(RESISTOR);
  }
  std::string capacitor = "capacitor";
  if (line.substr(0, capacitor.size()) == capacitor) {
    return std::make_shared<componentToken>(CAPACITOR);
  }
  std::string inductor = "inductor";
  if (line.substr(0, inductor.size()) == inductor) {
    return std::make_shared<componentToken>(INDUCTOR);
  }
  std::string voltageSource = "voltage_source";
  if (line.substr(0, voltageSource.size()) == voltageSource) {
    return std::make_shared<componentToken>(VOLTAGESOURCE);
  }
  std::string currentSource = "current_source";
  if (line.substr(0, currentSource.size()) == currentSource) {
    return std::make_shared<componentToken>(CURRENTSOURCE);
  }
  return 0;
}

void addComponent(std::vector<std::shared_ptr<token>>& tokens, const std::string &line) {
  std::shared_ptr<componentToken> component = getComponent(line);
  std::vector<std::string> inputs = getInputs(line);
  if (inputs.size() != 2) {
    // FIXME: When we add opams, bjts, etc this must change
    std::cerr << "ERROR: Components must have two inputs." << std::endl;
    std::cerr << "\tEX: COMPONENT{NAME}{VALUE}" << std::endl;
  }
  std::string name = getNameToken(inputs[0]);
  double value = getValueToken(inputs[1]);
  component->addName(name);
  component->addValue(value);
  tokens.push_back(component);
}

std::shared_ptr<nodeToken> getNode(const std::string &line) {
  return std::make_shared<nodeToken>();
}

void addNode(std::vector<std::shared_ptr<token>>& tokens, const std::string &line) {
  std::shared_ptr<nodeToken> node = getNode(line);
  std::vector<std::string> inputs = getInputs(line);
  if (inputs.size() == 0) {
    std::cerr << "ERROR: Nodes must have N + 1 inputs." << std::endl;
    std::cerr << "\tEX: node{NAME}{COMPONENT_1}{COMPONENT_2}...{COMPONENT_N}" << std::endl;
  }
  std::string name = getNameToken(inputs[0]);
  node->addName(name);
  std::vector<std::string> components(inputs.begin() + 1, inputs.end());
  node->addComponents(components);
  tokens.push_back(node);
}


std::shared_ptr<plotToken> getPlot(const std::string &line) {
  return std::make_shared<plotToken>();
}
void addPlot(std::vector<std::shared_ptr<token>>& tokens, const std::string &line) {
  std::shared_ptr<plotToken> plot = getPlot(line);
  std::vector<std::string> inputs = getInputs(line);
  if (inputs.size() != 1) {
    std::cerr << "ERROR: Plots must have 1 input." << std::endl;
    std::cerr << "\tEX: plot{NODE_NAME}" << std::endl;
  }
  // FIXME: Verify that toPlot is a valid voltage/current/node etc
  std::string toPlot = getNameToken(inputs[0]);
  plot->addPlot(toPlot);
  tokens.push_back(plot);
}

std::shared_ptr<keyWordToken> getKeyWord(const std::string &line) {
  std::string components = "Components:";
  if (line.substr(0, components.size()) == components) {
    return std::make_shared<keyWordToken>(keyWordToken::COMPONENTS);
  }
  std::string circuit = "Circuit:";
  if (line.substr(0, circuit.size()) == circuit) {
    return std::make_shared<keyWordToken>(keyWordToken::CIRCUIT);
  }
  std::string postProcess = "Post_Process:";
  if (line.substr(0, postProcess.size()) == postProcess) {
    return std::make_shared<keyWordToken>(keyWordToken::POST_PROCESS);
  }
  return 0;
}

void addKeyWord(std::vector<std::shared_ptr<token>>& tokens, const std::string &line) {
  std::shared_ptr<keyWordToken> keyWord = getKeyWord(line);
  tokens.push_back(keyWord);
}

bool tokenIsComponent(std::string token) {
  // FIXME: make this more elegent 
  return token == "resistor" || token == "capacitor" || token == "voltage_source" || token == "inductor";
}

bool tokenIsKeyWord(std::string token) {
  // FIXME: make this more elegent 
  return token == "Components:" || token == "Circuit:" || token == "Post_Process:";
}

bool tokenIsNode(std::string token) {
  // FIXME: make this more elegent 
  return token == "node";
}

bool tokenIsPlot(std::string token) {
  // FIXME: make this more elegent 
  return token == "plot";
}

std::vector<std::shared_ptr<token>> tokenize(const std::string& line) {
  std::istringstream stream(line);
  std::string currentToken;
  std::vector<std::shared_ptr<token>> tokens;
  for (char c : line) {
    currentToken = currentToken + c;
    if (tokenIsKeyWord(currentToken)) {
      addKeyWord(tokens, line);
    } else if (tokenIsComponent(currentToken)) {
      addComponent(tokens, line);
    } else if (tokenIsNode(currentToken)) {
      addNode(tokens, line);
    } else if (tokenIsPlot(currentToken)) {
      addPlot(tokens, line);
    }
  }

  return tokens;
}

int main() {
  std::ifstream file("test.circ");
  std::string line;
  std::vector<std::shared_ptr<token>> tokens;
  if (file.is_open()) {
    while (std::getline(file, line)) {
      auto lineTokens = tokenize(line);
      tokens.insert(tokens.end(), lineTokens.begin(), lineTokens.end());
    }
    file.close();
  } else {
    std::cerr << "Unable to open file" << std::endl;
  }
  
  if (false) {
    Node *Node1 = new Node("e1");
    Node *Node2 = new Node("e2");
    Node *Node3 = new Node("e3");
    Node *Node4 = new Node("e4");
    Node *GND = new Node("GND");

    auto Vcc = std::make_shared<VoltageSource>("Vcc", 5.0);
    auto R1 = std::make_shared<Resistor>("R1", 10);
    auto L1 = std::make_shared<Inductor>("L1", 1);

    Node1->addComponent(Vcc);
    Node1->addComponent(R1);

    Node2->addComponent(R1);
    Node2->addComponent(L1);

    GND->addComponent(L1);

    std::cout << "From Circ" << std::endl;
    Circuit circuit;
    circuit.addNode(Node1);
    circuit.addNode(Node2);
    circuit.addNode(GND);
    circuit.calculate();

    auto initalValues = circuit.initalValues;
    auto A = circuit.A;
    auto E = circuit.E;
    auto f = circuit.f;
    auto s = circuit.syms;

    double timeStep = 0.001;
    double endTime = 1.0;
    auto output = DAESolve(A, E, f, initalValues, timeStep, endTime);
    createOctavePlotFile(output.first, output.second, s);
    delete Node1;
    delete Node2;
    delete GND;
    return 0;
  }
};
