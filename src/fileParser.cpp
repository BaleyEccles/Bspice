#include "fileParser.h"

namespace fileParser {
  std::vector<std::shared_ptr<token>> parseFile(const std::string& filename ) {
    std::ifstream file(filename);
    std::string line;
    std::vector<std::shared_ptr<token>> tokens;
    if (file.is_open()) {
      while (std::getline(file, line)) {
        auto lineTokens = tokenize(line, tokens);
        tokens = lineTokens;
      }
      file.close();
    } else {
      std::cerr << "Unable to open file" << std::endl;
    }
    //std::cout << tokens.size() << std::endl;
    return tokens;
  }

  token::token(tokenType type)
    :type(type) {};
  
  componentToken::componentToken(ComponentType componentType)
    : token(token::COMPONENT), componentType(componentType) {};

  nodeToken::nodeToken()
    : token(token::NODE) {};

  plotToken::plotToken()
    : token(token::PLOT){};

  fourierToken::fourierToken()
    : token(token::FOURIER){};


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

  std::string getName(const std::string &name) {
    // FIXME: Deal with characters that break things ie spaces, curly braces etc.
    return name;
  }


  // only supports the format NUMBER SCIENTIFC_NOTATION for now
  double getValue(std::string &value) {
    auto prefix = value.back();
    if (std::isalpha(prefix)) {
      switch (prefix) {
      case 'M': {
        value.pop_back();
        value += "e6";
        break;
      }
      case 'k': {
        value.pop_back();
        value += "e3";
        break;
      }
      case 'm': {
        value.pop_back();
        value += "e-3";
        break;
      }
      case 'u': {
        value.pop_back();
        value += "e-6";
        break;
      }
      case 'n': {
        value.pop_back();
        value += "e-9";
        break;
      }
      case 'p': {
        value.pop_back();
        value += "e-12";
        break;
      }
      case 'f': {
        value.pop_back();
        value += "e-15";
        break;
      }
      default: {
        std::cerr << "ERROR: The prefix `" << prefix << "` was not handled" << std::endl;
      }
      }
    }
    auto number = std::stod(value);
    //std::cout << number << std::endl;
    return number;
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
      auto inputs = getInputs(line);
      return std::make_shared<componentToken>(VOLTAGESOURCE);
    }
    std::string currentSource = "current_source";
    if (line.substr(0, currentSource.size()) == currentSource) {
      return std::make_shared<componentToken>(CURRENTSOURCE);
    }
    std::cerr << "ERROR: Component not found" << std::endl;
    return 0;
  }

  void addComponent(std::vector<std::shared_ptr<token>>& tokens, const std::string &line) {
    std::shared_ptr<componentToken> component = getComponent(line);

    std::vector<std::string> inputs = getInputs(line);
    if (component->componentType != VOLTAGESOURCE && inputs.size() != 2) {
      // FIXME: When we add opams, bjts, etc this must change
      std::cerr << "ERROR: Components must have two inputs." << std::endl;
      std::cerr << "\tEX: COMPONENT{NAME}{VALUE}" << std::endl;
    } // TODO: Error handling for AC/sqaure wave signals
    if (component->componentType == VOLTAGESOURCE && inputs[1] == "AC") {
      std::string name = getName(inputs[0]);
      std::cout << "NAME: " <<  name << std::endl;
      double amplitude = getValue(inputs[2]);
      double frequency = getValue(inputs[3]);
      double shift = getValue(inputs[4]);
      component->setFunctionType(VoltageSource::AC);
      component->addName(name);
      component->addValues({amplitude, frequency, shift});
      tokens.push_back(component);
    } else {
      std::string name = getName(inputs[0]);
      double value = getValue(inputs[1]);
      component->setFunctionType(VoltageSource::NONE);
      component->addName(name);
      component->addValue(value);
      tokens.push_back(component);
    }
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
    std::string nodeName = getName(inputs[0]);
    node->addName(nodeName);
    std::vector<std::string> componentNames(inputs.begin() + 1, inputs.end());


    for (auto& name : componentNames) {
      auto it = std::find_if(tokens.begin(), tokens.end(), [&name](const std::shared_ptr<fileParser::token>& t) {
        if (t->type == fileParser::token::COMPONENT) {
          auto componentToken = dynamic_cast<fileParser::componentToken*>(t.get());
          return componentToken && componentToken->name == name;
        }
        return false;
      });

  
      if (it == tokens.end()) {
        std::cerr << "ERROR: Component `" << name << "` not defined." << std::endl;
      }
    }

      std::vector<std::shared_ptr<token>> componentTokens;
    //std::cout << tokens.size() << std::endl;
    for (auto& token : tokens) {
      if (token->type == fileParser::token::COMPONENT) {
        auto componentToken = dynamic_cast<fileParser::componentToken *>(token.get());
        auto itComponent = std::find(componentNames.begin(), componentNames.end(), componentToken->name);
        auto itDuplicate = std::find(componentTokens.begin(), componentTokens.end(), token);
        if(itComponent != componentNames.end() && itDuplicate == componentTokens.end()) {
          componentTokens.push_back(token);
          //std::cout << nodeName << " contains the component " << componentToken->name << std::endl;
        }
      }
    }

    
    node->addComponents(componentTokens);
    tokens.push_back(node);
  }

  std::shared_ptr<fourierToken> getFourier(const std::string &line) {
    return std::make_shared<fourierToken>();
  }

  void addFourier(std::vector<std::shared_ptr<token>>& tokens, const std::string &line) {
    std::shared_ptr<fourierToken> fourier = getFourier(line);
    std::vector<std::string> inputs = getInputs(line);
    if (inputs.size() != 2) {
      std::cerr << "ERROR: fourier transforms must have 2 input." << std::endl;
      std::cerr << "\tEX: fourier_transform{VARIBLE_NAME}{TRANSFORM_VARIBLE}" << std::endl;
    }
    std::string varibleName = getName(inputs[0]);
    // FIXME: Verify that toTransform is a valid voltage/current/node etc
    std::string toTransform = getName(inputs[1]);
    fourier->addFourier(varibleName, toTransform);
    tokens.push_back(fourier);
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
    std::string toPlot = getName(inputs[0]);
    plot->addPlot(toPlot);
    tokens.push_back(plot);
  }


  bool tokenIsComponent(std::string token) {
    // FIXME: make this more elegent 
    return token == "resistor" || token == "capacitor" || token == "voltage_source" || token == "inductor";
  }

  bool tokenIsNode(std::string token) {
    // FIXME: make this more elegent 
    return token == "node";
  }

  bool tokenIsPlot(std::string token) {
    // FIXME: make this more elegent 
    return token == "plot";
  }
  bool tokenIsFourier(std::string token) {
    // FIXME: make this more elegent 
    return token == "fourier_transform";
  }  

  std::vector<std::shared_ptr<token>> tokenize(const std::string& line, std::vector<std::shared_ptr<token>> tokens) {
    std::istringstream stream(line);
    std::string currentToken;
    for (char c : line) {
      currentToken = currentToken + c;
      if (tokenIsComponent(currentToken)) {
        addComponent(tokens, line);
      } else if (tokenIsNode(currentToken)) {
        addNode(tokens, line);
      } else if (tokenIsPlot(currentToken)) {
        addPlot(tokens, line);
      } else if (tokenIsFourier(currentToken)) {
        addFourier(tokens, line);
      }
    }

    return tokens;
  }

}
