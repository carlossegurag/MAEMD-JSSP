#include <iostream>
#include <string>
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand
#include <bits/stdc++.h>

#include "AlgoritmoGenetico.hpp"
#include "Solucion.hpp"
#include "GJobShop.hpp"
#include "JobShop.hpp"
#include "UtilJS.hpp"
#include "TabuSearch.hpp"
#include "PathRelinking.hpp"

using namespace std;

int main(int argc, char **argv) {

  unsigned int seed = time(NULL);
//  seed = 1541626152;
  srand(seed);

  int ejecucionLocal = argc > 7 ? atoi(argv[7]) : 1;

  string ruta="../";
  ruta = ejecucionLocal == 1 ? "" : ruta;

  string nombre_archivo(ruta + argv[1]);
  string fileInst(argv[1]);
  string nombreBase = ruta + argv[3];

  auto *coutbuf = std::cout.rdbuf();
  ofstream out(nombreBase + "_" + to_string(seed) + "_" + fileInst.substr(5));
  if (ejecucionLocal != 1) {
    cout.rdbuf(out.rdbuf());
  }

  cout << "#Seed: " << seed << "\n" << std::flush;

  int valOpt = atoi(argv[2]);

  JobShop *p = new JobShop(nombre_archivo);

  cout << "#Instancia: " << nombre_archivo << "\n" << std::flush;

  int tipoSelecc =atoi(argv[4]);
  int tipoCruza = atoi(argv[5]);

  int tipoLS = argc > 6 ? atoi(argv[6]) : 0;

  int tamPob = 30;
  AlgoritmoGenetico AG(p, tamPob, valOpt);
  AG.tipoMejora = tipoLS;

  double maxTime = argc > 8 ? atof(argv[8]) : 60; //60*60*24;

  int maxSinMejora = argc > 9 ? atoi(argv[9]) : 0;

  Solucion::evalExacta = argc > 10 && atoi(argv[10]) == 1;
  Solucion::calcularCotas = argc > 11 && atoi(argv[11]) == 1;
  Solucion::soloFact = argc > 12 && atoi(argv[12]) == 1;

  cout << "#Tamanio Pob: " << tamPob << "\n";
  cout << "#Max tiempo Ejecucion: " << maxTime << " segs \n";
  cout << "# Path Relinking + Tabu Search (N5 y N7) \n" << std::flush;

  TabuSearch::generarLog = false;

  string fileOutPob = nombreBase + "_" + to_string(seed) + "_pobs.txt";

  Solucion *best = AG.TSPR(maxTime);

  vector<vector<int>> permBest = best->getPermutacionSol();

  UtilJS::guardarPerms( permBest , nombreBase + "_sols.txt");

  ofstream filePerms;
  filePerms.open((nombreBase + "_res.txt").c_str(), 
    std::ofstream::out | std::ofstream::app) ;
  if (filePerms.is_open()) {
    filePerms << argv[1] << " " << seed << " " << p->getNumTrabajos() 
      << "x" << p->getNumMaquinas() << " " << valOpt << " "
       << best->getMakespan() << " " << AG.numIter 
      << " " << AG.time_search;

    filePerms << "\n";
    filePerms.flush();
    filePerms.close();
  }

  delete p;

  //if (ejecucionLocal != 1) {
    cout.rdbuf(coutbuf);
  //}
    if (ejecucionLocal != 1) {
      out.flush();
      out.close();
    }

  return 0;
}