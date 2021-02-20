#include <iostream>
#include <cstdlib>
#include <cmath>
#include <map>
#include <deque>
#include <utility> // std:pair  
#include <algorithm>    // std::random_shuffle
#include <climits>  // INT_MAX
#include <time.h>
#include <unistd.h>
#include <unordered_map>

#include "TabuSearch.hpp"
#include "IterativeSearch.hpp"
#include "UtilJS.hpp"
#include "Movimiento.hpp"

using namespace std;

class SolutionHash {

  public: 
    size_t operator()(const vector<vector<int>> &sol) const {
      return (size_t) UtilJS::hashPerms(sol);
    }

};

//ostream & TabuSearch::fout = cout;
bool TabuSearch::generarLog = false;
int TabuSearch::simpl1=0;
int TabuSearch::simpl2=0;

int TabuSearch::tabuSearch(Solucion *s, int tipoN, 
  double max_time, int maxIters, int maxSinMejora, bool aplicarPerturb) {  

  //tabuTenure = 3;

  int totalOps = s->getNumMaquinas()*s->getNumTrabajos();
  // init variable para Penalizacion
  vector<  int > & x = s->x;
  x.clear();
  x.resize( totalOps + 2 );

  if (max_time <= 0 && maxIters <= 0 && maxSinMejora <= 0) maxSinMejora = 12000;

  TabuSearch::simpl1=0;
  TabuSearch::simpl2=0;

  s->numNoFactible=0;
  s->numFactibles=0;
  s->numEvalsCI=0;
  s->numEvals=0;

  clock_t tIni = clock();

  /* Mejor solucion encontrada */
  int bestMakespan = s->getMakespan();
  //int bestEvalRutas = s->getNumRutas();
  //int bestNumBloqs = s->getNumBloques();
  int bestPesoOps = s->getPesoOpsCriticas();

  vector<vector<int>> bestSol = s->getPermutacionSol();

  deque<Movimiento> listaTabu_TS;
  deque<Movimiento> & listaTabuTS = s->listaTabu;

  /* Solucion actual */
  if (s->numItTS != 0) {
    //cout << "#\t\t cont TS Iter: " << s->numItTS << ".... \n";
    s->setPermutacionSol(s->permTS, true);
  }

  vector<vector<int>> permAct = s->getPermutacionSol();

  pair<int, int> ultMov = make_pair(-1, -1);

  numIt = s->numItTS;

  int makespanAct = s->getMakespan();
  int pesoAct = bestPesoOps;

  /* guardara al Mejor vecino encontrado */
  Movimiento bestMov(0, NULL, NULL);
  Movimiento movAnt(0, NULL, NULL);

  int n = s->getNumTrabajos();
  int m = s->getNumMaquinas();

  int d1 = 5;
  int d2 = 12;
  int tt = 2;

  int RL = max( (makespanAct - bestMakespan)/d1, d2 );
  tamLT = tt + (rand() % RL );
  
  // Valor para L tomado de Zhang 2007 (2006), para N7
  int L = 10+n/m;

  if (tipoN == 1) // Valor para L tomado de Taillard 1993
    L = (n + m/2)*exp(-n/(5*m)) + ((n*m)/2)*exp(-(5*m)/n);

  int Lmin = L;
  int Lmax = ( n <= 2*m ) ? ((int) (L*1.4 )) :  ((int) (L*1.5)) ;

  tamLT = Lmin;
  tamLT += rand() % ( Lmax - Lmin + 1);

//  tamLT = 12;

  int tAnt_U = 0;
  int rtAnt_U = 0;

  int tAnt_V = 0;  
  int rtAnt_V = 0;

  if (generarLog)
    cout << "# TS, tam LT: " <<  tamLT << "\n";

  sinMejora = 0;
  bool mejora = false;

  totalBloqueos = 0;
  int bloqueosSimplificacion = 0;

  clock_t t = clock() - tIni;
  time_search = ((double)t)/(CLOCKS_PER_SEC);

  double timeBest = time_search;

  unordered_map< vector<vector<int>>, int, SolutionHash> solVisitadas;
//  solVisitadas[s->getPermutacionSol()] = 1;
  unordered_map<vector<vector<int>>, int, SolutionHash>::const_iterator itSolVis;
  
  minRep = 0;
  maxRep = 1;
  promRep = 0;
  int cambia = false;
  int makespanAnt = makespanAct;

  double similitudConBest = 0;
  double similitudConBest2 = 0;
  double similitudConBest3 = 0;
  int iterBest = 0;
  int maxIterMejorar = 0; 
  int maxDistBest = 0;
  int maxDistBest2 = 0;
  int maxDistBest3 = 0;

  totalVecinos = 0;
  totalEvals = 0;
  totalEvalsCI = 0;

  if (generarLog)
    cout << "#Iter MakespanAct Best time_search tipoVec numMovs numEvalsCI numEvals tamLT simBest itBest timeBest itAlBest maxDistBest maxIterMejorar\n" << std::flush;

  bool simplificacion = false;
  int numPertur = s->getNumTrabajos()*s->getNumMaquinas();

  do {

    t = clock() - tIni;
    time_search = ((double)t)/(CLOCKS_PER_SEC);

    numIt++;

    sinMejora++;
    mejora = false;

//    cout << "Sol act: \n" << s->getPrintPerms() << "\n";

    bestMov = s->getBestN(listaTabuTS, numIt, tamLT, bestMakespan, makespanAct, true, tipoN);

    // Si ya no se encontro ningun vecino permitido, 
    // cambia vecindad y salta iteracion
    if (bestMov.eval == INT_MAX) {
      totalBloqueos++;

      // si el bloque se produce despues de una simplificacion
      // se recalculan los bloques para genera todos los vecinos
      if (simplificacion) {
        bloqueosSimplificacion++;
        // Repetir iteracion
        numIt--;
        sinMejora--;
        simplificacion = false;
//        tipoN=7;

        // Recalcular todos los bloques   
        s->calcularMakespan(true);
      }
      continue;
    }

    makespanAnt = makespanAct;

    totalVecinos += s->numMovs;
    totalEvals += s->numEvals;
    totalEvalsCI += s->numEvalsCI;

      /* AJUSTE DINAMICO DEL TAMANIO DE LA LISTA TABU */
/*    if (bestMov.eval < makespanAct ) {
      tamLT--;
    } else {
      tamLT++;
    } 
    tamLT = (tamLT < L) ? L : tamLT;
    tamLT = (tamLT > Lmax) ? Lmax : tamLT; */

    bestMov.actualizarValoresUV();

    /* REALIZAR MOVIMIENTO */
    makespanAct = s->aplicarCambio(bestMov);
    cambia = true;

    if ( Solucion::evalExacta && bestMov.eval != makespanAct){
      cout << "Tipo: " << bestMov.tipo << endl;
      cout << "EO!!!!" << endl;
      cout << "Error interno" << endl;
      cout << "Son: " << bestMov.eval << " y " << makespanAct << endl;
      exit(-1);
    }

    if (tabuTenure == 3) {
      RL = max( (makespanAct - bestMakespan)/d1, d2 );
      tamLT = tt + (rand() % RL );
    }

    // Agregar bestMov a lista tabu
    TabuSearch::actualizarListaTabu(listaTabuTS, bestMov, numIt, tamLT);

    if (bestMov.tipo == 0 || bestMov.tipo == 2 || bestMov.tipo == 3) {
      x[ bestMov.op2->getId() ] += bestMov.seq.size() ;
    }
    if (bestMov.tipo == 0 || bestMov.tipo == 1 || bestMov.tipo == 4) {
      x[ bestMov.op1->getId() ] += bestMov.seq.size();
    }

    /* SIMPLIFICACION DE LA VECINDAD.... */
    if (makespanAct > makespanAnt)  {//  && bestMov.tipo != 0 )
      simplificacion=false;
      simplificacion = TabuSearch::simplificarVecindad(s, bestMov);
      //tipoN=7;
    }
    else
      simplificacion = false;

    movAnt = bestMov;

    // Imprimir datos de la busqueda
    if (generarLog 
      && (makespanAct <= bestMakespan 
/*         || (similitudConBest > maxDistBest
         || similitudConBest2 > maxDistBest2
         || similitudConBest3 > maxDistBest3) */  
//          || (makespanAct <= 6700) || (makespanAct > 7100)
         || (numIt % 10000) == 0 )  
      ) {

      permAct = s->getPermutacionSol();
      similitudConBest = totalOps - UtilJS::similitudPerms(bestSol, permAct); 
      similitudConBest2 = UtilJS::difPosPerms(bestSol, permAct); 
      similitudConBest3 = UtilJS::numPosDefPerms(bestSol, permAct); 

      cout << numIt << " " << makespanAct
        << " " << bestMakespan << " " << time_search 
        << " " << tipoN << " " << s->numMovs << " " << s->numEvalsCI << " " << s->numEvals
        << " " << tamLT << "  " <<  similitudConBest 

        << " " << similitudConBest2<< " " << similitudConBest3
        
        << " " << iterBest 
        << " " << timeBest << " " << (numIt - iterBest)
        << " " << maxDistBest << " " << maxIterMejorar << "\n" << std::flush;
    }

    if (makespanAct < bestMakespan) {
      mejora = true;
      timeBest = time_search;

      bestMakespan = makespanAct;
      bestSol =  s->getPermutacionSol();
      sinMejora=0;

      /* if ( similitudConBest > maxDistBest)
        maxDistBest = similitudConBest;

      if ( similitudConBest2 > maxDistBest2)
        maxDistBest2 = similitudConBest2;

      if ( similitudConBest3 > maxDistBest3)
        maxDistBest3 = similitudConBest3; */

      if ((numIt - iterBest) > maxIterMejorar)
        maxIterMejorar = numIt - iterBest;

      iterBest = numIt;
      numPertur = s->getNumTrabajos()*s->getNumMaquinas();
      //tipoN=5;
    }

    if (aplicarPerturb && sinMejora > 0 && (sinMejora % numPertur) == 0 ) {
      numPertur *= 2;
      cout << "Aplicando PERTURBACIONES,...\n";
      makespanAct = IterativeSearch::perturbarSolucionN(s, 7,  s->getNumTrabajos()*s->getNumMaquinas(), true );
      
      if (!s->esFactible()) {
        cout << "ERROR, NO es Factible ...!!!!\n";
        cout << "numIt " << numIt << "\n";
        break;
      }
    } 

    /* AJUSTE DINAMICO DE LA LISTA TABU  */
//    if (sinMejora >= 2500 && sinMejora % 2500 == 0) {
    if (tabuTenure == 0) {
      tamLT = L;
      tamLT += rand() % ( Lmax - Lmin + 1);
    }

    t = clock() - tIni;
    time_search = ((double)t)/(CLOCKS_PER_SEC);

  } while (
      (
      // Criterio de paro por iteraciones sin mejora
        ( maxSinMejora <= 0 || sinMejora < maxSinMejora )

        // Criterio de paro por val optimo
        &&  bestMakespan > s->getValOptimo() 

        // Criterio de paro por iteraciones
        && ( maxIters <= 0 || numIt < maxIters  )

        // Criterio de parada por tiempo
        && ( 
          max_time <= 0 
          // Solo se aplica si no se especifica paro por iteraciones
          //|| (maxSinMejora > 0 || maxIters > 0)
          || (  time_search < max_time )  // parada por tiempo
           ) 

    // La sig condicion garantiza que siempre terminamos en un optimo local
      ) || mejora 
    );


  //cout << "# Sol Final: \n" << s->getPrintPerms() << "\n";

  // Imprimir datos
  if (generarLog) {
    cout << numIt << " " << makespanAct
        << " " << bestMakespan << " " << time_search 
        << " " << tipoN << " " << s->numMovs << " " << s->numEvalsCI << " " << s->numEvals
        << " " << tamLT << " " // <<  similitudConBest 
        
        << " " << (totalOps - UtilJS::similitudPerms(bestSol, permAct)) 
        << " " << (UtilJS::difPosPerms(bestSol, permAct))
        << " " << (UtilJS::numPosDefPerms(bestSol, permAct))
        
        << " " << iterBest 
        << " " << timeBest << " " << (numIt - iterBest)
        << " " << maxDistBest << " " << maxIterMejorar << "\n" << std::flush;
  }

  s->permTS = s->getPermutacionSol();
  s->setPermutacionSol(bestSol, true);

  promRep = minRep != 0 ? (promRep+minRep)/minRep : 0;

  numNoFactible = s->numNoFactible;
  numFactibles = s->numFactibles;

  if (generarLog) {
    cout << "\n#Total Bloqueos: " << totalBloqueos << "\n";
    cout << "#Total Bloqueos por Simpl: " << bloqueosSimplificacion << "\n";
    cout << "# total de Simpl cas 1: " << TabuSearch::simpl1 << "\n";
    cout << "# total de Simpl cas 2: " << TabuSearch::simpl2 << "\n";
    cout << "#max Iter para Mejorar: " <<  maxIterMejorar << "\n";
    cout << "#Total de Vecinos Generados: " << totalVecinos << "\n";
    cout << "#Total de Vecinos Evaluados cotInf: " << totalEvalsCI << "\n";
    cout << "#Total de Vecinos Evaluados makespan: " << totalEvals << "\n";
    cout << "#numFactibles: " << numFactibles << ", numNOFact: " << numNoFactible << "\n";

    cout << "#minRep: " << minRep << ", maxRep: " << maxRep 
      << ", promRep: " << promRep << "\n";

    cout << "#esFactible: " << s->esFactible() << "\n";
    cout << "#makespan Final: " << s->getMakespan() << "\n" << std::flush;
  }

  //cout << "# TS " << s->getMakespan() << " " << max_time << " " << maxIters << " " << maxSinMejora << "\n";

  return s->getMakespan();
}

int TabuSearch::escalada(Solucion *s, int tipoN) {  

  /* Solucion actual */
  int makespanAct = s->getMakespan();
  int pesoAct = s->getPesoOpsCriticas();

  /* Mejor solucion encontrada */
  int bestMakespan = makespanAct;
  vector<vector<int>> bestSol = s->getPermutacionSol();

  int bestPesoOps = pesoAct;

  vector<vector<int>> permAct = s->getPermutacionSol();

  int maxIter = 1000000; //1000

  deque<Movimiento> listaTabu;

  /* guardara al Mejor vecino encontrado */  
  Movimiento bestMov(0, NULL, NULL);

  int n = s->getNumTrabajos();
  int m = s->getNumMaquinas();

  int numIt = 0;

  bool mejora = false;

  clock_t tIni = clock();
  clock_t t = clock() - tIni;
  double time_search = ((double)t)/(CLOCKS_PER_SEC);

  int totalOps = s->getNumMaquinas()*s->getNumTrabajos();
  double similitudConBest = 0;

  do {
    numIt++;

    t = clock() - tIni;
    time_search = ((double)t)/(CLOCKS_PER_SEC);

/*    permAct = s->getPermutacionSol();
    similitudConBest = totalOps - UtilJS::similitudPerms(bestSol, permAct); */

    mejora = false; 

    bestMov = s->getBestN(listaTabu, numIt, 0, bestMakespan, makespanAct, true, tipoN);


    // Si ya no se encontro ningun vecino, termina el ciclo
    if (bestMov.eval == INT_MAX) {
      break;
    } 

    if (bestMov.eval < bestMakespan) {
      makespanAct = s->aplicarCambio(bestMov);
      bestSol =  s->getPermutacionSol();

      bestMakespan = makespanAct;
      bestPesoOps = pesoAct;

      mejora = true;
    }

    // Imprimir datos de la busqueda
    if (generarLog) {
        cout << numIt << " " << makespanAct
        << " " << bestMakespan << " " << time_search 
        << " " << tipoN << " " << s->getNumRutas()
        << " " << s->getNumBloques() << " " << s->getTamPromBloques()
        << " " << s->getPesoOpsCriticas()
        << " " << "0" << " " <<  similitudConBest << " " << numIt 
        << " " << "0"
        << " " << similitudConBest << " " << "0" << "\n" << std::flush;
    }
        
  } while (mejora && bestMakespan > s->getValOptimo());

  s->setPermutacionSol(bestSol, true);


// Imprimir datos de la busqueda
  if (generarLog) {
    cout << numIt << " " << makespanAct
        << " " << bestMakespan << " " << time_search 
        << " " << tipoN << " " << s->getNumRutas()
        << " " << s->getNumBloques() << " " << s->getTamPromBloques()
        << " " << s->getPesoOpsCriticas()
        << " " << "0" << " " <<  similitudConBest << " " << numIt 
        << " " << "0"
        << " " << similitudConBest << " " << "0" << "\n" << std::flush;
  
    cout << "# Total de Iteraciones : " << numIt << "\n";
    cout << "# Mejor makespan encontrado: " << bestMakespan << "\n" << std::flush;
  }

  return s->getMakespan();
}

void  TabuSearch::actualizarListaTabu( deque<Movimiento> & listaTabuTS, 
  Movimiento & bestMov, int numIt, int tamLT )  {

  // Eliminamos los mov en la lista tabu que ya no estan prohibidos
    while (listaTabuTS.size() > 0 &&
      listaTabuTS.front().numIter < numIt) listaTabuTS.pop_front();

    // Agregamos el movimiento a la lista tabu
    bestMov.numIter = numIt + tamLT;
    listaTabuTS.push_back(bestMov);
} 

bool TabuSearch::simplificarVecindad(Solucion *s, Movimiento & bestMov) {
  Operacion *opU = bestMov.op1;
  Operacion *opV = bestMov.op2;

  Operacion *JP_U = opU->getJobAnterior();  // alpha[U]
  Operacion *JS_U = opU->getJobSiguiente(); // gamma[U]

  Operacion *JP_V = opV->getJobAnterior();  // alpha[V]
  Operacion *JS_V = opV->getJobSiguiente();  // gamma[V]

  bool left = false;
  bool right = false;

  // Movimiento hacia la derecha (forward)
  if (bestMov.tipo == 1 || bestMov.tipo == 4 // ) { 
   || bestMov.tipo == 0) {
    // L(JS[v], n), y L(JS[U], n) cambian despues del movimiento
//    if (JS_V->getRT() >= JS_U->getRT())
    // Desigualdad 3.1, new_L( 0, v) > L(0, v) - p_u 
    if ( opV->getTInicio() > (bestMov.tAnt_V - opU->getDuration()) )
      left = true;
//    else
    if ( opV->getRT() > ( bestMov.rtAnt_V + opU->getDuration() ) )
    // Desigualdad 3.2, new_L( v, n) > L(v, n) + p_u 
      right = true;
  }

  // Movimiento hacia la izquierda (backward)
  if (bestMov.tipo == 2 || bestMov.tipo == 3  // ) { 
     || bestMov.tipo == 0 ) {
    // L(JP[v], n), y L(JP[U], n) cambian despues del movimiento
/*    if ( (JP_V->getTInicio() + JP_V->getDuration() ) <=
            (JP_U->getTInicio() + JP_U->getDuration() ) ) */
    // Desigualdad 3.4 new_L(u, n) > L(u, n) - p_v
    if ( opU->getRT() > ( bestMov.rtAnt_U - opV->getDuration() ) )
      right = true;
//    else
    if ( opU->getTInicio() > ( bestMov.tAnt_U + opV->getDuration() ) )
    // Desigualdad 3.5 new_L(0, u) > L(0, u) + p_v
      left = true;
  }

  vector< set<pair<int, int>> > bloquesAnt; 
  bool simplificacion=false;

  if (left && !right ) { // se alcanzo solamente un left guidepost 
    // eliminar bloques
    for(int i=0; i < s->bloques.size(); i++) {
      s->bloques[i].clear();
    }
    s->rutaC.clear();
    
    simplificacion =true;
    TabuSearch::simpl1++;
    bloquesAnt = s->bloques;
   
    // Buscar candidatos en (0, v)
    s->calcularRutaAlInicio( opU, true);
  }

  if (right && !left) { // se alcanzo solamente un right guidepost
    // eliminar bloques
    for(int i=0; i < s->bloques.size(); i++) {
      s->bloques[i].clear();
    }
    s->rutaC.clear();
    
    simplificacion =true;
    TabuSearch::simpl2++;
    bloquesAnt = s->bloques;

    // Buscar candidatos en (v, n) [calcular bloques]
    s->calcularRutaAlFinal( opV, true);
  }


  if (simplificacion && s->getNumBloques() == 0) {
    for(int i=0; i < s->bloques.size(); i++) {
      s->bloques[i] = bloquesAnt[i];
    }
  } 

  return simplificacion;
}