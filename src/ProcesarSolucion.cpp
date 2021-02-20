#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

#include "GJobShop.hpp"
#include "Operacion.hpp"
#include "JobShop.hpp"
#include "Solucion.hpp"
#include "UtilJS.hpp"
#include "TabuSearch.hpp"
#include "Diversidad.hpp"
#include "AlgoritmoGenetico.hpp"
#include "PathRelinking.hpp"

using namespace std;


int main(int argc, char **argv) {

  string nombre_archivo(argv[1]);
  int valOpt = atoi(argv[2]);
  string archivoSol(argv[3]);

  int numJobs=0;
  int numMachs=0;

  int makespan=0;

  Solucion::evalExacta = 1;
  Solucion::calcularCotas = 0;
  Solucion::soloFact = 0;
  TabuSearch::generarLog = false;

  JobShop *p = new JobShop(nombre_archivo);
  GJobShop *g = new GJobShop(p);
  Solucion *s = new Solucion(g, valOpt);

  int totalOPs = s->getNumTrabajos()*s->getNumMaquinas();

  ifstream filePerms;
  filePerms.open(archivoSol.c_str());

  cout << "# Cargando archivo con sol inicial: " << archivoSol << "\n";
  vector<vector<int>> sIni;

  if (argc > 4 && atoi(argv[4]) == 1 ) 
    sIni = UtilJS::leerPerms(filePerms, s->getNumTrabajos(), s->getNumMaquinas(), g->ops);
  else
    sIni = UtilJS::leerPerms(filePerms);

  filePerms.close();
    
  s->setPermutacionSol(sIni, true); 

  Solucion::tipoReparacion = 2;

  int makesIni = s->getMakespan();
  cout << "Makespan Sol Leida: " << makesIni << "\n";
  //cout << "# Sol INI: \n" << s->getPrintPerms() << "\n";

//  cout << "Rutas: \n" << s->toString() << "\n";
/*
  for(int i=1; i < 2000; i +=2) {
    s->permTS.clear();
    s->numItTS=0;

    TabuSearch ts; 
    ts.tabuSearch(s, 7, 0, 0, i);

    vector<vector<int>> permTS = s->permTS;
    GJobShop *gTS = new GJobShop(p);
    Solucion *sTS = new Solucion(gTS, valOpt);
    sTS->setPermutacionSol(permTS, true);

    sTS->valOptimo = 6638;

    int totalOps = s->getNumMaquinas()*s->getNumTrabajos();

    vector<vector<int>> permAct = sTS->getPermutacionSol();
    int similitudConBest = totalOps - UtilJS::similitudPerms(sIni, permAct); 
    int similitudConBest2 = UtilJS::difPosPerms(sIni, permAct); 
    int similitudConBest3 = UtilJS::numPosDefPerms(sIni, permAct);

    cout << "IterTS=" << i << "Mks: " << sTS->getMakespan() << ", dist-BKS: " 
      << similitudConBest << " " << similitudConBest2 << " " << similitudConBest3 << " ";


    TabuSearch ts2; 
    ts2.tabuSearch(sTS, 7, 0, 0, 6000);

    permAct = sTS->getPermutacionSol();
    similitudConBest = totalOps - UtilJS::similitudPerms(sIni, permAct); 
    similitudConBest2 = UtilJS::difPosPerms(sIni, permAct); 
    similitudConBest3 = UtilJS::numPosDefPerms(sIni, permAct);

    cout << " , Mks2: " << sTS->getMakespan() << ", dist2-BKS: " 
      << similitudConBest << " " << similitudConBest2 << " " 
      << similitudConBest3 << " , itTS: " << ts2.numIt << "\n";

  }
*/


  // *************** PRUEBAS REPARACION TABU SEARCH *************
/*  Solucion::evalExacta = 1; 
  Solucion::calcularCotas = 0; 
  Solucion::soloFact = 0;

  Solucion::tipoReparacion=2;

  string archivoSol2(argv[5]);

  ifstream filePerms2;
  filePerms2.open(archivoSol2);
  vector<vector<int>> perm2 = UtilJS::leerPerms(filePerms2);

  GJobShop *g2 = new GJobShop(p);
  Solucion *s2 = new Solucion(g2, valOpt);
  s2->setPermutacionSol(perm2, true);
  cout << "Makespan Sol Leida: " << s2->getMakespan() << "\n";

  TabuSearch::generarLog=true;

  TabuSearch TS; 
  TS.tabuSearch(s2, 7, 0, 0, 100);

  GJobShop *gRes = new GJobShop(p);
  Solucion *sRes = new Solucion(gRes, valOpt);

  int totalOps = s->getNumMaquinas()*s->getNumTrabajos();

  vector<vector<int>> permAct = s->getPermutacionSol();
  int similitudConBest = totalOps - UtilJS::similitudPerms(sIni, perm2); 
  int similitudConBest2 = UtilJS::difPosPerms(sIni, perm2); 
  int similitudConBest3 = UtilJS::numPosDefPerms(sIni, perm2);

  cout << "Distancias Iniciales: " << similitudConBest 
    << " " << similitudConBest2 << " " << similitudConBest3 << "\n";

  PathRelinking::pathRelinking(s, s2, sRes, 1000);
*/
 
  // ******************** VECINOS DE BKS ********************* 

  Solucion::evalExacta = 1; 
  Solucion::calcularCotas = 0; 
  Solucion::soloFact = 0;

  int numErrores=0;
  int numErrosFAct=0;
  int numNoFactibles=0;
  int tipoN = 7;

  vector<Movimiento> movs = s->getMovimientosN7(true);
  cout << "Movimientos N" << tipoN << " desde la SOl inicial: " << movs.size() << "\n";
  for(int i=1334; i < movs.size(); i++) {
    s->setPermutacionSol(sIni, true);
    s->numItTS = 0;
    s->listaTabu.clear();
    cout << "#i=" << i  << movs[i].toString() << ", mksAct: " << s->getMakespan() << "\n";

    Operacion * V_ANT = movs[i].op2->getMachAnterior();
    Operacion * U_SIG = movs[i].op1->getMachSiguiente();

    movs[i].eval = s->aplicarCambio(movs[i]); 
    
    vector<vector<int>> permAct = s->getPermutacionSol();

    double dist1 = totalOPs - UtilJS::similitudPerms(sIni, permAct);
    double dist2 = UtilJS::difPosPerms(sIni, permAct);
    double dist3 = UtilJS::numPosDefPerms(sIni, permAct);

    if (i==1334) {
      cout << "# Sol Act: \n" << s->getPrintPerms() << "\n";
      //cout << "Rutas: \n" << s->toString() << "\n";
//      break;
    } 


    int esFact = s->esFactible() ? 1 : 0;
    int numIt= 0;
    if (esFact) {
//      s->recalcularBloques();      
      // movs[i].op1 = movs[i].op2;
      // movs[i].op2 = V_ANT;
      // movs[i].tipo=1;

      // s->aplicarCambio( movs[i]);
      Movimiento bestMov = s->getBestN(s->listaTabu, i, 0, movs[i].eval, movs[i].eval, true, tipoN);

      //bestMov.op1 = U_SIG;
      //bestMov.op2 = movs[i].op1;
      //bestMov.tipo = 2;

      s->aplicarCambio( bestMov );

      cout << "#i=" << i << ", bestMov: " << bestMov.toString() << ", mksAct: " << s->getMakespan() << "\n";
      
    }

//    s->calcularMakespan(true);

    if (!esFact) numNoFactibles++;

    if (esFact && s->getMakespan() != makesIni) {

      TabuSearch TS; 
     // TS.tabuSearch(s, tipoN, 0, 0, 200);
      numIt = TS.numIt;

      cout << "#i=" << i  << movs[i].toString() << ", mksAct: " << s->getMakespan() << "\n";

      cout << "#i=" << i  << movs[i].toString() << ", mksAct: " << s->getMakespan() 
        << ", esFact: " << esFact << ", IterTS: " << numIt 
        << ", dists: " << dist1 << " " << dist2 << " " << dist3 << "\n";

      if (s->getMakespan() != makesIni) numErrosFAct++;
    }


    if (s->getMakespan() != makesIni ) numErrores++;

    break;
  } // Fin for movimientos

  cout << "Num No Factibles: " << numNoFactibles << "\n";
  cout << "Num Errores: " << numErrores << "\n";
  cout << "Num Errores FAct: " << numErrosFAct << "\n";

  // ******************** FIN VECINOS DE BKS *********************  

  delete p;
  
  return 0;
} 