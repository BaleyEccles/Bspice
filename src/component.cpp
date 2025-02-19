#include "component.h"

Component::Component(const std::string &Name, ComponentType Type)
    : ComponentName(Name), Type(Type) {}

Resistor::Resistor(const std::string &Name, double Value)
  : Component(Name, ComponentType::RESISTOR), Resistance(Value) {}

Capacitor::Capacitor(const std::string &Name, double Value)
    : Component(Name, ComponentType::CAPACITOR), Capacitance(Value) {}

Inductor::Inductor(const std::string &Name, double Value)
    : Component(Name, ComponentType::INDUCTOR), Inductance(Value) {}

Opamp::Opamp(const std::string &Name)
    : Component(Name, ComponentType::OPAMP) {}

Diode::Diode(const std::string &Name, double Value)
    : Component(Name, ComponentType::DIODE), voltageDrop(Value) {}

VoltageSource::VoltageSource(const std::string& Name, functionType type, std::vector<double> Values)
  : Component(Name, ComponentType::VOLTAGESOURCE), fType(type), Values(Values) {}

Node::Node(const std::string &name) : nodeName(name) {}

void Node::addComponent(std::shared_ptr<Component> component, Component::connectionType cType) {
  components.push_back(std::make_pair(component, cType));
}

