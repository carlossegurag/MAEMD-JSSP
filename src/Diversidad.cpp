#include <cstdlib>
#include <iostream>
#include <cmath>
#include <map>
#include <cfloat>
#include "Diversidad.hpp"
#include "UtilJS.hpp"

using namespace std;

double Diversidad::calcularEntropiaPob(vector<Solucion *> & pob, int tamPob) {



  int numMaquinas = pob[0]->getNumMaquinas();
  int numTrabajos = pob[0]->getNumTrabajos();
  double infoGenes[numMaquinas][numTrabajos][numTrabajos];

  /* Inicializar contadores a cero */
  for(int i =0; i < numMaquinas; i++) {
    for (int j=0; j < numTrabajos; j++) {
      for(int k=0; k < numTrabajos; k++) {
        infoGenes[i][j][k] = 0.0;
      }
    }
  }
  vector<vector<int>> perSol = pob[0]->getPermutacionSol();

  int trabajo = 0;

  tamPob = tamPob <= 0 ? pob.size() : tamPob;

  /* Recorrer la poblacion */
  for(int k=0; k < tamPob; k++) {
    /* Recorrer cada individuo */
    perSol = pob[k]->getPermutacionSol();
    for(int i=0; i < numMaquinas; i++) {
      for(int j=0; j < numTrabajos; j++) {
        trabajo = (perSol[i][j]-1)/numMaquinas;
        infoGenes[i][j][ trabajo ] += 1.0;
      }
    }
  }

  // Calcular frecuencia para cada posible valor de los genes
  for(int i=0; i < numMaquinas; i++) {
    for (int j=0; j < numTrabajos; j++) {
      for(int k=0; k < numTrabajos; k++) {
        infoGenes[i][j][k] /= tamPob;
      }
    }
  }

  double res = 0;

  for(int i=0; i < numMaquinas; i++) {
    for(int j=0; j < numTrabajos; j++) {
      /* Calcular entropia para cada gen */
     res += Diversidad::calcularEntropiaGen(infoGenes[i][j], numTrabajos);
    }
  }

  res /= (numMaquinas*numTrabajos);

  return res;
}

/** 
 * arr es la frecuencia de aparicion para cada posible valor del gen
 **/
double Diversidad::calcularEntropiaGen(double *arr,  int n) {
  double res = 0;

  double logBase  = log(n*1.0); 
  for(int i =0; i < n; i++) {
    if (arr[i] != 0) {
      res += (-1)*arr[i]*(log(arr[i])/logBase);
    }
  }

  return res;
}

double Diversidad::similitudInds(Solucion *s1, Solucion *s2) {

  vector<vector<int>> perSol1 = s1->getPermutacionSol();
  vector<vector<int>> perSol2 = s2->getPermutacionSol();

  double res = UtilJS::similitudPerms(perSol1, perSol2);
  return res;
}

double Diversidad::distanciaInds(Solucion *s1, Solucion *s2, int tipo) {
  double totalOps = (s1->getNumMaquinas()*s1->getNumTrabajos()); 
  double distancia=0;

  switch(tipo) {
    case 1:
      distancia = Diversidad::distanciaPosInds(s1, s2);
      break;
    case 2:
      distancia = Diversidad::distanciaDifsInds(s1, s2);
      break;
    case 0:
    default:
      distancia= (totalOps - Diversidad::similitudInds(s1, s2));
      break;
  }

  return distancia;
}

double Diversidad::distanciaPosInds(Solucion *s1, Solucion *s2) {
  vector<vector<int>> perSol1 = s1->getPermutacionSol();
  vector<vector<int>> perSol2 = s2->getPermutacionSol();

  return UtilJS::difPosPerms(perSol1, perSol2);
}

double Diversidad::distanciaDifsInds(Solucion *s1, Solucion *s2) {
  vector<vector<int>> perSol1 = s1->getPermutacionSol();
  vector<vector<int>> perSol2 = s2->getPermutacionSol();

  return UtilJS::numPosDefPerms(perSol1, perSol2);
}


double Diversidad::distNCSInds(Solucion *s1, Solucion *s2) {
  vector<vector<int>> perSol1 = s1->getPermutacionSol();
  vector<vector<int>> perSol2 = s2->getPermutacionSol();

  vector< map<int, pair<int,int> > > ncs = UtilJS::NCS_vint(perSol1, perSol2);

  double dist=0;
  for(int i=0; i < ncs.size(); i++) {
    dist += ncs[i].size();
  }

  return dist;
}

double Diversidad::contribucionDivInd(Solucion *s, vector<Solucion *> & pob, int total, int tipo) {
  double distanciaTmp = 0;
  double distancia_min = DBL_MAX;
  double totalOps = (s->getNumMaquinas()*s->getNumTrabajos());

  if (s == NULL) return 0;

  for(int i=0; i < total && i < pob.size(); i++) {
    if (s == pob[i]) continue;

    distanciaTmp = Diversidad::distanciaInds(s, pob[i], tipo);

    distancia_min = distanciaTmp < distancia_min ? distanciaTmp : distancia_min;
  }

  return distancia_min;
}

double Diversidad::calcularDiversidadPob(vector<Solucion *> & pob, int tamPob, int tipo, bool actualiza) {
  double promedio = 0;

  double distancia_min = 0;

  double minDistMin = DBL_MAX;
  double maxDisMin = DBL_MIN;

  tamPob = tamPob <= 0 ? pob.size() : tamPob;

  for(int i=0; i < tamPob; i++) {  
    distancia_min = Diversidad::contribucionDivInd(pob[i], pob, tamPob, tipo);

    if (actualiza) {
      switch(tipo) {
        case 1:
          pob[i]->distMinPob1 = distancia_min;
          break;
        case 2:
          pob[i]->distMinPob2 = distancia_min;
          break;
        default:
        case 0:
          pob[i]->distMinPob0 = distancia_min;
          break;
      }
    }

    promedio += distancia_min;
    minDistMin = minDistMin > distancia_min ? distancia_min : minDistMin;
    maxDisMin = maxDisMin < distancia_min ? distancia_min : maxDisMin;
  }


  promedio /= tamPob;

  return promedio;
}

bool Diversidad::comparaMakespanSols(Solucion *s1, Solucion *s2) {

  if (s1 == NULL) return s2;
  if (s2 == NULL) return s1;

  if (s1->getMakespan() == s2->getMakespan()) {
    return s1->valRdm < s2->valRdm;
//    return s1->numItTS > s2->numItTS;
//    || s1->getPesoOpsCriticas() < s2->getPesoOpsCriticas();
  }

  return s1->getMakespan() < s2->getMakespan();
}