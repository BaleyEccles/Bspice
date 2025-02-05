template<typename T1, typename T2, typename T3>
Circuit<T1, T2, T3> createCircuitFromTokens(std::vector<std::shared_ptr<token>>& tokens) {
  Circuit<T1, T2, T3> circuit;
  for (auto& token : tokens) {
    if (token->type == token::NODE) {
      auto nodeT = dynamic_cast<nodeToken *>(token.get());
      //std::cout << "processing node " << nodeToken->name << std::endl;
      Node *node = new Node(nodeT->name);
      for (auto& component : nodeT->components) {
        auto componentT = dynamic_cast<componentToken *>(component.get());
        switch(componentT->componentType){
        case ComponentType::VOLTAGESOURCE: {
          std::shared_ptr<Component> c = std::make_shared<VoltageSource>(componentT->name, componentT->fType, componentT->values);;
          node->addComponent(c);
          break;
        }
        case ComponentType::RESISTOR: {
          auto c = std::make_shared<Resistor>(componentT->name, componentT->values[0]);
          node->addComponent(c);
          break;
        }
        case ComponentType::CAPACITOR: {
          auto c = std::make_shared<Capacitor>(componentT->name, componentT->values[0]);
          node->addComponent(c);
          break;
        }
        case ComponentType::INDUCTOR: {
          auto c = std::make_shared<Inductor>(componentT->name, componentT->values[0]);
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

