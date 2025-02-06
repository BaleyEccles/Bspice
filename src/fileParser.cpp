#include "fileParser.h"




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

calculateToken::calculateToken()
  : token(token::CALCULATE){};

dataToken::dataToken(std::string nameIn)
  : token(token::DATA), name(nameIn){};



fileParser::fileParser(const std::string& filename) {
  std::ifstream file(filename);
  std::string line;
  if (file.is_open()) {
    while (std::getline(file, line)) {
      auto lineTokens = tokenize(line);
      tokens = lineTokens;
    }
    file.close();
  } else {
    std::cerr << "Unable to open file" << std::endl;
  }
  //std::cout << tokens.size() << std::endl;
}

std::vector<std::shared_ptr<token>> fileParser::tokenize(const std::string& line) {
  std::istringstream stream(line);
  std::string currentToken;
  for (char c : line) {
    currentToken = currentToken + c;
    if (tokenIsComponent(currentToken)) {
      addComponent(line);
    } else if (tokenIsNode(currentToken)) {
      addNode(line);
    } else if (tokenIsPlot(currentToken)) {
      addPlot(line);
    } else if (tokenIsFourier(currentToken)) {
      addFourier(line);
    } else if (tokenIsCalculate(currentToken)) {
      addCalculate(line);
    }
  }

  return tokens;
}


std::vector<std::string> fileParser::getInputs(const std::string &line) {
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

std::string fileParser::getName(const std::string &name) {
  // FIXME: Deal with characters that break things ie spaces, curly braces etc.
  return name;
}

// only supports the format NUMBER SCIENTIFC_NOTATION for now
double fileParser::getValue(std::string &value) {
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


calculateToken::calculateType fileParser::getCalculateType(std::string input) {
  if (input == "VOLTAGE") {
    return calculateToken::VOLTAGE;
  } else  if (input == "CURRENT") {
    return calculateToken::CURRENT;
  } else  if (input == "ADD") {
    return calculateToken::ADD;
  } else  if (input == "SUBTRACT") {
    return calculateToken::SUBTRACT;
  }
  std::cerr << "ERROR: Calculate type not handled." << std::endl;
  return calculateToken::VOLTAGE;
}

bool fileParser::isVaribleDefined(std::string varName) {
  for(auto& token : tokens) {
    switch (token->type) {
    case token::PLOT: {
      break;
    }
    case token::COMPONENT: {
      auto t = dynamic_cast<componentToken *>(token.get());
      if (t->name == varName) { return true; }
      break;
    }
    case token::NODE: {
      auto t = dynamic_cast<nodeToken *>(token.get());
      if (t->name == varName) { return true; }
      break;
    }
    case token::FOURIER: {
      auto t = dynamic_cast<fourierToken *>(token.get());
      if (t->name == varName) { return true; }
      break;
    }
    case token::CALCULATE: {
      auto t = dynamic_cast<calculateToken *>(token.get());
      if (t->name == varName) { return true; }
      break;
    }
    default: {
      std::cerr << "ERROR: Varible type not detected." << std::endl;
      break;
    }
    }
  }
  return false;
}

bool fileParser::isComponentDefined(std::string cName) {
  for(auto& token : tokens) {
    if (token->type == token::COMPONENT) {
      auto t = dynamic_cast<componentToken *>(token.get());
      if (t->name == cName) { return true; }
    }
  }
  return false;
}
  

void fileParser::verifyCalculateArguments(calculateToken::calculateType type, std::vector<std::string> inputs) {
  switch (type) {
  case calculateToken::VOLTAGE: {
    if (inputs.size() != 3) {
      std::cerr << "ERROR: Calculating VOLTAGE must have three args." << std::endl;
    } if (isVaribleDefined(inputs[1])) {
      std::cerr << "ERROR: Unable to define varible '" << inputs[1] << "', it is already defined." << std::endl;
    } if (!isComponentDefined(inputs[2])) {
      std::cerr << "ERROR: Unable to calculate '" << inputs[1] << "', the component '" << inputs[2] << "' does not exist." << std::endl;
    }
    break;
  }
  case calculateToken::CURRENT: {
    if (inputs.size() != 3) {
      std::cerr << "ERROR: Calculating CURRENT must have three args." << std::endl;
    } if (isVaribleDefined(inputs[1])) {
      std::cerr << "ERROR: Unable to define varible '" << inputs[1] << "', it is already defined." << std::endl;
    } if (!isComponentDefined(inputs[2])) {
      std::cerr << "ERROR: Unable to calculate '" << inputs[1] << "', the component '" << inputs[2] << "' does not exist." << std::endl;
    }
    break;
  }
  default: {
    std::cerr << "ERROR: Calculate type not handled." << std::endl;
    break;
  }
  }
}

std::shared_ptr<token> fileParser::getTokenPtrFromName(const std::string& argName) {
  for(auto& token : tokens) {
    switch (token->type) {
    case token::PLOT: {
      break;
    }
    case token::COMPONENT: {
      auto t = dynamic_cast<componentToken *>(token.get());
      if (t->name == argName) { return token; }
      break;
    }
    case token::NODE: {
      auto t = dynamic_cast<nodeToken *>(token.get());
      if (t->name == argName) { return token; }
      break;
    }
    case token::FOURIER: {
      auto t = dynamic_cast<fourierToken *>(token.get());
      if (t->name == argName) { return token; }
      break;
    }
    case token::CALCULATE: {
      auto t = dynamic_cast<calculateToken *>(token.get());
      if (t->name == argName) { return token; }
      break;
    }
    default: {
      std::cerr << "ERROR: Varible type not detected." << std::endl;
      break;
    }
    }

  }
  std::cerr << "ERROR: Unable to find token, this should be unreachable" << std::endl;
  std::cerr << "\tToken name is:" << argName << std::endl;
  return nullptr;
}

void fileParser::addComponent(const std::string &line) {
  std::shared_ptr<componentToken> component = getComponent(line);

  std::vector<std::string> inputs = getInputs(line);
  if (component->componentType != VOLTAGESOURCE && inputs.size() != 2) {
    // FIXME: When we add opams, bjts, etc this must change
    std::cerr << "ERROR: Components must have two inputs." << std::endl;
    std::cerr << "\tEX: COMPONENT{NAME}{VALUE}" << std::endl;
  } // TODO: Error handling for AC/sqaure wave signals
  if (component->componentType == VOLTAGESOURCE && inputs[1] == "AC") {
    std::string name = getName(inputs[0]);
    double amplitude = getValue(inputs[2]);
    double frequency = getValue(inputs[3]);
    double shift = getValue(inputs[4]);
    component->setFunctionType(VoltageSource::AC);
    component->addName(name);
    component->addValues({amplitude, frequency, shift});
  } else if(component->componentType == VOLTAGESOURCE && inputs[1] == "SQUARE") {
    std::string name = getName(inputs[0]);
    double amplitude = getValue(inputs[2]);
    double frequency = getValue(inputs[3]);
    double shift = getValue(inputs[4]);
    component->setFunctionType(VoltageSource::SQUARE_WAVE);
    component->addName(name);
    component->addValues({amplitude, frequency, shift});
  } else {
    std::string name = getName(inputs[0]);
    double value = getValue(inputs[1]);
    component->setFunctionType(VoltageSource::NONE);
    component->addName(name);
    component->addValue(value);
  }
  component->voltageDataToken = getData("v_" + component->name);
  component->currentDataToken = getData("i_" + component->name);
  tokens.push_back(component);
}

void fileParser::addNode(const std::string &line) {
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
    auto it = std::find_if(tokens.begin(), tokens.end(), [&name](const std::shared_ptr<token>& t) {
      if (t->type == token::COMPONENT) {
        auto component = dynamic_cast<componentToken*>(t.get());
        return component && component->name == name;
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
    if (token->type == token::COMPONENT) {
      auto component = dynamic_cast<componentToken *>(token.get());
      auto itComponent = std::find(componentNames.begin(), componentNames.end(), component->name);
      auto itDuplicate = std::find(componentTokens.begin(), componentTokens.end(), token);
      if(itComponent != componentNames.end() && itDuplicate == componentTokens.end()) {
        componentTokens.push_back(token);
        //std::cout << nodeName << " contains the component " << componentToken->name << std::endl;
      }
    }
  }

    
  node->addComponents(componentTokens);
  node->voltageDataToken = getData(node->name);
  tokens.push_back(node);
}

void fileParser::addPlot(const std::string &line) {
  std::shared_ptr<plotToken> plot = getPlot(line);
  std::vector<std::string> inputs = getInputs(line);
  if (inputs.size() != 1) {
    std::cerr << "ERROR: Plots must have 1 input." << std::endl;
    std::cerr << "\tEX: plot{NODE_NAME}" << std::endl;
  }
  // FIXME: Verify that toPlot is a valid voltage/current/node etc
  std::string toPlot = getName(inputs[0]);
  plot->addPlot(toPlot);
  plot->dataToken = getData(plot->name);
  tokens.push_back(plot);
}


void fileParser::addCalculate(const std::string &line) {
  std::shared_ptr<calculateToken> calculate = getCalculate(line);
  std::vector<std::string> inputs = getInputs(line);
  calculateToken::calculateType calculateType = getCalculateType(inputs[0]);
  verifyCalculateArguments(calculateType, inputs);
  std::vector<std::shared_ptr<token>> args;
  std::vector<std::string> argStrings(inputs.begin() + 2, inputs.end());
  for (auto& s : argStrings) {
    args.push_back(getTokenPtrFromName(s));
  }
  calculate->addArgs(args);
  calculate->addName(inputs[1]);
  std::cout << inputs[1] << " : " << calculate->name << std::endl;
  calculate->addType(calculateType);
  calculate->output = getData(calculate->name);
  tokens.push_back(calculate);
}



void fileParser::addFourier(const std::string &line) {
  std::shared_ptr<fourierToken> fourier = getFourier(line);
  std::vector<std::string> inputs = getInputs(line);
  if (inputs.size() != 2) {
    std::cerr << "ERROR: fourier transforms must have 2 input." << std::endl;
    std::cerr << "\tEX: fourier_transform{VARIBLE_NAME}{TRANSFORM_VARIBLE}" << std::endl;
  }
  std::string varibleName = getName(inputs[0]);
  // FIXME: Verify that toTransform is a valid voltage/current/node etc
  std::string toTransform = getName(inputs[1]);
  fourier->addFourier(varibleName);
  fourier->outputDataToken = getData(varibleName);
  fourier->inputDataToken = getData(toTransform);
  tokens.push_back(fourier);
}

std::shared_ptr<nodeToken> fileParser::getNode(const std::string &line) {
  return std::make_shared<nodeToken>();
}


std::shared_ptr<fourierToken> fileParser::getFourier(const std::string &line) {
  return std::make_shared<fourierToken>();
}


std::shared_ptr<plotToken> fileParser::getPlot(const std::string &line) {
  return std::make_shared<plotToken>();
}

std::shared_ptr<calculateToken> fileParser::getCalculate(const std::string &line) {
  return std::make_shared<calculateToken>();
}

std::shared_ptr<dataToken> fileParser::getData(std::string name) {
  for (auto& token : tokens) {
    switch (token->type) {
    case token::COMPONENT: {
      auto t = dynamic_cast<componentToken *>(token.get());
      std::shared_ptr<dataToken> vDataT = std::dynamic_pointer_cast<dataToken>(t->voltageDataToken);
      if (vDataT->name == name) {
        return vDataT;
      }
      std::shared_ptr<dataToken> cDataT = std::dynamic_pointer_cast<dataToken>(t->currentDataToken);
      if (cDataT->name == name) {
        return cDataT;
      }
      break;
    }
    case token::NODE: {
      auto t = dynamic_cast<nodeToken *>(token.get());
      std::shared_ptr<dataToken> dataT = std::dynamic_pointer_cast<dataToken>(t->voltageDataToken);
      if (dataT->name == name) {
        return dataT;
      }
      break;
    }
    case token::PLOT: {
      auto t = dynamic_cast<plotToken *>(token.get());
      std::shared_ptr<dataToken> dataT = std::dynamic_pointer_cast<dataToken>(t->dataToken);
      if (dataT->name == name) {
        return dataT;
      }
      break;
    }
    case token::FOURIER: {
      auto t = dynamic_cast<fourierToken *>(token.get());
      std::shared_ptr<dataToken> inDataT = std::dynamic_pointer_cast<dataToken>(t->inputDataToken);
      if (inDataT->name == name) {
        return inDataT;
      }
      std::shared_ptr<dataToken> outDataT = std::dynamic_pointer_cast<dataToken>(t->outputDataToken);
      if (outDataT->name == name) {
        return outDataT;
      }
      break;
    }
    case token::CALCULATE: {
      auto t = dynamic_cast<calculateToken *>(token.get());
      std::shared_ptr<dataToken> dataT = std::dynamic_pointer_cast<dataToken>(t->output);
      if (dataT->name == name) {
        return dataT;
      }
      break;
    }
    case token::DATA: {
      break;
    }
    default:
      std::cerr << "ERROR: Token type not handled" << std::endl;
      break;
    }
  }
  return std::make_shared<dataToken>(name);
}


std::shared_ptr<componentToken> fileParser::getComponent(const std::string &line) {
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

bool fileParser::tokenIsComponent(std::string token) {
  // FIXME: make this more elegent 
  return token == "resistor" || token == "capacitor" || token == "voltage_source" || token == "inductor";
}

bool fileParser::tokenIsNode(std::string token) {
  // FIXME: make this more elegent 
  return token == "node";
}

bool fileParser::tokenIsPlot(std::string token) {
  // FIXME: make this more elegent 
  return token == "plot";
}
bool fileParser::tokenIsFourier(std::string token) {
  // FIXME: make this more elegent 
  return token == "fourier_transform";
}

bool fileParser::tokenIsCalculate(std::string token) {
    return token == "calculate";
}


