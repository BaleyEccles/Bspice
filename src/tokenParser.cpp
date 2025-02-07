#include "tokenParser.h"
#include "Circuit.h"
#include "fileParser.h"
#include <memory>

// data is in a weird format
// it is a vector of matrix that is similar to syms
postProcess::postProcess(const std::string& octaveFileName, std::vector<double> &time, std::vector<matrix<double>> &data, matrix<symbol> &syms, std::vector<std::shared_ptr<token>> &tokens)
  : fileName(octaveFileName), time(time), data(data), syms(syms), tokens(tokens) {

  file.open(fileName);

  if (!file.is_open()) {
    std::cerr << "Error opening file" << std::endl;
    return;
  }
  fillInDataTokens();
  createOctavePlotFileFromTokens();
  file.close();
};

postProcess::~postProcess() {
}

void postProcess::fillInDataTokens() {
  for (auto& token : tokens) {
    switch (token->type) {
    case token::NODE:{
      auto t = dynamic_cast<nodeToken *>(token.get());
      auto d = dynamic_cast<dataToken *>(t->voltageDataToken.get());
      int idx = findIdxFromName(d->name);
      if (idx >= 0) {
        d->data = {data[idx].data[0]};
      }
      
      break;
    }
    case token::COMPONENT:{
      auto t = dynamic_cast<componentToken *>(token.get());
      auto d1 = dynamic_cast<dataToken *>(t->voltageDataToken.get());
      int idx1 = findIdxFromName(d1->name);
      if (idx1 >= 0) {
        d1->data = {data[idx1].data[0]};
      }

      auto d2 = dynamic_cast<dataToken *>(t->currentDataToken.get());
      int idx2 = findIdxFromName(d2->name);
      if (idx2 >= 0) {
        d2->data = {data[idx2].data[0]};
      }

      break;
    }
    case token::PLOT: {
      auto t = dynamic_cast<plotToken *>(token.get());
      auto d = dynamic_cast<dataToken *>(t->dataToken.get());
      int idx = findIdxFromName(d->name);
      if (idx >= 0) {
        d->data = {data[idx].data[0]};
      }
      break;
    }
    case token::CALCULATE:
    case token::FOURIER:
    case token::DATA: {
      break;
    }
    }
  }
};

int postProcess::findIdxFromName(std::string name) {
  for (int row = 0; row < syms.rows; row++) {
    for (int col = 0; col < syms.cols; col++) {
      if (syms.data[row][col].name == name) {
        return row;
      }
    }
  }
  //std::cerr << "ERROR: Could not find sym idx from name: " << name << std::endl;
  return -1;
}
  
void postProcess::addOctaveVarible(const std::string &name, std::vector<double> plotData) {
  file << (name + " = [");
  //std::cout << plotData.size() << std::endl;
  for (int i = 0; i < plotData.size(); i++) {
    file << std::scientific << std::setprecision(5) << plotData[i] << " ";
  }
  file << ("];\n");
};

void postProcess::addPlot(const std::string &name, std::vector<double> plotData) {
  //std::cout << name << " " << plotData.size() << std::endl;
  addOctaveVarible(name, plotData);
  file << ("figure();\n");
  file << ("plot(t, " + name + ");\n");
  file << ("xlabel(\"t\");\n");
  file << ("ylabel(\"" + name + "\");\n");
};

void postProcess::addFourierPlot(const std::string &name, std::vector<double> frequencyData, std::vector<double> magnitudeData) {// TODO: Deal with phase
  const std::string fName = "frequency_" + name;
  const std::string mName = "magnitude_" + name;
  const std::string pName = "phase_" + name;
  addOctaveVarible(fName, frequencyData);
  addOctaveVarible(mName, magnitudeData);  
  file << ("figure();\n");
  file << ("plot(" + fName + "," + mName + ");\n");
  file << ("xlabel(\"" + fName + "\");\n");
  file << ("ylabel(\"" + mName + "\");\n");
};

void postProcess::createOctavePlotFileFromTokens() {
  file << "graphics_toolkit(\"gnuplot\")\n";
  file << "set(0, 'DefaultTextFontSize', 25);";
  file << "set(0, 'DefaultAxesFontSize', 25);";
  addOctaveVarible("t", time);
  
  for (auto& token : tokens) {

    if (token->type == token::PLOT) {
      auto plotT = dynamic_cast<plotToken *>(token.get());
      bool isValidPlot = false;
      auto dataT = dynamic_cast<dataToken *>(plotT->dataToken.get());
      //for (auto i : dataT->data[0]) {
      //  std::cout << i << " ";
      //}
      addPlot(plotT->name, dataT->data[0]);
      if (dataT->data[0].size() == time.size()) {
        isValidPlot = true;
      }
      if (!isValidPlot) {
        std::cerr << "ERROR: `" << plotT->name << "` was not able to be plotted." << std::endl;
      }

    } else if (token->type == token::FOURIER) {
      auto fourierT = dynamic_cast<fourierToken *>(token.get());
      bool isValidPlot = false;
      auto inputDataT = dynamic_cast<dataToken *>(fourierT->inputDataToken.get());      
      auto outputDataT = dynamic_cast<dataToken *>(fourierT->outputDataToken.get());      
      matrix<double> toTransform = {{inputDataT->data}, (int)inputDataT->data[0].size(), 1};
      FourierTransform dft(time, toTransform);
      addFourierPlot(outputDataT->name, dft.frequency.data[0], dft.magnitudes.data[0]);
      outputDataT->addData(dft.frequency.data[0]);
      outputDataT->addData(dft.magnitudes.data[0]);
      outputDataT->addData(dft.phases.data[0]);
      isValidPlot = true;

      if (!isValidPlot) {
        std::cerr << "ERROR: `" << fourierT->name << "` was not able to be plotted." << std::endl;
      }

    } else if (token->type == token::CALCULATE) {
      auto calcT = dynamic_cast<calculateToken *>(token.get());
      switch (calcT->cType) {
      case calculateToken::VOLTAGE: {
        calcT->addData(calculateVoltage(calcT->args[0]));
        break;
      }
      case calculateToken::CURRENT: {
        calcT->addData(calculateCurrent(calcT->args[0]));
        break;
      }
      case calculateToken::ADD: {
        calcT->addData(calculateAdd(calcT->args[0], calcT->args[1]));
        break;
      }
      case calculateToken::SUBTRACT: {
        calcT->addData(calculateSubtract(calcT->args[0], calcT->args[1]));
        break;
      }
      default: {
        std::cerr << "ERROR: Calculation type not handled" << std::endl;
      }
      }

    }
  }
  file << ("pause;");

};

std::vector<double> postProcess::calculateCurrent(std::shared_ptr<token> t) {
  auto componentT = dynamic_cast<componentToken *>(t.get());
  switch (componentT->componentType) {
  case RESISTOR: { // use I = V/R
    std::vector<std::shared_ptr<token>> connectedNodes = getConnectedNodesFromComponentPtr(t);
    if (connectedNodes.size() != 2) {
      std::cerr << "ERROR: Voltage calculation failed, too many or not enough nodes." << std::endl;
    }
    auto voltage = calculateVoltage(t);
    
    std::vector<double> output(voltage.size(), 0.0);
    double resistance = dynamic_cast<Resistor *>(componentT->circuitComponentPtr.get())->Resistance;
    for (int i = 0; i < voltage.size(); i++) {
      output[i] = voltage[i]/resistance;
    }
    return output;
    break;
  }
  case CAPACITOR: { // use I = C*dV/dt
    std::vector<std::shared_ptr<token>> connectedNodes = getConnectedNodesFromComponentPtr(t);
    if (connectedNodes.size() != 2) {
      std::cerr << "ERROR: Voltage calculation failed, too many or not enough nodes." << std::endl;
    }
    auto voltage = calculateVoltage(t);
    auto d_voltage_dt = differentiateVector(time, voltage);
    
    std::vector<double> output(voltage.size(), 0.0);
    double capacitance = dynamic_cast<Capacitor *>(componentT->circuitComponentPtr.get())->Capacitance;
    for (int i = 0; i < voltage.size(); i++) {
      output[i] = capacitance*d_voltage_dt[i];
    }
    return output;
    break;
  }
  default: {
    std::cerr << "ERROR: Component type not handled for calculation" << std::endl;
  }
  }
  std::cerr << "ERROR: Component type not handled for calculation" << std::endl;
  return std::vector<double>();
}

std::vector<double> postProcess::calculateSubtract(std::shared_ptr<token> t1, std::shared_ptr<token> t2) {
    auto var1T = dynamic_cast<dataToken *>(t1.get());
  auto var2T = dynamic_cast<dataToken *>(t2.get());
  
  auto v1 = var1T->data[0];
  auto v2 = var2T->data[0];
  if (v1.size() != v2.size()) {
    std::cerr << "ERROR: For some reason varibles have different sizes?" << std::endl;
  }
  std::vector<double> output(v1.size(), 0.0);
  for (int i = 0; i < v1.size(); i++) {
    output[i] = v1[i] - v2[i];
  }
  return output;
};
 
std::vector<double> postProcess::calculateAdd(std::shared_ptr<token> t1, std::shared_ptr<token> t2) {
  auto var1T = dynamic_cast<dataToken *>(t1.get());
  auto var2T = dynamic_cast<dataToken *>(t2.get());
  
  auto v1 = var1T->data[0];
  auto v2 = var2T->data[0];
  if (v1.size() != v2.size()) {
    std::cerr << "ERROR: For some reason varibles have different sizes?" << std::endl;
  }
  std::vector<double> output(v1.size(), 0.0);
  for (int i = 0; i < v1.size(); i++) {
    output[i] = v1[i] + v2[i];
  }
  return output;

}

std::vector<double> postProcess::calculateVoltage(std::shared_ptr<token> t) {
  auto componentT = dynamic_cast<componentToken *>(t.get());
  switch (componentT->componentType) {
  case CAPACITOR:
  case RESISTOR: { // use V = node1 - node2
    std::vector<std::shared_ptr<token>> connectedNodes = getConnectedNodesFromComponentPtr(t);
    if (connectedNodes.size() != 2) {
      std::cerr << "ERROR: Voltage calculation failed, too many or not enough nodes." << std::endl;
    }
    auto v1 = getNodeDataFromName(connectedNodes[0]);
    auto v2 = getNodeDataFromName(connectedNodes[1]);
    if (v1.size() != v2.size()) {
      std::cerr << "ERROR: For some reason voltages have different sizes?" << std::endl;
    }
    std::vector<double> output(v1.size(), 0.0);
    for (int i = 0; i < v1.size(); i++) {
      output[i] = v1[i] - v2[i];
    }
    return output;
    break;
  }

  default: {
    std::cerr << "ERROR: Component type not handled for calculation" << std::endl;
  }
  }
  std::cerr << "ERROR: Component type not handled for calculation" << std::endl;
  return std::vector<double>();
}

std::vector<std::shared_ptr<token>> postProcess::getConnectedNodesFromComponentPtr(std::shared_ptr<token> component) {
  auto componentT = dynamic_cast<componentToken *>(component.get());
  std::vector<std::shared_ptr<token>> output;
  for (auto& t : tokens) {
    if (t->type == token::NODE) {
      auto nodeT = dynamic_cast<nodeToken *>(t.get());
      for (auto& c : nodeT->components) {
        auto cToken = dynamic_cast<componentToken *>(c.get());
        if (cToken->name == componentT->name) {
          output.push_back(t);
        }
      }
    }
  }
  return output;
}

std::vector<double> postProcess::getNodeDataFromName(std::shared_ptr<token> t) {
  auto nodeT = dynamic_cast<nodeToken *>(t.get());
  for (int row = 0; row < syms.rows; row++) {
    for (int col = 0; col < syms.cols; col++) {
      if (syms.data[row][col].name == nodeT->name) {
        return data[row].data[0];
      }
    }
  }
  std::cerr << "ERROR: Unable to find node data." << std::endl;
  return std::vector<double>();
}



