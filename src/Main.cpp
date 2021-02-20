#include <iostream>
#include <fstream>      // std::filebuf
#include <map>
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand
#include <fstream>

#include "JobShop.hpp"
#include "GJobShop.hpp"
#include "Solucion.hpp"
#include "TabuSearch.hpp"
#include "IterativeSearch.hpp"
#include "UtilJS.hpp"

using namespace std;

int main(int argc, char **argv) {

  unsigned int seed = time(NULL);
  //seed=1550164649;
  //seed=1572283084;
  srand(seed);  

  int ejecucionLocal = argc > 7 ? atoi(argv[7]) : 1;

  string ruta="../";
  ruta = ejecucionLocal == 1 ? "" : ruta;

  string fileInst(argv[1]);

  string nombre_archivo(ruta + argv[1]);
  int valOpt = atoi(argv[2]);
  string nombreArchivoSol(ruta + argv[3]);

  auto *coutbuf = std::cout.rdbuf();
  ofstream out(nombreArchivoSol + "_" + to_string(seed) + "_" + fileInst.substr(5));
  if (ejecucionLocal != 1) {
    cout.rdbuf(out.rdbuf());
  }

  cout << "#Seed: " << seed << "\n" << std::flush;
  cout << "# Archivo a procesar: " << nombre_archivo << "\n" << std::flush;

  JobShop *p = new JobShop(nombre_archivo);
  GJobShop *g = new GJobShop(p);

  Solucion *s = new Solucion(g, valOpt);    

  if (argc > 13) {
    ifstream filePerms;
    string archivoSol(argv[13]);
    filePerms.open(archivoSol.c_str());

    cout << "# Cargando archivo con sol inicial: " << archivoSol << "\n";
    vector<vector<int>> sIni;

    if (argc > 14 && atoi(argv[14]) == 1 ) 
      sIni = UtilJS::leerPerms(filePerms, s->getNumTrabajos(), s->getNumMaquinas(), g->ops);
    else
      sIni = UtilJS::leerPerms(filePerms);
    s->setPermutacionSol(sIni); 
    filePerms.close();

    cout << "# Sol INI: \n" << s->getPrintPerms() << "\n";
  }

  cout << "#Instancia: " << nombre_archivo << ", #jobs: " << s->getNumTrabajos() 
    << ", #machs: " << s->getNumMaquinas() << "\n" << std::flush;


  vector<vector<int>> permIni = s->getPermutacionSol();

  int makespanIni = s->getMakespan();
  cout << "#makespanIni: " << makespanIni << "\n" << std::flush;

  TabuSearch TS;
  IterativeSearch IS;

  double maxTime = argc > 8 ? atof(argv[8]) : 60*60; //60*60*24;

  int tipoLS = 0; // argc > 6 ? atoi(argv[6]) : 0;
  int tipoN = argc > 6 ? atoi(argv[6]) : 0;

  int maxSinMejora = argc > 9 ? atoi(argv[9]) : 0;
  cout << "# MaxTime: " << maxTime << "\n" << std::flush;
  cout << "# maxSinMejora: " << maxSinMejora << "\n" << std::flush;

  int totalIter = 0;

  Solucion::evalExacta = 0; // argc > 10 ? atoi(argv[10]) : 0;
  Solucion::calcularCotas = 1; // argc > 11 ? atoi(argv[11]) : 1;
  Solucion::soloFact = 1; //argc > 12 ? atoi(argv[12]) : 1;  

  cout << "#config Eval, exacta: " << Solucion::evalExacta 
    << ", calcCotas: " << Solucion::calcularCotas 
    << ", soloFact: " << Solucion::soloFact << "\n" << std::flush;

  bool aplicarPerturbaciones = argc > 10 ? atoi(argv[10]) : 0;

  cout << "#Aplicar Perturbaciones: " << aplicarPerturbaciones << "\n";
  TabuSearch::generarLog = true;

  switch(tipoLS) {
    case 5: /* Escalada N5 */
      cout << "# Busqueda Local por EScalada en N5 \n" << std::flush;
      totalIter = TS.escalada(s, 5);
      break;
    case 57:
      cout << "# Busqueda Local por EScalada en N5 \n" << std::flush;
      totalIter = TS.escalada(s, 5);
    case 7: /* Escalada N7 */
      cout << "# Busqueda Local por EScalada en N7 \n" << std::flush;
      totalIter += TS.escalada(s, 7);
      break;

    case 1:
      cout << "# Busqueda Local Iterativa, N5 y N7 \n" << std::flush;
      totalIter = IS.iterativeSearch(s, maxTime);
      TS.numIt = IS.numIt;
      TS.totalBloqueos = IS.totalBloqueos;
      TS.numFactibles = IS.numFactibles;
      TS.numNoFactible = IS.numNoFactible;
      TS.promRep = IS.promRep;
      TS.maxRep = IS.maxRep;
      TS.minRep = IS.minRep;
      TS.time_search = IS.time_search;
      break;

    case 11: /* Tabu Search N1 */
      cout << "# Busqueda Tabu, N1 \n" << std::flush;
      totalIter = TS.tabuSearch(s, 1, maxTime, 0, maxSinMejora);
      break;

    case 0: /* Tabu_Search N7 */
    default:
      cout << "# Busqueda Tabu,  N" << tipoN << " \n" << std::flush;
      totalIter = TS.tabuSearch(s, tipoN, maxTime, 0, maxSinMejora, aplicarPerturbaciones);
      break;
  }


  vector<vector<int>> permFin = s->getPermutacionSol(); 

  int bestLS = s->getMakespan();


  if (ejecucionLocal != 1) {
    UtilJS::guardarPerms(permIni, nombreArchivoSol+"_sols.txt");
    UtilJS::guardarPerms(permFin, nombreArchivoSol+"_sols.txt");
  }
  
  ofstream filePerms;
  filePerms.open((nombreArchivoSol + "_res.txt").c_str(), 
    std::ofstream::out | std::ofstream::app) ;
  if (filePerms.is_open()) {
    filePerms << argv[1] << " " << seed << " " << s->getNumTrabajos() 
      << "x" << s->getNumMaquinas() << " " << valOpt << " "
      << makespanIni <<  " " << bestLS << " " << TS.time_search
      << " " << UtilJS::similitudPerms(permIni, permFin)
      << " " << TS.tamLT << " " << TS.totalVecinos 
      << " " << TS.totalEvalsCI << " " << TS.totalEvals
      << " " << TS.numIt << " " << TS.totalBloqueos << " " << TS.minRep 
      << " " << TS.maxRep << " " << TS.promRep << " " << TS.numFactibles
      << " " << TS.numNoFactible << "\n";
    filePerms.flush();
    filePerms.close();
  }

  delete s;
//  delete g;
  delete p;  

  cout.rdbuf(coutbuf);

  return 0;
} 