#pragma once
#include <sstream>
#include <fstream>
#include <memory>
#include <utility>
#include "Circuit.h"


class token {
public:
  enum tokenType {
    COMPONENT = 0,
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

class componentToken : public token {
public:
  componentToken(ComponentType componentType);
  std::string name;
  std::vector<double> values;
  ComponentType componentType;
  VoltageSource::functionType fType; // For functions ie. AC, square wave etc
  std::shared_ptr<token> voltageDataToken, currentDataToken;
  std::shared_ptr<Component> circuitComponentPtr;
  inline void addName(std::string componentName) {name = componentName;};
  inline void setFunctionType(VoltageSource::functionType type) {fType = type;};
  inline void addValue(double componentValue) {values.push_back(componentValue);};
  inline void addValues(std::vector<double> componentValues) {for (auto& val : componentValues) {values.push_back(val); }};

};
  
class nodeToken : public token {
public:
  enum connectionType {
    UNDEFINED = 0,
    DIODE_P,
    DIODE_N,
    BJT_BASE,
    BJT_COLLECTOR,
    BJT_EMITTER,
  };
  nodeToken();
  std::string name;
  std::vector<std::pair<std::shared_ptr<token>, nodeToken::connectionType>> components;
  std::shared_ptr<token> voltageDataToken;
  inline void addName(std::string componentName) {name = componentName;};
};

class plotToken : public token {
public:
  plotToken();
  std::string name;
  std::shared_ptr<token> dataToken;
  inline void addPlot(std::string plotName) {name = plotName;};

};
class fourierToken : public token {
public:
  fourierToken();
  std::string name;
  std::shared_ptr<token> inputDataToken, outputDataToken;
  inline void addFourier(std::string name) { name = name; };
};

class dataToken : public token {
public:
  dataToken(std::string nameIn);
  std::string name;
  std::vector<std::vector<double>> data;
  inline void addData(std::vector<double> dataIn) { data.push_back(dataIn); };
  inline void addName(std::string nameIn) { name = nameIn; };
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
  calculateType cType;
  std::shared_ptr<token> output;
  std::vector<std::shared_ptr<token>> args;
  inline void addArgs(std::vector<std::shared_ptr<token>> argsIn) {args = argsIn;};
  inline void addName(std::string nameIn) { name = nameIn; };
  inline void addType(calculateType type) { cType = type; };
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
  bool isVaribleDefined(std::string varName);
  bool isComponentDefined(std::string cName);
  void verifyCalculateArguments(calculateToken::calculateType type, std::vector<std::string> inputs);
  std::shared_ptr<token> getTokenPtrFromName(const std::string& argName);
  bool sourceIsFunction(std::vector<std::string> inputs);
  void checkIfComponentIsValid(std::shared_ptr<componentToken> component, std::vector<std::string> inputs);
  void createVoltageSource(std::shared_ptr<componentToken> component, std::vector<std::string> inputs);
  void createDiode(std::shared_ptr<componentToken> component, std::vector<std::string> inputs);

  void addComponent(const std::string &line);
  void addNode(const std::string &line);
  void addPlot(const std::string &line);
  void addFourier(const std::string &line);
  void addCalculate(const std::string &line);
    
  std::shared_ptr<plotToken> getPlot(const std::string &line);
  std::shared_ptr<nodeToken> getNode(const std::string &line);
  std::shared_ptr<fourierToken> getFourier(const std::string &line);
  std::shared_ptr<componentToken> getComponent(const std::string &line);
  std::shared_ptr<calculateToken> getCalculate(const std::string &line);
  std::shared_ptr<dataToken> getData(std::string name);
  

  bool tokenIsComponent(std::string token);
  bool tokenIsNode(std::string token);
  bool tokenIsPlot(std::string token);
  bool tokenIsFourier(std::string token);
  bool tokenIsCalculate(std::string token);
};
