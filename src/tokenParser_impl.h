template <class T>
Circuit<T> createCircuitFromTokens(std::vector<std::shared_ptr<fileParser::token>>& tokens) {
  Circuit<T> circuit;
  for (auto& token : tokens) {
    if (token->type == fileParser::token::NODE) {
      auto nodeToken = dynamic_cast<fileParser::nodeToken *>(token.get());
      //std::cout << "processing node " << nodeToken->name << std::endl;
      Node *node = new Node(nodeToken->name);
      for (auto& component : nodeToken->components) {
        auto componentToken = dynamic_cast<fileParser::componentToken *>(component.get());
        switch(componentToken->componentType){
        case ComponentType::VOLTAGESOURCE: {
          auto c = std::make_shared<VoltageSource>(componentToken->name, componentToken->value);
          node->addComponent(c);
          break;
        }
        case ComponentType::RESISTOR: {
          auto c = std::make_shared<Resistor>(componentToken->name, componentToken->value);
          node->addComponent(c);
          break;
        }
        case ComponentType::CAPACITOR: {
          auto c = std::make_shared<Capacitor>(componentToken->name, componentToken->value);
          node->addComponent(c);
          break;
        }
        case ComponentType::INDUCTOR: {
          auto c = std::make_shared<Inductor>(componentToken->name, componentToken->value);
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

