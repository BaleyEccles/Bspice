#pragma once
#include "Circuit.h"
#include "Bmaths/FourierTransform.h"
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
  
  void addOctaveVarible(const std::string &name, std::vector<double> &plotData);
  void addPlot(const std::string &name, std::vector<double> &plotData);
  void addFourierPlot(const std::string &name, std::vector<double> &frequencyData, std::vector<double> &magnitudeData);
  void createOctavePlotFileFromTokens();


};


#include "tokenParser_impl.h"
