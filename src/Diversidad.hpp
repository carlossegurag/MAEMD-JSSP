#ifndef DIVERSIDAD_HPP
#define DIVERSIDAD_HPP

#include <vector>

using namespace std;

#include "Solucion.hpp"

class Diversidad
{
public:

  static double calcularEntropiaGen(double *arr,  int n);
  static double calcularEntropiaPob(vector<Solucion *> & pob, int tamPob=0);

  static double similitudInds(Solucion *s1, Solucion *s2);
  static double distanciaInds(Solucion *s1, Solucion *s2, int tipo=0);

  static double distanciaPosInds(Solucion *s1, Solucion *s2);

  static double distanciaDifsInds(Solucion *s1, Solucion *s2);

  static  double distNCSInds(Solucion *s1, Solucion *s2);

  static double calcularDiversidadPob(vector<Solucion *> & pob, int tamPob=0, int tipo=0, bool actualiza=true);

  static double contribucionDivInd(Solucion *s, vector<Solucion *> & pob, int total, int tipo=0);
  
  // static double contribucionDivPob(Solucion *s, vector<Solucion *> pob); 

  static bool comparaMakespanSols(Solucion *s1, Solucion *s2);
};


#endif