#include "Circuit.h"

Component::Component(const std::string &Name, ComponentType Type)
    : ComponentName(Name), Type(Type) {}

Resistor::Resistor(const std::string &Name, double Value)
  : Component(Name, ComponentType::RESISTOR), Resistance(Value) {}

Capacitor::Capacitor(const std::string &Name, double Value)
    : Component(Name, ComponentType::CAPACITOR), Capacitance(Value) {}

Inductor::Inductor(const std::string &Name, double Value)
    : Component(Name, ComponentType::INDUCTOR), Inductance(Value) {}

VoltageSource::VoltageSource(const std::string &Name, double Value)
    : Component(Name, ComponentType::VOLTAGESOURCE), Voltage(Value) {}

Node::Node(const std::string &name) : nodeName(name) {}

void Node::addComponent(std::shared_ptr<Component> component) {
  components.push_back(component);
}

