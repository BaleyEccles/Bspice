#include "fileParser.h"
#include "circuit.h"
#include "component.h"

token::token(tokenType type)
  :type(type) {};

timeToken::timeToken()
  : token(token::TIME) {};
  
componentToken::componentToken(Component::ComponentType componentType)
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
}

std::vector<std::shared_ptr<token>> fileParser::tokenize(const std::string& line) {
  std::istringstream stream(line);
  std::string currentToken;
  for (char c : line) {
    currentToken = currentToken + c;
    if (tokenIsTime(currentToken)) {
      addTime(line);
    } else if (tokenIsComponent(currentToken)) {
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
  int bracketCount = 0;
  std::vector<int> startCapture, endCapture;
  for (int i = 0; i < line.size(); i++) {
    char c = line[i];
    if (c == '{') {
      if (bracketCount == 0) startCapture.push_back(i);
      bracketCount++;
    } else if (c == '}') {
      bracketCount--;
      if (bracketCount == 0) endCapture.push_back(i + 1);
    }
  }
  if (startCapture.size() != endCapture.size() || bracketCount != 0) {
    std::cerr << "ERROR: Mismatched curly braces in " << line << std::endl;
  }
  std::vector<std::string> results;
  for (int i = 0; i < startCapture.size(); i++) {
    int captureLength = endCapture[i] - startCapture[i];
    results.push_back(line.substr(startCapture[i], captureLength));
  }
  // Remove curly braces
  for (auto& str : results) {
    str.erase(0, 1);
    str.pop_back();
  }
  return results;

}

std::string fileParser::getName(std::string name) {
  // this is scuffed and only removes the inputs
  for (int i = 0; i < name.size(); i++) {
    if (name[i] == '{') {
      name = name.substr(0, i);
    }
  }
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

bool fileParser::isDataDefined(std::string varName) {
  for(auto& token : tokens) {
    switch (token->type) {
    case token::PLOT: {
      break;
    }
    case token::TIME: {
      auto t = dynamic_cast<timeToken *>(token.get());
      auto stopTime = dynamic_cast<dataToken *>(t->stopTime.get());
      auto timeStep = dynamic_cast<dataToken *>(t->timeStep.get());
      if (stopTime->name == varName) { return true; }
      if (timeStep->name == varName) { return true; }
      break;
    }
    case token::COMPONENT: {
      auto t = dynamic_cast<componentToken *>(token.get());
      auto voltageData = dynamic_cast<dataToken *>(t->voltageDataToken.get());
      auto currentData = dynamic_cast<dataToken *>(t->currentDataToken.get());
      if (voltageData->name == varName) { return true; }
      if (currentData->name == varName) { return true; }
      break;
    }
    case token::NODE: {
      auto t = dynamic_cast<nodeToken *>(token.get());
      auto voltageData = dynamic_cast<dataToken *>(t->voltageDataToken.get());
      if (voltageData->name == varName) { return true; }
      break;
    }
    case token::FOURIER: {
      auto t = dynamic_cast<fourierToken *>(token.get());
      auto inputData = dynamic_cast<dataToken *>(t->inputDataToken.get());
      auto outputData = dynamic_cast<dataToken *>(t->outputDataToken.get());
      if (inputData->name == varName) { return true; }
      if (outputData->name == varName) { return true; }
      break;
    }
    case token::CALCULATE: {
      auto t = dynamic_cast<calculateToken *>(token.get());
      for (auto& d : t->inputs) {
        auto data = dynamic_cast<dataToken *>(d.get());
        if (data->name == varName) { return true; }
      }
      break;
    }
    case token::DATA: {
      auto t = dynamic_cast<dataToken *>(token.get());
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
    } if (isDataDefined(inputs[1])) {
      std::cerr << "ERROR: Unable to define varible '" << inputs[1] << "', it is already defined." << std::endl;
    } if (!isComponentDefined(inputs[2])) {
      std::cerr << "ERROR: Unable to calculate '" << inputs[1] << "', the component '" << inputs[2] << "' does not exist." << std::endl;
    }
    break;
  }
  case calculateToken::CURRENT: {
    if (inputs.size() != 3) {
      std::cerr << "ERROR: Calculating CURRENT must have three args." << std::endl;
    } if (isDataDefined(inputs[1])) {
      std::cerr << "ERROR: Unable to define varible '" << inputs[1] << "', it is already defined." << std::endl;
    } if (!isComponentDefined(inputs[2])) {
      std::cerr << "ERROR: Unable to calculate '" << inputs[1] << "', the component '" << inputs[2] << "' does not exist." << std::endl;
    }
    break;
  }
  case calculateToken::ADD: {
    if (inputs.size() != 4) {
      std::cerr << "ERROR: Calculating ADD must have four args." << std::endl;
    } if (isDataDefined(inputs[1])) {
      std::cerr << "ERROR: Unable to define varible '" << inputs[1] << "', it is already defined." << std::endl;
    } if (!isDataDefined(inputs[2])) {
      std::cerr << "ERROR: Unable to calculate '" << inputs[1] << "', the varible '" << inputs[2] << "' does not exist." << std::endl;
    } if (!isDataDefined(inputs[3])) {
      std::cerr << "ERROR: Unable to calculate '" << inputs[1] << "', the varible '" << inputs[3] << "' does not exist." << std::endl;
    }
  }
    break;
  case calculateToken::SUBTRACT: {
    if (inputs.size() != 4) {
      std::cerr << "ERROR: Calculating SUBTRACT must have four args." << std::endl;
    } if (isDataDefined(inputs[1])) {
      std::cerr << "ERROR: Unable to define varible '" << inputs[1] << "', it is already defined." << std::endl;
    } if (!isDataDefined(inputs[2])) {
      std::cerr << "ERROR: Unable to calculate '" << inputs[1] << "', the varible '" << inputs[2] << "' does not exist." << std::endl;
    } if (!isDataDefined(inputs[3])) {
      std::cerr << "ERROR: Unable to calculate '" << inputs[1] << "', the varible '" << inputs[3] << "' does not exist." << std::endl;
    }
    break;
  }
  default: {
    std::cerr << "ERROR: Calculate type not handled." << std::endl;
    break;
  }
  }
  if (getTokenPtrFromName(inputs[1]) != nullptr) {
    std::cerr << "ERROR: Redefinition of the token " << inputs[1] << " in calulation." << std::endl;
  }
}


std::shared_ptr<token> fileParser::getTokenPtrFromName(const std::string& argName) {
  for(auto& token : tokens) {
    switch (token->type) {
    case token::PLOT: {
      break;
    }
    case token::TIME: {
      auto t = dynamic_cast<timeToken *>(token.get());
      if (t->name == argName) { return token; }
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
    case token::DATA: {
      auto t = dynamic_cast<dataToken *>(token.get());
      if (t->name == argName) { return token; }
      break;
    }
    default: {
      std::cerr << "ERROR: Varible type not detected." << std::endl;
      break;
    }
    }

  }
  //std::cerr << "ERROR: Unable to find token, this should be unreachable" << std::endl;
  //std::cerr << "Token name is:" << argName << std::endl;
  return nullptr;
}

void fileParser::addTime(const std::string &line) {
  std::shared_ptr<timeToken> time = getTime(line);
  std::vector<std::string> inputs = getInputs(line);
  if (inputs.size() != 2) {
    std::cerr << "ERROR: time must have two number inputs." << std::endl;
    std::cerr << "EX: time{STOP_TIME}{TIME_STEP}" << std::endl;
  }
  time->stopTime = getData("STOP_TIME");
  time->timeStep = getData("TIME_STEP");
  auto stopTime = dynamic_cast<dataToken *>(time->stopTime.get());
  stopTime->addData({getValue(inputs[0])});
  auto timeStep = dynamic_cast<dataToken *>(time->timeStep.get());
  timeStep->addData({getValue(inputs[1])});
  tokens.push_back(time);
}

bool fileParser::sourceIsFunction(std::vector<std::string> inputs) {
  if (inputs[1] == "AC" || inputs[1] == "SQUARE") {
    return true;
  }
  return false;
}

void fileParser::checkIfComponentIsValid(std::shared_ptr<componentToken> component, std::vector<std::string> inputs) {
  switch (component->componentType) {
  case Component::RESISTOR:
  case Component::CAPACITOR:
  case Component::INDUCTOR: {
    if (inputs.size() != 2) {
      std::cerr << "ERROR: Components must have two inputs." << std::endl;
      std::cerr << "EX: COMPONENT{NAME}{VALUE}" << std::endl;
    }
    break;
  }
  case Component::VOLTAGESOURCE: {
    if (sourceIsFunction(inputs)) {
      if (inputs.size() != 5) {
        std::cerr << "ERROR: Functional voltage sources must have two inputs." << std::endl;
        std::cerr << "EX: voltage_source{NAME}{TYPE}{AMPLITUDE}{FREQUENCY}{PHASE SHIFT}" << std::endl;
      }
    } else if (inputs.size() != 2) {
      std::cerr << "ERROR: Constant voltage sources must have two inputs." << std::endl;
      std::cerr << "EX: voltage_source{NAME}{VALUE}" << std::endl;
    }
    break;
  }
  case Component::OPAMP: {
    if (inputs.size() != 1) {
      std::cerr << "ERROR: Op-amps must have one inputs." << std::endl;
      std::cerr << "EX: opamp{NAME}" << std::endl;
    }
    break;
  }
  case Component::DIODE: {
    if (inputs.size() > 3) {
      std::cerr << "ERROR: Didodes must have one or two inputs." << std::endl;
      std::cerr << "EX: diode{NAME}{OPTIONAL VALUE}" << std::endl;
    }
    break;
  }
  default: {
    std::cerr << "ERROR: checkIfComponentIsValid failed" << std::endl;
    std::cerr << "inputs: " << std::endl;
    for (auto& input : inputs) {
      std::cerr << input << std::endl;
    }
    break;
  }
  }

}

void fileParser::createVoltageSource(std::shared_ptr<componentToken> component, std::vector<std::string> inputs) {
  if (inputs[1] == "AC") {
    std::string name = getName(inputs[0]);
    double amplitude = getValue(inputs[2]);
    double frequency = getValue(inputs[3]);
    double shift = getValue(inputs[4]);
    component->fType = VoltageSource::AC;
    component->name = name;
    component->addValues({amplitude, frequency, shift});
  } else if (inputs[1] == "SQUARE") {
    std::string name = getName(inputs[0]);
    double amplitude = getValue(inputs[2]);
    double frequency = getValue(inputs[3]);
    double shift = getValue(inputs[4]);
    component->fType = VoltageSource::SQUARE_WAVE;
    component->name = name;
    component->addValues({amplitude, frequency, shift});
  } else {
    std::string name = getName(inputs[0]);
    double value = getValue(inputs[1]);
    component->fType = VoltageSource::NONE;
    component->name = name;
    component->addValue(value);
  }
}

void fileParser::createDiode(std::shared_ptr<componentToken> component, std::vector<std::string> inputs) {
  std::string name = getName(inputs[0]);
  double value = 0.0;
  if (inputs.size() == 2) {
    value = getValue(inputs[1]);
  }
  component->fType = VoltageSource::NONE;
  component->name = name;
  component->addValue(value);
}
  
void fileParser::addComponent(const std::string &line) {
  std::shared_ptr<componentToken> component = getComponent(line);
  
  std::vector<std::string> inputs = getInputs(line);
  checkIfComponentIsValid(component, inputs);
  
  if (component->componentType == Component::VOLTAGESOURCE) {
    createVoltageSource(component, inputs);
  } else if (component->componentType == Component::DIODE) {
    createDiode(component, inputs);
  } else if (component->componentType == Component::OPAMP) {
    std::string name = getName(inputs[0]);
    component->fType = VoltageSource::NONE;
    component->name = name;
  } else {
    std::string name = getName(inputs[0]);
    double value = getValue(inputs[1]);
    component->fType = VoltageSource::NONE;
    component->name = name;
    component->addValue(value);
  }
  
  component->voltageDataToken = getData("v_" + component->name);
  component->currentDataToken = getData("i_" + component->name);
  tokens.push_back(component);
}

void fileParser::addNode(const std::string &line) {

  std::shared_ptr<nodeToken> node = getNode(line);
  std::vector<std::string> inputs = getInputs(line);
  for (auto& s : inputs) {
    std::vector<std::string> subInputs = getInputs(line);
  }
  if (inputs.size() < 2) {
    std::cerr << "ERROR: Nodes must have N + 1 inputs." << std::endl;
    std::cerr << "EX: node{NAME}{COMPONENT_1}{COMPONENT_2}...{COMPONENT_N}" << std::endl;
  }
  std::string nodeName = getName(inputs[0]);
  node->name = nodeName;
  
  std::vector<std::string> componentNames(inputs.begin() + 1, inputs.end());
  for (int i = 0; i < componentNames.size(); i++) {
    componentNames[i] = getName(componentNames[i]);
  }

  for (auto& name : componentNames) {
    if (!isComponentDefined(name)) {
      std::cerr << "ERROR: Component `" << name << "` not defined." << std::endl;
    }
  }

  std::vector<std::pair<std::shared_ptr<token>, Component::connectionType>> componentTokens;
  
  for (auto& token : tokens) {
    if (token->type == token::COMPONENT) {
      auto component = dynamic_cast<componentToken *>(token.get());
      auto itComponent = std::find(componentNames.begin(), componentNames.end(), component->name);
      //auto itDuplicate = std::find(componentTokens.begin(), componentTokens.end(), token);
      if(itComponent != componentNames.end()) {// && itDuplicate == componentTokens.end()) {
        if (component->componentType == Component::DIODE) {
          for (auto& input : getInputs(line)) {
            if (getName(input) == component->name) {
              std::vector<std::string> diodeInputs = getInputs(input);
              if (diodeInputs.size() != 1) {
                std::cerr << "ERROR: When applying a diode to a node you must define the connection direction" << std::endl;
                std::cerr << "DIODE_NAME{+ OR -}" << std::endl;
              }
              if (diodeInputs[0] == "+") {
                componentTokens.push_back(std::make_pair(token, Component::DIODE_P));
              } else if (diodeInputs[0] == "-") {
                componentTokens.push_back(std::make_pair(token, Component::DIODE_N));
              } else {
                std::cerr << "ERROR: When applying a diode to a node you must define the connection direction" << std::endl;
                std::cerr << "DIODE_NAME{+ OR -}" << std::endl;
              }
            }
          }
        } if (component->componentType == Component::OPAMP) {
          for (auto& input : getInputs(line)) {
            if (getName(input) == component->name) {
              std::vector<std::string> opampInputs = getInputs(input);
              if (opampInputs.size() != 1) {
                std::cerr << "ERROR: When applying a opamp to a node you must define the connection direction" << std::endl;
                std::cerr << "OPAMP_NAME{+ OR - OR out}" << std::endl;
              }
              if (opampInputs[0] == "+") {
                componentTokens.push_back(std::make_pair(token, Component::OPAMP_P));
              } else if (opampInputs[0] == "-") {
                componentTokens.push_back(std::make_pair(token, Component::OPAMP_N));
              } else if (opampInputs[0] == "out") {
                componentTokens.push_back(std::make_pair(token, Component::OPAMP_OUT));
              } else {
                std::cerr << "ERROR: When applying a opamp to a node you must define the connection direction" << std::endl;
                std::cerr << "OPAMP_NAME{+ OR - OR out}" << std::endl;
              }
            }
          }
        } else {
          componentTokens.push_back(std::make_pair(token, Component::UNDEFINED));
        }
      }
    }
  }
    
  node->components = componentTokens;
  node->voltageDataToken = getData(node->name);
  tokens.push_back(node);
}

void fileParser::addPlot(const std::string &line) {
  std::shared_ptr<plotToken> plot = getPlot(line);
  std::vector<std::string> inputs = getInputs(line);
  if (inputs.size() != 1) {
    std::cerr << "ERROR: Plots must have 1 input." << std::endl;
    std::cerr << "EX: plot{VARIBLE_NAME}" << std::endl;
  }
  
  std::string varibleName = getName(inputs[0]);
  plot->plotVaribleName = varibleName;
  if (!isDataDefined(plot->plotVaribleName)) {
    std::cerr << "ERROR: Unable to create plot, the varible " << plot->plotVaribleName << " is not defined." << std::endl;
  }
  plot->dataToken = getData(plot->plotVaribleName);
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
  calculate->args = args;
  calculate->name = inputs[1];
  calculate->calculationType = calculateType;
  if (isDataDefined(calculate->name)) {
    std::cerr << "ERROR: Could not calculate, the varible " << calculate->name  << " is already defined." << std::endl;
  }
  calculate->output = getData(calculate->name);
  tokens.push_back(calculate);
}



void fileParser::addFourier(const std::string &line) {
  std::shared_ptr<fourierToken> fourier = getFourier(line);
  std::vector<std::string> inputs = getInputs(line);
  if (inputs.size() != 2) {
    std::cerr << "ERROR: fourier transforms must have 2 input." << std::endl;
    std::cerr << "EX: fourier_transform{VARIBLE_NAME}{TRANSFORM_VARIBLE}" << std::endl;
  }
  std::string inputVaribleName = getName(inputs[0]);
  std::string varibleToTransformName = getName(inputs[1]);
  if (!isDataDefined(varibleToTransformName)) {
    std::cerr << "ERROR: Unable to create fourier, the varible '" << inputVaribleName << "' is not defined." << std::endl;
  }
  fourier->name = inputVaribleName;
  fourier->outputDataToken = getData(inputVaribleName);
  fourier->inputDataToken = getData(varibleToTransformName);
  tokens.push_back(fourier);
}

std::shared_ptr<nodeToken> fileParser::getNode(const std::string &line) {
  return std::make_shared<nodeToken>();
}

std::shared_ptr<timeToken> fileParser::getTime(const std::string &line) {
  return std::make_shared<timeToken>();
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
    case token::TIME: {
      auto t = dynamic_cast<timeToken *>(token.get());
      std::shared_ptr<dataToken> stopTime = std::dynamic_pointer_cast<dataToken>(t->stopTime);
      if (stopTime->name == name) {
        return stopTime;
      }
      std::shared_ptr<dataToken> timeStep = std::dynamic_pointer_cast<dataToken>(t->timeStep);
      if (stopTime->name == name) {
        return timeStep;
      }
      break;
    }
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
      std::shared_ptr<dataToken> dataT = std::dynamic_pointer_cast<dataToken>(token);
      if (dataT->name == name) {
        return dataT;
      }
      break;
    }
    default:
      
      std::cerr << "ERROR: Token of type " << token->type << " was not handled" << std::endl;
      break;
    }
  }
  auto output = std::make_shared<dataToken>(name);
  tokens.push_back(output);
  return output;
}


std::shared_ptr<componentToken> fileParser::getComponent(const std::string &line) {
  std::string resistor = "resistor";
  if (line.substr(0, resistor.size()) == resistor) {
    return std::make_shared<componentToken>(Component::RESISTOR);
  }
  std::string capacitor = "capacitor";
  if (line.substr(0, capacitor.size()) == capacitor) {
    return std::make_shared<componentToken>(Component::CAPACITOR);
  }
  std::string inductor = "inductor";
  if (line.substr(0, inductor.size()) == inductor) {
    return std::make_shared<componentToken>(Component::INDUCTOR);
  }
  std::string voltageSource = "voltage_source";
  if (line.substr(0, voltageSource.size()) == voltageSource) {
    auto inputs = getInputs(line);
    return std::make_shared<componentToken>(Component::VOLTAGESOURCE);
  }
  std::string currentSource = "current_source";
  if (line.substr(0, currentSource.size()) == currentSource) {
    return std::make_shared<componentToken>(Component::CURRENTSOURCE);
  }
  std::string opamp = "opamp";
  if (line.substr(0, opamp.size()) == opamp) {
    return std::make_shared<componentToken>(Component::OPAMP);
  }
  std::string diode = "diode";
  if (line.substr(0, diode.size()) == diode) {
    return std::make_shared<componentToken>(Component::DIODE);
  }
  std::cerr << "ERROR: Component not found" << std::endl;
  return 0;
}

bool fileParser::tokenIsTime(std::string token) {
    return token == "time";
}

bool fileParser::tokenIsComponent(std::string token) {
  return token == "resistor" || token == "capacitor" || token == "voltage_source" || token == "inductor" || token == "opamp" || token == "diode";
}

bool fileParser::tokenIsNode(std::string token) {
  return token == "node";
}

bool fileParser::tokenIsPlot(std::string token) {
  return token == "plot";
}
bool fileParser::tokenIsFourier(std::string token) {
  return token == "fourier_transform";
}

bool fileParser::tokenIsCalculate(std::string token) {
    return token == "calculate";
}





