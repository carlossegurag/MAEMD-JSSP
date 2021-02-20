#include <iostream>
#include <cstdlib>
#include <cmath>
#include <utility> // std:pair  
#include <algorithm> // std::random_shuffle
#include <climits>  // INT_MAX
#include <time.h>
#include <unistd.h>

#include "IterativeSearch.hpp"
#include "UtilJS.hpp"
#include "Movimiento.hpp"

using namespace std;

int IterativeSearch::perturbarSolucionN(Solucion *s, int tipoN, int maxMovs, bool soloMejora) {

  int numMovs = 0;
  int numMaq = 0;

  int op1 = 0;
  int op2=0;

  deque<Movimiento>  listaTabu;
  bool checkTabu = false;
  int evalMov = INT_MAX;

  int soloFactAnt = Solucion::soloFact;
  int evalExactaAnt = Solucion::evalExacta;
  int calcularCotasAnt = Solucion::calcularCotas;

  Solucion::soloFact=1; // No checar factibilidad
  Solucion::evalExacta = 0; // No realizar evaluacion exacta
  Solucion::calcularCotas = 1; // Evaluación por estimación

  while (numMovs < maxMovs) {

    numMaq = rand() % s->getNumMaquinas();

    op1 = rand() % s->getNumTrabajos();
//    op2 = rand() % s->getNumTrabajos();
    op2 = op1 == 0 || (op1 != s->getNumTrabajos()-1 && rand()%2== 0) ? op1+1 : op1-1;
//    while (op2 == op1) op2 = rand() % s->getNumTrabajos();

    if (op2 < op1) swap(op1, op2);

    /* ***************** Movimientos de N7 *************** */
    // (3) insertar v a la izquierda de u
    Movimiento mInsertIzq(0, s->perms[numMaq][op1], s->perms[numMaq][op2]);
    mInsertIzq.maquina = numMaq;
    mInsertIzq.indOp1 = op1;
    mInsertIzq.seq.clear();
    for(int i=op1; i <= op2; i++) {
      mInsertIzq.seq.push_back(s->perms[numMaq][i]);
    }

    Operacion *JS_U = s->perms[numMaq][op1]->getJobSiguiente();
    Operacion *JP_V = s->perms[numMaq][op2]->getJobAnterior();

    int c_sj = JS_U != NULL ? JS_U->getTInicio() + JS_U->getDuration() : s->makespan;
    int c_pj = JP_V != NULL ? JP_V->getTInicio() : 0;
 
    // Checar Factibilidad:
    if ( c_sj <= c_pj ) continue; // el movimiento podria ser infactible

    checkTabu = false;
    mInsertIzq.eval = s->evalMovIzquierda(mInsertIzq, true, false, false,
        checkTabu, listaTabu, 0, s->getMakespan()+1);

    if (mInsertIzq.eval != INT_MAX && // El movimiento es Factible
        (!soloMejora || ( soloMejora && mInsertIzq.eval < s->getMakespan() )) ) {
//      cout << "pre Makespan Actual: " << s->getMakespan() << "\n";
      s->aplicarCambio(mInsertIzq);
      //break;
//      cout << "Makespan Actual: " << s->getMakespan() << "\n";
    }
    numMovs++;

    if ( (op2 - op1) == 1 ) continue;

    // (4) mover u a la derecha v
    Movimiento mMovDer(4, s->perms[numMaq][op1], s->perms[numMaq][op2]);
    mMovDer.maquina = numMaq;
    mMovDer.indOp1 = op1;
    mMovDer.seq = mInsertIzq.seq;
  
    checkTabu = false;
    mMovDer.eval = s->evalMovDerecha(mMovDer, true, false, true,
        checkTabu, listaTabu, 0, s->getMakespan()+1);

    //cout << "pertubacion : " << mMovDer.toString() << "\n";

    if (mMovDer.eval != INT_MAX && // El movimiento es Factible
        (!soloMejora || ( soloMejora && mMovDer.eval < s->getMakespan() )) ) {
//      cout << "pre2 Makespan Actual: " << s->getMakespan() << "\n";
      s->aplicarCambio(mMovDer);
//      cout << "Makespan Actual2: " << s->getMakespan() << "\n";
      //break;
    }

    numMovs++;
  }

  Solucion::soloFact=soloFactAnt;
  Solucion::evalExacta = evalExactaAnt; // No realizar evaluacion exacta
  Solucion::calcularCotas = calcularCotasAnt; // Evaluación por estimación

//  if (s->no)

  return s->getMakespan();
}

int IterativeSearch::perturbarSolucion(Solucion *s) {

  vector<vector<int>> solIni = s->getPermutacionSol();

  vector<int> maquinas(solIni.size());
  for(int i=0; i < solIni.size(); i++) {
    maquinas[i] = i;
  }
  std::next_permutation(maquinas.begin(), maquinas.end() );

  vector<Operacion *> &N =  s->getOperaciones();

  bool pertubacion = false;

  int i=0;

  int iniBloque = 0;
  int tamBloque = 0;

  int indOp = 0;

  int makespan = s->getMakespan();

  Operacion *op;

  vector< pair<int, int> > bloques;

  int ranBloque = 0;

  while (!pertubacion && i < solIni.size() ) {
    iniBloque = 0;
    tamBloque = 0;
    indOp = 0;
    bloques.clear();

    // Buscamos el inicio de un Bloque en la maquina:
    while ( indOp < solIni[i].size() ) {
      op = N[ solIni[maquinas[i]][iniBloque] ];
      if ( (op->getTInicio() + op->getRT()  ) == makespan ) {
        iniBloque = tamBloque == 0 ? indOp : iniBloque;
        tamBloque ++;
      } else if (tamBloque > 1) {
        bloques.push_back(make_pair(iniBloque, tamBloque));
        //break;
        tamBloque = 0;
      } else {
        tamBloque = 0;
      }

      indOp++;
    }

    // Si no encontramos un bloque, elegimos otra maquina:
    if (bloques.size() == 0 ) {
      i++;
      continue;
    }

    ranBloque = bloques.size() == 1 ? 0 : (rand() % bloques.size());
    iniBloque = bloques[ranBloque].first;
    tamBloque = bloques[ranBloque].second;

    std::random_shuffle ( solIni[maquinas[i]].begin() + iniBloque, 
        solIni[maquinas[i]].begin() + iniBloque + tamBloque );
    pertubacion = true;

  }

  if (!pertubacion && i == solIni.size()) {
    int mRand = rand() % solIni.size();
    // generar permutacion aleatoria
    random_shuffle (solIni[mRand].begin(), solIni[mRand].end() );
  }

  // Asignamos
  s->setPermutacionSol(solIni, false);

  // reparamos
  s->reparar();

  return s->calcularMakespan(true);
}

int IterativeSearch::iterativeSearch(Solucion *s, double maxTime) {

  int bestMakespan = s->getMakespan();
  vector<vector<int>> bestSol = s->getPermutacionSol();
  vector<vector<int>> permAct = s->getPermutacionSol();

  int makespanAct = s->getMakespan();

  /* guardara al Mejor vecino encontrado */
  Movimiento bestMov(0, NULL, NULL);

  clock_t tIni = clock();
  clock_t t;
  time_search =  0;

  bool cambia = false;

  int tipoN = 7;

  numIt = 0;

  int numPerturbaciones = 0;
  int iterBest = 0;
  int maxDistBest = 0;
  int maxIterMejorar = 0;
  double similitudConBest = 0;
  int totalOps = s->getNumMaquinas()*s->getNumTrabajos();

  sinMejora = 0;

  while (time_search < maxTime && bestMakespan > s->getValOptimo() ) {
    sinMejora++;
    cambia = false;

    bestMov = s->getBestN(s->listaTabu, numIt, 0, bestMakespan, makespanAct, true, tipoN);

    // Imprimir datos de la busqueda
    if (bestMov.eval < bestMakespan || (numIt % 1000) == 0 ) {

      permAct = s->getPermutacionSol();
      similitudConBest = totalOps - UtilJS::similitudPerms(bestSol, permAct);

      // Imprimir Datos
      cout << numIt << " " << makespanAct 
        << " " << bestMakespan << " " << time_search 
        << " " << tipoN << " " << numPerturbaciones
        << " " <<  similitudConBest << " " << iterBest 
        << " " << (numIt - iterBest)
        << " " << maxDistBest << " " << maxIterMejorar << "\n" << std::flush; 
    }

    if (bestMov.eval < makespanAct) {
      makespanAct = s->aplicarCambio(bestMov);
      cambia = true;
      if (makespanAct  < bestMakespan) {
        bestMakespan = makespanAct;
        bestSol = s->getPermutacionSol();

        maxIterMejorar = (numIt - iterBest) > maxIterMejorar ? 
          (numIt - iterBest) : maxIterMejorar;
        iterBest = numIt;

        maxDistBest = similitudConBest > maxDistBest ? 
          similitudConBest : maxDistBest;
        sinMejora = 0;
      }
    } 

    if (!cambia) {
      tipoN = 7;
      //tipoN = tipoN == 7 ? 5 : 7;

      if (sinMejora % 1000 == 0) {
//        cout << "Aplicando PERTURBACIONES,...\n";
        makespanAct = IterativeSearch::perturbarSolucionN(s, 7,  s->getNumTrabajos()*s->getNumMaquinas(), true );
        numPerturbaciones++ ;
        if (!s->esFactible()) {
          exit(0);
          s->reparar();
          s->calcularMakespan(true);
        }
      }

    }

    t = clock() - tIni;
    time_search = ((double)t)/(CLOCKS_PER_SEC);
    numIt++;

  }

  permAct = s->getPermutacionSol();
  similitudConBest = totalOps - UtilJS::similitudPerms(bestSol, permAct);

  // Imprimir datos de la busqueda
   cout << numIt << " " << makespanAct 
        << " " << bestMakespan << " " << time_search 
        << " " << tipoN << " " << numPerturbaciones
        << " " <<  similitudConBest << " " << iterBest 
        << " " << (numIt - iterBest)
        << " " << maxDistBest << " " << maxIterMejorar << "\n" << std::flush;

  s->setPermutacionSol(bestSol, true);

  this->totalBloqueos =  numPerturbaciones;
  numNoFactible = s->numNoFactible;
  numFactibles = s->numFactibles;

//  cout << "# bestMakespan: " << s->getMakespan() << "\n" << std::flush;

  return s->getMakespan();
}