#include "tokenParser.h"

Circuit createCircuitFromTokens(std::vector<std::shared_ptr<fileParser::token>> tokens) {
  Circuit circuit;
  for (auto& token : tokens) {
    if (token->type == fileParser::token::NODE) {
      auto nodeToken = dynamic_cast<fileParser::nodeToken *>(token.get());
      //std::cout << "processing node " << nodeToken->name << std::endl;
      Node *node = new Node(nodeToken->name);
      for (auto& component : nodeToken->components) {
        auto componentToken = dynamic_cast<fileParser::componentToken *>(component.get());
        switch(componentToken->componentType){
        case ComponentType::VOLTAGESOURCE: {
          auto c = std::make_shared<VoltageSource>(componentToken->name, componentToken->value);
          node->addComponent(c);
          break;
        }
        case ComponentType::RESISTOR: {
          auto c = std::make_shared<Resistor>(componentToken->name, componentToken->value);
          node->addComponent(c);
          break;
        }
        case ComponentType::CAPACITOR: {
          auto c = std::make_shared<Capacitor>(componentToken->name, componentToken->value);
          node->addComponent(c);
          break;
        }
        case ComponentType::INDUCTOR: {
          auto c = std::make_shared<Inductor>(componentToken->name, componentToken->value);
          node->addComponent(c);
          break;
        }
        default: {
          std::cerr << "ERROR: Component type not handled" << std::endl;
        }
        }
        
      }
      circuit.addNode(node);
    }
    
  }
  return circuit;
}


postProcess::postProcess(const std::string& octaveFileName, std::vector<double> &time, std::vector<matrix<double>> &data, matrix<symbol> &syms, std::vector<std::shared_ptr<fileParser::token>> &tokens)
  : fileName(octaveFileName), time(time), data(data), syms(syms), tokens(tokens) {

  file.open(fileName);

  if (!file.is_open()) {
    std::cerr << "Error opening file" << std::endl;
    return;
  }
  createOctavePlotFileFromTokens();
  file.close();
};
postProcess::~postProcess() {
}
  
void postProcess::addOctaveVarible(const std::string &name, std::vector<double> &plotData) {
  file << (name + " = [");
  for (int i = 0; i < plotData.size(); i++) {
    file << std::scientific << std::setprecision(5) << plotData[i] << " ";
  }
  file << ("];\n");
};

void postProcess::addPlot(const std::string &name, std::vector<double> &plotData) {
  //std::cout << name << " " << plotData.size() << std::endl;
  addOctaveVarible(name, plotData);
  file << ("figure();\n");
  file << ("plot(t, " + name + ");\n");
  file << ("xlabel(\"t\");\n");
  file << ("ylabel(\"" + name + "\");\n");
};

void postProcess::createOctavePlotFileFromTokens() {
  file << "graphics_toolkit(\"gnuplot\")\n";
  file << "set(0, 'DefaultTextFontSize', 25);";
  file << "set(0, 'DefaultAxesFontSize', 25);";
  addOctaveVarible("t", time);
  
  for (auto& token : tokens) {
    if (token->type == fileParser::token::PLOT) {
      auto plotToken = dynamic_cast<fileParser::plotToken *>(token.get());
      bool isValidPlot = false;
      for (int row = 0; row < syms.rows; row++) {
        if (syms.data[row][0].name == plotToken->name) {
          std::vector<double> plotData;
          for (auto& d : data) {
            plotData.push_back(d.data[row][0]);
          }
          addPlot(plotToken->name, plotData);
          isValidPlot = true;
        }
      }
      if (!isValidPlot) {
        std::cerr << "ERROR: `" << plotToken->name << "` was not able to be plotted." << std::endl;
      }

    }
  }
  file << ("pause;");

};


