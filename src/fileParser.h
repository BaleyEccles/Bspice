#pragma once
#include <sstream>
#include <fstream>
#include <regex>
#include <memory>
#include "Circuit.h"

namespace fileParser {
  class token;
  std::vector<std::shared_ptr<token>> parseFile(const std::string& filename);

  class token {
  public:
    enum tokenType {
      COMPONENT = 0,
      NODE,
      PLOT
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
    inline void addName(std::string componentName) {name = componentName;};
    inline void setFunctionType(VoltageSource::functionType type) {fType = type;};
    inline void addValue(double componentValue) {values.push_back(componentValue);};
    inline void addValues(std::vector<double> componentValues) {for (auto& val : componentValues) {values.push_back(val); }};

  };
  
  class nodeToken : public token {
  public:
    nodeToken();
    std::string name;
    std::vector<std::shared_ptr<token>> components;
    inline void addName(std::string componentName) {name = componentName;};
    inline void addComponents(std::vector<std::shared_ptr<token>> connectedComponents) {components = connectedComponents;};
  };

  class plotToken : public token {
  public:
    plotToken();
    std::string name;
    inline void addPlot(std::string plotName) {name = plotName;};
  };
  std::vector<std::shared_ptr<token>> tokenize(const std::string& line, std::vector<std::shared_ptr<token>> tokens);

  
  std::vector<std::string> getInputs(const std::string &line);
  
  std::string getName(const std::string &name);
  double getValue(std::string &value);

  
  void addComponent(std::vector<std::shared_ptr<token>>& tokens, const std::string &line);
  void addNode(std::vector<std::shared_ptr<token>>& tokens, const std::string &line);
  void addPlot(std::vector<std::shared_ptr<token>>& tokens, const std::string &line);
    
  std::shared_ptr<plotToken> getPlot(const std::string &line);
  std::shared_ptr<nodeToken> getNode(const std::string &line);
  std::shared_ptr<componentToken> getComponent(const std::string &line);
  

  bool tokenIsComponent(std::string token);
  bool tokenIsNode(std::string token);
  bool tokenIsPlot(std::string token);
  
  
}
