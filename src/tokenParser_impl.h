template<typename T1, typename T2, typename T3>
Circuit<T1, T2, T3> createCircuitFromTokens(std::vector<std::shared_ptr<fileParser::token>>& tokens) {
  Circuit<T1, T2, T3> circuit;
  for (auto& token : tokens) {
    if (token->type == fileParser::token::NODE) {
      auto nodeToken = dynamic_cast<fileParser::nodeToken *>(token.get());
      //std::cout << "processing node " << nodeToken->name << std::endl;
      Node *node = new Node(nodeToken->name);
      for (auto& component : nodeToken->components) {
        auto componentToken = dynamic_cast<fileParser::componentToken *>(component.get());
        switch(componentToken->componentType){
        case ComponentType::VOLTAGESOURCE_FUNCTION: 
        case ComponentType::VOLTAGESOURCE: {
          std::shared_ptr<Component> c = nullptr;
          if (componentToken->fType == VoltageSourceFunction::NONE) {
            c = std::make_shared<VoltageSource>(componentToken->name, componentToken->values[0]);
          } else {
            switch(componentToken->type) {
            case VoltageSourceFunction::AC: {
              c = std::make_shared<VoltageSourceFunction>(componentToken->name, VoltageSourceFunction::AC, componentToken->values);
              break;
            }
            default: {
              std::cerr << "ERROR: Voltage source function not handled yet." << std::endl;
              break;
            }
            }
          }
          node->addComponent(c);
          break;
        }
        case ComponentType::RESISTOR: {
          auto c = std::make_shared<Resistor>(componentToken->name, componentToken->values[0]);
          node->addComponent(c);
          break;
        }
        case ComponentType::CAPACITOR: {
          auto c = std::make_shared<Capacitor>(componentToken->name, componentToken->values[0]);
          node->addComponent(c);
          break;
        }
        case ComponentType::INDUCTOR: {
          auto c = std::make_shared<Inductor>(componentToken->name, componentToken->values[0]);
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

