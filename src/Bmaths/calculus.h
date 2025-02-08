#include <vector>

template<typename T> 
std::vector<T> differentiateVector(std::vector<double> time, std::vector<T> data) {
  std::vector<T> output(data.size());
  for (int i = 0; i < data.size(); i++) {
    if (i == 0) { // use forward approximation
      double h = time[i + 1] - time[i];
      output[i] = (data[i + 1] - data[i])/h;
    } else if (i == data.size() - 1) { // use backward approximation
      double h = time[i] - time[i - 1];
      output[i] = (data[i] - data[i - 1])/h;
    } else { // use three-point central difference formula
      double h1 = time[i] - time[i - 1];
      double h2 = time[i + 1] - time[i];
      output[i] = (data[i + 1] - data[i - 1])/(h1 + h2);
    }
  }
  return output;
}

// \int_{0}^{t} f(x) dx
template<typename T> 
std::vector<T> integrateVectorWithTime(std::vector<double> time, std::vector<T> data) {
  std::vector<T> output(data.size());
  for (int i = 0; i < data.size(); i++) {
    double f_i = 0;
    for (int j = 1; j < i - 1; j++) {
      f_i += data[j];
    }
    if (i == 0) {
      double h = time[i + 1] - time[i];
      output[i] = (h/2)*(data[0] + 2*f_i + data[i]) ;
    } else if (i == data.size() - 1) {
      double h = time[i] - time[i - 1];
      output[i] = (h/2)*(data[0] + 2*f_i + data[i]) ;
    } else {
      double h = time[i] - time[i - 1];
      output[i] = (h/2)*(data[0] + 2*f_i + data[i]) ;
    }
  }
  return output;
}
