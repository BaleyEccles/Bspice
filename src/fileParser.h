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
      KEY_WORD = 0,
      COMPONENT,
      NODE,
      PLOT
    };
  
    token(tokenType type);
  
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
    keyWordToken(KeyWordType keyWordType);
    KeyWordType keyWordType;

  };
  
  class componentToken : public token {
  public:
    componentToken(ComponentType componentType);
    std::string name;
    double value;
    ComponentType componentType;
    inline void addName(std::string componentName) {name = componentName;};
    inline void addValue(double componentValue) {value = componentValue;};
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
  double getValue(const std::string &value);

  
  void addComponent(std::vector<std::shared_ptr<token>>& tokens, const std::string &line);
  void addNode(std::vector<std::shared_ptr<token>>& tokens, const std::string &line);
  void addPlot(std::vector<std::shared_ptr<token>>& tokens, const std::string &line);
  void addKeyWord(std::vector<std::shared_ptr<token>>& tokens, const std::string &line);
    
  std::shared_ptr<plotToken> getPlot(const std::string &line);
  std::shared_ptr<nodeToken> getNode(const std::string &line);
  std::shared_ptr<componentToken> getComponent(const std::string &line);
  std::shared_ptr<keyWordToken> getKeyWord(const std::string &line);
  

  bool tokenIsComponent(std::string token);
  bool tokenIsKeyWord(std::string token);
  bool tokenIsNode(std::string token);
  bool tokenIsPlot(std::string token);
  
  
}
