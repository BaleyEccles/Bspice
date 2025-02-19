#pragma once
#include <string>
#include <vector>
#include <memory>


class Node;
class Component {
public:
  enum ComponentType {
    VOLTAGESOURCE = 0,
    CURRENTSOURCE,
    RESISTOR,
    CAPACITOR,
    INDUCTOR,
    OPAMP,
    DIODE
  };

  enum connectionType {
    UNDEFINED = 0,
    OPAMP_N,
    OPAMP_P,
    OPAMP_OUT,
    DIODE_P,
    DIODE_N,
    BJT_BASE,
    BJT_COLLECTOR,
    BJT_EMITTER,
  };
  
  Component(const std::string& name, ComponentType Type);
  virtual ~Component() = default;
  std::string ComponentName;
  ComponentType Type;
  std::vector<Node*> Connections;
  
};

class Resistor : public Component {
public:
  Resistor(const std::string& Name, double Value);
  double Resistance;
};

class Capacitor : public Component {
public:
  Capacitor(const std::string& Name, double Value);
  double Capacitance;
};

class Inductor : public Component {
public:
  Inductor(const std::string& Name, double Value);
  double Inductance;
};

class Opamp : public Component {
public:
  Opamp(const std::string& Name);
};

class Diode : public Component {
public:
  Diode(const std::string& Name, double Value);
  double voltageDrop = 0.0;
};

class VoltageSource : public Component {
public:
  enum functionType {
    NONE,
    AC,
    SQUARE_WAVE
  };
  
  VoltageSource(const std::string& Name, functionType type, std::vector<double> Values);
  std::vector<double> Values;
  functionType fType;
};


class Node {
public:
  Node(const std::string& name);
  void addComponent(std::shared_ptr<Component> component, Component::connectionType cType);
  std::string nodeName;
  std::vector<std::pair<std::shared_ptr<Component>, Component::connectionType>> components;
};
