#ifndef HEA_HPP
#define HEA_HPP

#include <vector>
#include <deque>
#include <map>

using namespace std;

#include "AlgoritmoGenetico.hpp"
#include "Solucion.hpp"

class HEA : public AlgoritmoGenetico {  

  protected:
    int mksMax;
    int mksMin;
    vector<vector<double>> sP;
    double sPMax;
    double sPMin;

  public:
    HEA(JobShop *p, int tamPob, int valOpt, int max_itersTS);
    ~HEA();

    void calcularMatrizSP();
    void actualizarMatrizSP(int ind1, int ind2);

    Solucion * optimizador(double beta,
      double maxTimeSearch, bool guardarPob, string nameFileOut);

    void seleccionaIndividuos(double beta, int indH=-1);

};

#endif