#pragma once
#include "circuit.h"
#include "BMaths/fourierTransform.h"
#include "fileParser.h"
#include <memory>
#include <string>
#include <vector>
#include <fstream>

template<typename T1, typename T2, typename T3>
Circuit<T1, T2, T3> createCircuitFromTokens(std::vector<std::shared_ptr<token>>& tokens);


class postProcess {
public:
  postProcess(const std::string& octaveFileName, std::vector<double> &time, std::vector<matrix<double>> &data, matrix<symbol> &syms, std::vector<std::shared_ptr<token>> &tokens);
  ~postProcess();
  
private:
  const std::string fileName;
  std::vector<double> time;
  std::vector<matrix<double>> data;
  matrix<symbol> syms;
  std::vector<std::shared_ptr<token>> tokens;
  std::ofstream file;
  
  void addOctaveVarible(const std::string &name, std::vector<double> plotData);
  void addPlot(const std::string &name, std::vector<double> plotData);
  void addFourierPlot(const std::string &name, std::vector<double> frequencyData, std::vector<double> magnitudeData);

  void createOctavePlotFileFromTokens();

  std::vector<std::shared_ptr<token>> getConnectedNodesFromComponentPtr(std::shared_ptr<token> componentT);
  std::vector<double> calculateVoltage(std::shared_ptr<token> token);
  std::vector<double> calculateCurrent(std::shared_ptr<token> t);
  std::vector<double> calculateAdd(std::shared_ptr<token> t1, std::shared_ptr<token> t2);
  std::vector<double> calculateSubtract(std::shared_ptr<token> t1, std::shared_ptr<token> t2);
  std::vector<double> getDataFromToken(std::shared_ptr<token> t);
  void fillInDataTokens();
  int findIdxFromName(std::string name);


};


template<typename T1, typename T2, typename T3>
Circuit<T1, T2, T3> createCircuitFromTokens(std::vector<std::shared_ptr<token>>& tokens) {
  Circuit<T1, T2, T3> circuit;
  for (auto& token : tokens) {
    if (token->type == token::TIME) {
      auto time = dynamic_cast<timeToken *>(token.get());
      auto stopTime = dynamic_cast<dataToken *>(time->stopTime.get());
      circuit.stopTime = stopTime->data[0][0];
      auto timeStep = dynamic_cast<dataToken *>(time->timeStep.get());
      circuit.timeStep = timeStep->data[0][0];
    }
    if (token->type == token::NODE) {
      auto nodeT = dynamic_cast<nodeToken *>(token.get());
      //std::cout << "processing node " << nodeToken->name << std::endl;
      Node *node = new Node(nodeT->name);
      for (auto& component : nodeT->components) {
        auto componentT = dynamic_cast<componentToken *>(component.first.get());
        switch(componentT->componentType){
        case Component::VOLTAGESOURCE: {
          std::shared_ptr<Component> c = std::make_shared<VoltageSource>(componentT->name, componentT->fType, componentT->values);
          componentT->circuitComponentPtr = c;
          node->addComponent(c, component.second);
          break;
        }
        case Component::RESISTOR: {
          auto c = std::make_shared<Resistor>(componentT->name, componentT->values[0]);
          componentT->circuitComponentPtr = c;
          node->addComponent(c, component.second);
          break;
        }
        case Component::CAPACITOR: {
          auto c = std::make_shared<Capacitor>(componentT->name, componentT->values[0]);
          componentT->circuitComponentPtr = c;
          node->addComponent(c, component.second);
          break;
        }
        case Component::INDUCTOR: {
          auto c = std::make_shared<Inductor>(componentT->name, componentT->values[0]);
          componentT->circuitComponentPtr = c;
          node->addComponent(c, component.second);
          break;
        }
        case Component::OPAMP: {
          auto c = std::make_shared<Opamp>(componentT->name);
          componentT->circuitComponentPtr = c;
          node->addComponent(c, component.second);
          break;
        }
        case Component::DIODE: {
          auto c = std::make_shared<Diode>(componentT->name, componentT->values[0]);
          componentT->circuitComponentPtr = c;
          node->addComponent(c, component.second);
          circuit.isLinear = false;
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



