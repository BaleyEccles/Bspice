#pragma once
#include <sstream>
#include <fstream>
#include <memory>
#include <utility>
#include "circuit.h"


class token {
public:
  enum tokenType {
    TIME, 
    COMPONENT,
    NODE,
    PLOT,
    FOURIER,
    CALCULATE,
    DATA
  };
  
  token(tokenType type);
  virtual ~token() = default;
  
  tokenType type;
};

class timeToken : public token {
public:
  timeToken();
  
  std::string name = "time token";
  std::shared_ptr<token> stopTime, timeStep;
};

class componentToken : public token {
public:
  componentToken(Component::ComponentType componentType);
  
  std::string name;
  std::vector<double> values;
  
  Component::ComponentType componentType;
  VoltageSource::functionType fType; // For functions ie. AC, square wave etc
  
  std::shared_ptr<token> voltageDataToken, currentDataToken;
  std::shared_ptr<Component> circuitComponentPtr;
  
  inline void addValue(double componentValue) {values.push_back(componentValue);};
  inline void addValues(std::vector<double> componentValues) {for (auto& val : componentValues) {values.push_back(val); }};
};
  
class nodeToken : public token {
public:

  nodeToken();
  
  std::string name;
  
  std::vector<std::pair<std::shared_ptr<token>, Component::connectionType>> components;
  std::shared_ptr<token> voltageDataToken;
};

class plotToken : public token {
public:
  plotToken();
  
  std::string plotVaribleName;
  std::shared_ptr<token> dataToken;

};

class fourierToken : public token {
public:
  fourierToken();
  
  std::string name;
  std::shared_ptr<token> inputDataToken, outputDataToken;
};

class dataToken : public token {
public:
  dataToken(std::string nameIn);
  
  std::string name;
  std::vector<std::vector<double>> data;
  
  inline void addData(std::vector<double> dataIn) { data.push_back(dataIn); };
};

class calculateToken : public token {
public:
  enum calculateType {
    VOLTAGE = 0,
    CURRENT,
    ADD,
    SUBTRACT
  };
  
  calculateToken();
  
  std::string name;
  calculateToken::calculateType calculationType;
  std::vector<std::shared_ptr<token>> inputs;
  std::shared_ptr<token> output;
  std::vector<std::shared_ptr<token>> args;
  
  inline void addData(std::vector<double> dataIn) {
    auto dataT = dynamic_cast<dataToken *>(output.get());
    dataT->addData(dataIn);
  };
};

class fileParser {
public:
  fileParser(const std::string& filename);
  std::vector<std::shared_ptr<token>> tokens;
private:
  std::vector<std::shared_ptr<token>> tokenize(const std::string& line);
  
  std::vector<std::string> getInputs(const std::string &line);
  
  std::string getName(std::string name);
  double getValue(std::string &value);
  calculateToken::calculateType getCalculateType(std::string input);
  std::shared_ptr<token> getTokenPtrFromName(const std::string& argName);
  
  bool isDataDefined(std::string varName);
  bool isComponentDefined(std::string cName);
  
  void verifyCalculateArguments(calculateToken::calculateType type, std::vector<std::string> inputs);
  bool sourceIsFunction(std::vector<std::string> inputs);
  void checkIfComponentIsValid(std::shared_ptr<componentToken> component, std::vector<std::string> inputs);
  
  void createVoltageSource(std::shared_ptr<componentToken> component, std::vector<std::string> inputs);
  void createDiode(std::shared_ptr<componentToken> component, std::vector<std::string> inputs);

  void addTime(const std::string &line);
  void addComponent(const std::string &line);
  void addNode(const std::string &line);
  void addPlot(const std::string &line);
  void addFourier(const std::string &line);
  void addCalculate(const std::string &line);

  std::shared_ptr<componentToken> getComponent(const std::string &line);
  std::shared_ptr<dataToken> getData(std::string name);
  
  bool tokenIsTime(std::string token);
  bool tokenIsComponent(std::string token);
  bool tokenIsNode(std::string token);
  bool tokenIsPlot(std::string token);
  bool tokenIsFourier(std::string token);
  bool tokenIsCalculate(std::string token);
};
