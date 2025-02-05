#include "tokenParser.h"

// data is in a weird format
// it is a vector of matrix that is similar to syms
postProcess::postProcess(const std::string& octaveFileName, std::vector<double> &time, std::vector<matrix<double>> &data, matrix<symbol> &syms, std::vector<std::shared_ptr<token>> &tokens)
  : fileName(octaveFileName), time(time), data(data), syms(syms), tokens(tokens) {

  file.open(fileName);

  if (!file.is_open()) {
    std::cerr << "Error opening file" << std::endl;
    return;
  }
  createOctavePlotFileFromTokens();
  file.close();
};
postProcess::~postProcess() {
}
  
void postProcess::addOctaveVarible(const std::string &name, std::vector<double> &plotData) {
  file << (name + " = [");
  for (int i = 0; i < plotData.size(); i++) {
    file << std::scientific << std::setprecision(5) << plotData[i] << " ";
  }
  file << ("];\n");
};

void postProcess::addPlot(const std::string &name, std::vector<double> &plotData) {
  //std::cout << name << " " << plotData.size() << std::endl;
  addOctaveVarible(name, plotData);
  file << ("figure();\n");
  file << ("plot(t, " + name + ");\n");
  file << ("xlabel(\"t\");\n");
  file << ("ylabel(\"" + name + "\");\n");
};

void postProcess::addFourierPlot(const std::string &name, std::vector<double> &frequencyData, std::vector<double> &magnitudeData) {// TODO: Deal with phase
  const std::string fName = "frequency_" + name;
  const std::string mName = "magnitude_" + name;
  const std::string pName = "phase_" + name;
  addOctaveVarible(fName, frequencyData);
  addOctaveVarible(mName, magnitudeData);  
  file << ("figure();\n");
  file << ("plot(" + fName + "," + mName + ");\n");
  file << ("xlabel(\"" + fName + "\");\n");
  file << ("ylabel(\"" + mName + "\");\n");
};

void postProcess::createOctavePlotFileFromTokens() {
  //file << "graphics_toolkit(\"gnuplot\")\n";
  file << "set(0, 'DefaultTextFontSize', 25);";
  file << "set(0, 'DefaultAxesFontSize', 25);";
  addOctaveVarible("t", time);
  
  for (auto& token : tokens) {
    if (token->type == token::PLOT) {
      auto plotT = dynamic_cast<plotToken *>(token.get());
      bool isValidPlot = false;
      for (int row = 0; row < syms.rows; row++) {
        if (syms.data[row][0].name == plotT->name) {
          std::vector<double> plotData;
          for (auto& d : data) {
            plotData.push_back(d.data[row][0]);
          }
          addPlot(plotT->name, plotData);
          isValidPlot = true;
        }
      }
      if (!isValidPlot) {
        std::cerr << "ERROR: `" << plotT->name << "` was not able to be plotted." << std::endl;
      }

    } else if (token->type == token::FOURIER) {
      auto fourierT = dynamic_cast<fourierToken *>(token.get());
      bool isValidPlot = false;
      for (int row = 0; row < syms.rows; row++) {
        if (syms.data[row][0].name == fourierT->transformVarible) {
          std::vector<double> toTransformData;
          for (auto& d : data) {
            toTransformData.push_back(d.data[row][0]);
          }
          matrix<double> toTransform {
            {toTransformData},
            (int)toTransformData.size(), 1
          };
          FourierTransform dft(time, toTransform);
          addFourierPlot(fourierT->transformVarible, dft.frequency.data[0], dft.magnitudes.data[0]);
          isValidPlot = true;
        }
      }
      if (!isValidPlot) {
        std::cerr << "ERROR: `" << fourierT->transformVarible << "` was not able to be plotted." << std::endl;
      }

    }
  }
  file << ("pause;");

};



