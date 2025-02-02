 
template<typename T1, typename T2, typename T3>
std::pair<std::vector<double>, std::vector<matrix<double>>> DAESolve2(DifferentialAlgebraicEquation<T1, T2, T3> DAE, matrix<double> initalGuess, double timeStep, double timeEnd) {
  std::vector<matrix<double>> results;
  std::vector<double> time;
  int steps = ceil(timeEnd/timeStep);

  auto DEIdx = getDifferentailEquationIdxFromDAE(DAE);
  auto DEColIdx = getDEColIdx(DAE.E);
  auto DEs = getDifferentailEquationsFromDAE(DAE);
  DEs.E.print("E");
  auto AEIdx = getAlgebraicEquationIdxFromDAE(DAE);
  auto AEs = getAlgebraicEquationsFromDAE(DAE);
  AEs.A.print("A");
    
  for (int i = 0; i < steps; i++) {
    double tn = i * timeStep - timeStep;
    matrix<double> yn;
    if (results.size() == 0) {
      yn = initalGuess;
    } else {
      yn = results[results.size() - 1];
    }
    
    auto ynDE = getRowsFromIdx(yn, DEIdx);
    matrix<double> xn1;
    if constexpr (std::is_arithmetic<T3>::value) {
      xn1 = add(multiply(DEs.E.invert(), subtract(DEs.f, multiply(DEs.A, yn))).scale(timeStep), ynDE);

    } else if constexpr (std::is_same<T3, function>::value) {
      matrix<double> DEsfEval = DEs.f.evaluate(tn);
      xn1 = ((DEs.E.invert() * (DEsfEval - (DEs.A * yn))).scale(timeStep) + ynDE);  
    }


    auto An = eliminateColsFromIdx(AEs.A, DEColIdx);

    matrix<double> xn1New = {std::vector<std::vector<double>>(DAE.f.rows, std::vector<double>(DAE.f.cols, 0.0)), DAE.f.cols, DAE.f.rows};
    int j = 0;
    for (auto row : DEIdx) {
      xn1New.data[row][0] = xn1.data[j][0];
      j++;
    }
    auto An1xn1 = (AEs.A * xn1New);
    matrix<double> newf;
    if constexpr (std::is_arithmetic<T3>::value) {
      newf = subtract(AEs.f, An1xn1);
    } else if constexpr (std::is_same<T3, function>::value) {
      auto AEfEval = AEs.f.evaluate(tn);
      newf = (AEfEval - An1xn1);
    }

    auto NewtonGuess = yn;
    j = 0;
    for (auto row : DEIdx) {
      NewtonGuess.eliminateRow(row - j);
      j++;
    }
    auto AEsols = NewtonsMethod(An, newf, NewtonGuess);

    matrix<double> AEsolsNew = {std::vector<std::vector<double>>(
                                                                 DAE.f.rows, std::vector<double>(DAE.f.cols, 0.0)),
                                DAE.f.cols, DAE.f.rows};

    j = 0;
    for (auto row : AEIdx) {
      AEsolsNew.data[row][0] = AEsols.data[j][0];
      j++;
    }
    auto nextStep = AEsolsNew + xn1New;
    results.push_back(nextStep);
    time.push_back(tn);
  };
  auto output = std::pair<std::vector<double>, std::vector<matrix<double>>>{time, results};
  return output;
}


template<typename T1, typename T2, typename T3>
std::vector<int> getDifferentailEquationIdxFromDAE(DifferentialAlgebraicEquation<T1, T2, T3> DAE) {
  std::vector<int> DERowIdx;
  for (int row = 0; row < DAE.E.rows; ++row) {
    bool isDE = false;
    for (int col = 0; col < DAE.E.cols; ++col) {
      if (DAE.E.data[row][col] != 0.0) {
        isDE = true;
      }
    }
    if (isDE) {
      DERowIdx.push_back(row);
    }
  }
  return DERowIdx;
}


template<typename T1, typename T2, typename T3>
std::vector<int> getAlgebraicEquationIdxFromDAE(DifferentialAlgebraicEquation<T1, T2, T3> DAE) {
  std::vector<int> AERowIdx;
  for (int row = 0; row < DAE.E.rows; ++row) {
    bool isDE = true;
    for (int col = 0; col < DAE.E.cols; ++col) {
      if (DAE.E.data[row][col] != 0.0) {
        isDE = false;
      }
    }
    if (isDE) {
      AERowIdx.push_back(row);
    }
  }
  return AERowIdx;
}




// TODO: Template this
template<typename T1, typename T2, typename T3>
DifferentialEquation<T1, T2, T3> getDifferentailEquationsFromDAE(DifferentialAlgebraicEquation<T1, T2, T3> DAE) {
  auto DERowIdx = getDifferentailEquationIdxFromDAE(DAE);
  //auto AERowIdx = getAlgebraicEquationIdxFromDAE(DAE);
  matrix<T1> ADE = {std::vector<std::vector<T1>>{}, DAE.A.cols, (int)DERowIdx.size()};
  matrix<T2> EDE = {std::vector<std::vector<T2>>{}, DAE.E.cols, (int)DERowIdx.size()};
  matrix<T3> fDE = {std::vector<std::vector<T3>>{}, DAE.f.cols, (int)DERowIdx.size()};
  matrix<symbol> symsDE = {std::vector<std::vector<symbol>>{}, DAE.syms.cols, (int)DERowIdx.size()};

  for (auto& row : DERowIdx) {
    EDE.data.push_back(DAE.E.getRow(row).transpose().data[0]);
    ADE.data.push_back(DAE.A.getRow(row).transpose().data[0]);
    fDE.data.push_back(DAE.f.getRow(row).transpose().data[0]);
    symsDE.data.push_back(DAE.syms.getRow(row).transpose().data[0]);
  }


  std::vector<int> AEColIdx;
  for (int col = 0; col < DAE.E.cols; col++) {
    bool isZero = true;
    for (int row = 0; row < DAE.E.rows; row++) {
      if (DAE.E.data[row][col] != 0.0) {
        isZero = false;
      }
    }
    if (isZero) {
      AEColIdx.push_back(col);
    }
  }
  int i = 0;
  for (auto col : AEColIdx) {
    EDE.eliminateCol(col - i);
    i++;
  }
  
  DifferentialEquation<T1, T2, T3> DE;
  DE.E = EDE;
  DE.A = ADE;
  DE.f = fDE;
  DE.syms = symsDE;
  return DE;
}


template<typename T1, typename T2, typename T3>
AlgebraicEquation<T1, T2> getAlgebraicEquationsFromDAE(DifferentialAlgebraicEquation<T1, T3, T2> DAE) {
  auto AERowIdx = getAlgebraicEquationIdxFromDAE(DAE);
  matrix<T3> EAE = {std::vector<std::vector<T3>>{}, DAE.E.cols, (int)AERowIdx.size()};
  matrix<T1> AAE = {std::vector<std::vector<T1>>{}, DAE.A.cols, (int)AERowIdx.size()};
  matrix<T2> fAE = {std::vector<std::vector<T2>>{}, DAE.f.cols, (int)AERowIdx.size()};
  matrix<symbol> symsAE = {std::vector<std::vector<symbol>>{}, DAE.syms.cols, (int)AERowIdx.size()};

  for (auto& row : AERowIdx) {
    EAE.data.push_back(DAE.E.getRow(row).transpose().data[0]);
    AAE.data.push_back(DAE.A.getRow(row).transpose().data[0]);
    fAE.data.push_back(DAE.f.getRow(row).transpose().data[0]);
    symsAE.data.push_back(DAE.syms.getRow(row).transpose().data[0]);
  }
  AlgebraicEquation<T1, T2> AE;
  AE.A = AAE;
  AE.f = fAE;
  AE.syms = symsAE;
  return AE;
}
