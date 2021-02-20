#include <cstdlib>
#include <utility>
#include <iostream>
#include <ctime>
#include <list>
#include <map>
#include <cmath>
#include <cfloat>
#include <climits>  // INT_MAX
#include <iomanip>  // std::setprecision
#include <algorithm>
#include <limits>

#include <cstdio>
#include <string>
#include <ext/stdio_filebuf.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <fstream>      // std::ifstream
#include <signal.h>

#include "AlgoritmoGenetico.hpp"
#include "UtilJS.hpp"
#include "JobShop.hpp"
#include "GJobShop.hpp"
#include "TabuSearch.hpp"
#include "IterativeSearch.hpp"
#include "Diversidad.hpp"
#include "PathRelinking.hpp"

#ifdef AG_PARALLEL
#include "UtilMPI.hpp"
#include <mpi.h>
#endif

using namespace std;
using __gnu_cxx::stdio_filebuf;

AlgoritmoGenetico::AlgoritmoGenetico(JobShop *p, int tamPob, int valOpt, int numHijos, int max_itersTS, int deltaIncFP) {
  this->tamPob = tamPob;
  this->pob.resize(tamPob, NULL);
  this->valOpt = valOpt;
  this->deltaIncFP = deltaIncFP;

  this->numMaquinas = p->getNumMaquinas();
  this->numTrabajos = p->getNumTrabajos();

  this->max_itersTS = max_itersTS;

  cout << "# AG, maxITers: " << this->max_itersTS << "\n";

  for(int i=0; i < tamPob; i++) {
    pob[i] = new Solucion(new GJobShop(p), valOpt);
    pob[i]->indPob=i;
  }

  cout << "# AG, numHijos: " << numHijos << "\n";

  if (numHijos <= 0) numHijos = tamPob;
  if (numHijos == 2) pcr = 1.0;

  this->hijos.resize(numHijos);

  for(int i=0; i < numHijos; i++) {
    hijos[i] = new Solucion(new GJobShop(p), valOpt);
    hijos[i]->indPob=tamPob+i;
  }

  this->p = p;

//  this->seedIni = time(NULL);

  this->distancias.resize(tamPob+numHijos);
  for(int i=0; i < tamPob+numHijos; i++) {
    distancias[i].resize(tamPob+numHijos, DBL_MAX);
  }
}

AlgoritmoGenetico::~AlgoritmoGenetico() {
  for(int i=0; i< tamPob; i++) {
    if (pob[i] != NULL) delete pob[i];
  }
  int limHijos = hijos.size();
  for(int i=0; i < limHijos; i++) {
    if (hijos[i] != NULL) delete hijos[i];
  }
}

// Cruza considerando las maquinas en dos
// conjuntos complementarios 
int AlgoritmoGenetico::cruzaPorMaquinas(int pos1, int pos2, int pos) {
  
  if (pos < 0 || pos >= hijos.size()) return 0;

  Solucion *s1 = pob[pos1];
  Solucion *s2 = pob[pos2];

  VPermutaciones p1 = s1->getPermutacionSol();
  VPermutaciones p2 = s2->getPermutacionSol();

  VPermutaciones hijo1 = hijos[pos]->getPermutacionSol();
  VPermutaciones hijo2 = hijos[pos]->getPermutacionSol();

  int randomP = 0;
  double nRand = ((double)rand())/RAND_MAX;


  for(int i=0; i < numMaquinas; i++) {
    hijo1[i].resize(s1->getNumTrabajos());
    hijo2[i].resize(s2->getNumTrabajos());
    // Elegimos un padre al azar para esta maquina
    randomP = rand() % 2;

    for(int j=0; j < numTrabajos; j++) {
      if (nRand > pcr || pos1 == pos2 ) {
        hijo1[i][j] = p1[i][j];        
        continue;
      }
      if (randomP == 0) {
        hijo1[i][j] = p1[i][j];
        hijo2[i][j] = p2[i][j];
      } else {
        hijo1[i][j] = p2[i][j];
        hijo2[i][j] = p1[i][j];
      }
    }
  }
  int numHijosGenerados = pos < (hijos.size())-1 ? 2 : 1;

  hijos[pos]->setPermutacionSol(hijo1, false);
  int totalRep = hijos[pos]->reparar();
  hijos[pos]->calcularMakespan(true);

  hijos[pos]->listaTabu.clear();

  if (nRand > pcr || pos1 ==  pos2) {
    numHijosGenerados = 1;
    
    hijos[pos]->permTS = s1->permTS;
    hijos[pos]->numItTS = s1->numItTS;
  } else {
    hijos[pos]->permTS.clear();
    hijos[pos]->numItTS = 0;

    if (numHijosGenerados == 2 && pos1 != pos2) {
      hijos[pos+1]->setPermutacionSol(hijo2, false);
      totalRep = hijos[pos+1]->reparar();
      hijos[pos+1]->calcularMakespan(true);
      hijos[pos+1]->listaTabu.clear();
      hijos[pos+1]->permTS.clear();
      hijos[pos+1]->numItTS = 0;
    }
  }

  return numHijosGenerados;
}

int AlgoritmoGenetico::cruzaPorOrden(int pos1, int pos2, int pos) {
  if (pos < 0 || pos >= hijos.size()) return 0;

  Solucion *s1 = pob[pos1];
  Solucion *s2 = pob[pos2];

  VPermutaciones p1 = s1->getPermutacionSol();
  VPermutaciones p2 = s2->getPermutacionSol();

  VPermutaciones hijo1 = hijos[pos]->getPermutacionSol();

  double nRand = ((double)rand())/RAND_MAX;


  for(int i=0; i < numMaquinas; i++) {
    //hijo1[i].resize(s1->getNumTrabajos());

    set <int> consideredJobs;
    int index1 = 0, index2 = 0; 
    for(int j=0; j < numTrabajos; j++) {
      while(consideredJobs.count(p1[i][index1])) index1++;
      while(consideredJobs.count(p2[i][index2])) index2++;
      if (rand() % 2 == 0) {
        consideredJobs.insert(p1[i][index1]);
        hijo1[i][j] = p1[i][index1];
      } else {
        consideredJobs.insert(p2[i][index2]);
        hijo1[i][j] = p2[i][index2];
      }    
    }
  }
  int numHijosGenerados = 1; 

  hijos[pos]->listaTabu.clear();
  hijos[pos]->mksP1 = s1->getMakespan();
  hijos[pos]->mksP2 = s2->getMakespan();

  if (nRand > pcr ||  pos1 == pos2) { /* Cruce consigimo mismo => Intensificacion TS */
    hijos[pos]->setPermutacionSol(hijo1, true);
    //hijos[pos]->listaTabu = s1->listaTabu;
    hijos[pos]->permTS = s1->permTS;
    hijos[pos]->numItTS = s1->numItTS;
    hijos[pos]->generacion = s1->generacion;
    hijos[pos]->distPadres = 0;

    hijos[pos]->distAlosPadres = Diversidad::distanciaInds(pob[pos1], hijos[pos]);

  } else {
    hijos[pos]->setPermutacionSol(hijo1, false);
    int totalRep = hijos[pos]->reparar();

    // IMPORTANTE: Se debe calcular el makespan aqui
    // para poder generar las rutas criticas que se utilizan
    // en la exploracion de la vecindad (N5 y N7; las funciones
    // para estas vecindades, asumen que las rutas ya estan calculadas)
    fitPromHijos +=  hijos[pos]->calcularMakespan(true);

    difPromHijos += min(fabs(pob[pos1]->getMakespan() - hijos[pos]->getMakespan()),
      fabs(pob[pos2]->getMakespan() - hijos[pos]->getMakespan()));

//    cout << "#\tFitness Hijo: " << hijos[pos]->getMakespan() << "\n";
    numCruzas++;

    hijos[pos]->permTS.clear();
    hijos[pos]->numItTS = 0;
    hijos[pos]->generacion=numIter;
    hijos[pos]->distPadres = Diversidad::distanciaInds(pob[pos1], pob[pos2]);
    hijos[pos]->distAlosPadres = min(
      Diversidad::distanciaInds(pob[pos1], hijos[pos]),
      Diversidad::distanciaInds(pob[pos2], hijos[pos])
      );
  }

  return numHijosGenerados;
}

// Cruza considerando las subcadenas en comun mas largas
// para cada maquina
int AlgoritmoGenetico::cruza(int pos1, int pos2, int pos) {
  if (pos < 0 || pos >= hijos.size()) return 0;

  Solucion *s1 = pob[pos1];
  Solucion *s2 = pob[pos2];

  VPermutaciones p1 = s1->getPermutacionSol();
  VPermutaciones p2 = s2->getPermutacionSol();

  vector< vector< pair<int, int> > > lcss;

  double nRand = ((double)rand())/RAND_MAX;
  if (nRand <= pcr) {
    lcss = UtilJS::LCS_vint(p1, p2);
  }

  VPermutaciones hijo1 = hijos[pos]->getPermutacionSol();

  int posP1 = 0;
  int posP2 = 0;
  int posLCS1 = 0;
  int posLCS2 = 0;

  for(int i =0; i < numMaquinas; i++) {
    posP1 = 0;
    posP2 = 0;
    posLCS1 = 0;
    posLCS2 = 0;

    for(int j=0; j < numTrabajos; j++) {
      if ( nRand > pcr  // Con probabilidad (1-pcr) se copiara siempre el padre al hijo
          || pos1 == pos2 // Se selecciono al mismo padre
          ||  (posLCS1 < lcss[i].size() && j == lcss[i][posLCS1].first) 
        ) {
        // Se copia el gen de P1, en la misma posicion
        hijo1[i][j] = p1[i][j];
        posLCS1++;
        //posP2++;
      } else {

        // Buscamos el primer elem que no este en la subsecuencia
        while( posLCS2 < lcss[i].size() 
          && lcss[i][posLCS2].second <= posP2 ) {
          posP2++;
          posLCS2++;
        }

        // Se copia el gen de P2, manteniendo el orden
        hijo1[i][j] = p2[i][posP2];
        posP2++;

      }
    }
  }

  hijos[pos]->listaTabu.clear();
  hijos[pos]->generacion=numIter;
  hijos[pos]->mksP1 = s1->getMakespan();
  hijos[pos]->mksP2 = s2->getMakespan();
  if (nRand > pcr ||  pos1 == pos2) { /* Cruce consigo mismo => Intensificacion TS */
    hijos[pos]->setPermutacionSol(hijo1, true);
    //hijos[pos]->listaTabu = s1->listaTabu;
    hijos[pos]->permTS = s1->permTS;
    hijos[pos]->numItTS = s1->numItTS;
    hijos[pos]->distPadres = 0;
    hijos[pos]->distAlosPadres = Diversidad::distanciaInds(pob[pos1], hijos[pos]);

  } else {
    hijos[pos]->setPermutacionSol(hijo1, false);
    int totalRep = hijos[pos]->reparar();

    // IMPORTANTE: Se debe calcular el makespan aqui
    // para poder generar las rutas criticas que se utilizan
    // en la exploracion de la vecindad (N5 y N7; las funciones
    // para estas vecindades, asumen que las rutas ya estan calculadas)
    fitPromHijos +=  hijos[pos]->calcularMakespan(true);

    difPromHijos += min(fabs(pob[pos1]->getMakespan() - hijos[pos]->getMakespan()),
      fabs(pob[pos2]->getMakespan() - hijos[pos]->getMakespan()));

    numCruzas++;

    hijos[pos]->permTS.clear();
    hijos[pos]->numItTS = 0;
    hijos[pos]->distPadres = Diversidad::distanciaInds(pob[pos1], pob[pos2]);
    hijos[pos]->distAlosPadres = min(
      Diversidad::distanciaInds(pob[pos1], hijos[pos]),
      Diversidad::distanciaInds(pob[pos2], hijos[pos])
      );
  }

  return 1;
}

int AlgoritmoGenetico::mejorarIndividuo(Solucion *ind) {  
  TabuSearch TS;
  TS.tabuTenure = this->tabuTenure;
  // Aplicamos una TS con criterio de paro por un max num de iters sin mejora
  int evalAct = TS.tabuSearch(ind, 7, 0, 0,  max_itersTS);

  ind->numItTS = TS.numIt;
  ind->timeTS = TS.time_search;

  return ind->getMakespan();
}

int AlgoritmoGenetico::mejorarPoblacion(int tipo_pob, int pMin, int pMax, double maxTimePob) {

  vector<Solucion *> & poblacion = (tipo_pob==0) ? pob : hijos;
  int indBest = pMin;
  int evalAct = 0;
  clock_t t = clock() - this->tIni;

  double maxTimeTS = 0;
  double time_searchMP = 0;

  double iterProm = 0;

  int totalOps = numMaquinas*numTrabajos;
  #ifdef AG_PARALLEL
  int tamMsg = totalOps*2+UtilMPI::NUM_DATA_EXTRA;
  #endif

  t = clock() - tIni;
  time_searchMP = ((double)t)/(CLOCKS_PER_SEC);

  // Calculamos el tiempo restante y lo dividimos entre el numero de generaciones:
  maxTimeTS = (this->maxTime - time_searchMP);
  maxTimeTS /= this->numIter < this->maxGeneraciones ? ( this->maxGeneraciones - this->numIter ) : 1;

  // Calculamos el tiempo restante para cada individuo en esta generacion
  maxTimeTS /= (pMax - pMin + 1);

  clock_t tIter = clock();
  double time_iter = 0;

  fitPromPobM = 0;
  //difPromHijos = 0;
  int numMejoras = 0;

  bestHijo = INT_MAX;

  numProcesos=1;

  for(int i=pMin; i<= pMax; i++) {
    if (totalProcesos == 1) {
      mejorarIndividuo(poblacion[i]);
      cout << "#\tH[" << i << "] " << getPrintInfo(poblacion[i]) 
        << " " << poblacion[i]->timeTS << "\n" << std::flush;
      continue;
    }
    #ifdef AG_PARALLEL
    vector<int> buffMsgRes;
    /* MPI............ */    
    if (numProcesos < totalProcesos) {
      // Enviar Datos al proceso numProcesos
      buffMsgRes = UtilMPI::make_msg_send_Ind(poblacion[i], i);
      MPI_Send( buffMsgRes.data(), buffMsgRes.size(), MPI_INT, numProcesos, TASK_TS, MPI_COMM_WORLD );

      /* PENDIENTE: El anterior podría hacerse con una comunicacion no Bloqueante
        Pero se debe tener cuidado con los buffers que se utilizaran.
       */
      numProcesos++;
    } else {
      vector<int> buffMsg(tamMsg);
      MPI_Status status;
      // Esperar MENSAJE de Cualquier fuente (ANY_SOURCE)
      MPI_Recv(&buffMsg[0], buffMsg.size(), MPI_INT, MPI_ANY_SOURCE, RES_TS, MPI_COMM_WORLD, &status);

      // Enviar Datos al proceso del que se recibe el mensaje
      buffMsgRes = UtilMPI::make_msg_send_Ind(poblacion[i], i);
      MPI_Send( buffMsgRes.data(), buffMsgRes.size(), MPI_INT, status.MPI_SOURCE, TASK_TS, MPI_COMM_WORLD );

      int indH = buffMsg[0];
      double timeHs = UtilMPI::upd_Ind_msg_rcv(poblacion[indH], buffMsg);

      cout << "#\tHP[" << indH << "] " << getPrintInfo(poblacion[indH]) 
        << " " << timeHs << "\n" << std::flush; 
    }
    #endif
  }

  #ifdef AG_PARALLEL
  while (numProcesos > 1) {
    vector<int> buffMsg(tamMsg);
    MPI_Status status;
    // Esperar MENSAJE de Cualquier fuente (ANY_SOURCE)
    MPI_Recv(&buffMsg[0], buffMsg.size(), MPI_INT, MPI_ANY_SOURCE, RES_TS, MPI_COMM_WORLD, &status);

    int indH = buffMsg[0];
    double timeHs = UtilMPI::upd_Ind_msg_rcv(poblacion[indH], buffMsg);

    cout << "#\tHP[" << indH << "] " << getPrintInfo(poblacion[indH]) 
      << " " << timeHs << "\n" << std::flush; 
    numProcesos--;
  }
  #endif

  indBest = pMin;
  numMejoras++;
  for(int i=pMin+1; i < pMax; i++ ) {
  fitPromPobM += poblacion[i]->getMakespan();
    if (poblacion[i]->getMakespan() < poblacion[indBest]->getMakespan())  {
      indBest = i;
    }
    numMejoras++;
  }

  iterProm /= (pMax - pMin + 1);

  fitPromPobM = numMejoras > 1 ? fitPromPobM/numMejoras : fitPromPobM;

  return indBest;
}

int AlgoritmoGenetico::seleccionaPadre() {
  
  int posP1 = rand() % tamPob;
  /* int posP2 = rand() % tamPob;

  // Seleccion por Torneo Binario:
  return pob[posP1]->getMakespan() < pob[posP2]->getMakespan() ? 
    posP1 : posP2; */

  // seleccion aleatoria
  return posP1; 
}


void AlgoritmoGenetico::calcularMatrizDistancias() {

  int indI, indJ;

  for(int i=0; i < tamPob; i++) {
    distancias[i][i] = 0;
    for(int j=i+1; j < tamPob; j++) {
      indI = pob[i]->indPob;
      indJ = pob[j]->indPob;
      distancias[indI][indJ] = Diversidad::distanciaInds(pob[i], pob[j], tipoDiversidad);
      distancias[indJ][indI] = distancias[indI][indJ];
    }
  }

}

void AlgoritmoGenetico::calcularDistanciasH(int iH, int pMin, int pMax) {

  //cout << "#calcularDistanciasH, tipoDiversidad: " << tipoDiversidad << "\n" << flush;

  int indH = hijos[iH]->indPob;
  int indI;

  distancias[indH][indH] = 0;
  Solucion *sol;
  for(int i=pMin; i <= pMax; i++) {
    sol = i < tamPob ? pob[i] : hijos[i-tamPob]; 
    indI = sol->indPob;

    distancias[indH][indI] = Diversidad::distanciaInds(sol, hijos[iH], tipoDiversidad);
    distancias[indI][indH] = distancias[indH][indI];
     
  }
}

double AlgoritmoGenetico::calcularContribucion(vector<Solucion *> & new_pob, 
  int tamNewPob, Solucion *cand, int D) {
  double contribucion=DBL_MAX;
  for(int i=0; i < tamNewPob; i++) {
    contribucion = min( contribucion, 
      distancias[ new_pob[i]->indPob ][cand->indPob]);
    
    if (distancias[new_pob[i]->indPob][cand->indPob] <= D ) {
      cand->cluster.push_back(i);
    }
  }
  return contribucion;
}

void AlgoritmoGenetico::actualizarMatriz(int newInd, int indE) {
  // Actualizamos solo el renglo del nuevo individuo
  // la informacion del individuo eliminado ya no es relevante
  for(int i=0; i < tamPob; i++) {
    distancias[indE][i] = distancias[newInd][i]; 
  }

  // actualizar columna... 
  for(int i=0; i < tamPob; i++) {
    distancias[i][indE] = distancias[i][newInd];
  }

  distancias[indE][indE] = 0;
}

// Seleccion del Mejor no Penalizado 
void AlgoritmoGenetico::selBNP(int D, list<Solucion *> & pobTot, vector<int> indsH,
  int tamMaxCluster) {

  //cout << "#**********************SELBNP******************\n";
  //cout << "#indsH.size(): " << indsH.size() << ", tamMaxCluster: " << tamMaxCluster << ", D:" << D << "\n";

  vector<Solucion *> & new_pob = pob;  
  generarValsRandom();
  pobTot.sort(Diversidad::comparaMakespanSols);

  list<int> indsEx;
  int tamNewPob=0;
  list<Solucion *>::iterator itBest = pobTot.begin();
  list<Solucion *>::iterator it = itBest;
  list<Solucion *>::iterator indIniCand = itBest;

  (*it)->cluster.clear();
  (*it)->cluster.push_back(tamNewPob);

  // Insertamos al mejor
  new_pob[tamNewPob] = *itBest;
  if (new_pob[tamNewPob]->esHijo) {
    numExitos++;
    indsEx.push_back(tamNewPob);
  }

  //cout << "#\tIndividuo Best seleccionado, mks: " << new_pob[tamNewPob]->getMakespan() << "\n";

  tamNewPob++;


  // Lo eliminamos de la lista de seleccionables:
  it = pobTot.erase(itBest);

  double contribAct=0;
  double maxContrDiv=0;
  indIniCand=pobTot.begin();
  bool seleccionado=false;

  while( tamNewPob < pob.size() ) {
    maxContrDiv = 0;
    contribAct = 0;
    it = indIniCand; // vamos a buscar primero en [iniCand, end)
    itBest = it;
    seleccionado = D < 0;

    while(!seleccionado && it != pobTot.end()) { 
      (*it)->cluster.clear();
      (*it)->cluster.push_back(tamNewPob);

      // Calculamos la contrib del ind a newPob
      contribAct = calcularContribucion(new_pob, tamNewPob, *it, D);

      if (contribAct > maxContrDiv) {
        maxContrDiv = contribAct;
        itBest = it;
      }

      seleccionado = contribAct > D;

      if (!seleccionado // El individuo esta en la distancia del Cluster
        && contribAct > (D*0.1) // esta en el cluster de almenos otro ind
        && (*it)->cluster.size() < tamMaxCluster // el cluster del ind act no esta lleno
        ) { 

          seleccionado = true;
          for(int i=0; i < (*it)->cluster.size(); i++) {
            // Los cluster de los vecinos no estan llenos
            if ( new_pob[ (*it)->cluster[i] ]->cluster.size() >= tamMaxCluster ) {
              seleccionado = false;
              break;
            }
          }
      }// Fin Seleccion por Cluster

      if (seleccionado) itBest = it;
      else it++;
    } 

    if (!seleccionado) { // buscamos en [begin, iniCand )
      // Si todos los candidatos ya estan penalizados, elegimos al de mayor contribucion
      for(it=pobTot.begin(); it != indIniCand; it++) {
        (*it)->cluster.clear();
        (*it)->cluster.push_back(tamNewPob);
        contribAct = calcularContribucion(new_pob, tamNewPob, *it, D);
        if (contribAct > maxContrDiv) {
          maxContrDiv = contribAct;
          itBest = it;
        }
      }
    }

    // En itBest tenemos al candidato que debe ser selecionado
    // lo agregramos
    new_pob[tamNewPob] = *itBest; 

//    cout << "#\tIndividuo seleccionado, mks=" << new_pob[tamNewPob]->getMakespan() 
//      << ", maxContrDiv=" << maxContrDiv << " \n";

    // Actualizamos los cluster de new_pob
    for(int i=1; i < (*itBest)->cluster.size(); i++) {
      new_pob[(*itBest)->cluster[i]]->cluster.push_back(tamNewPob);
    }

    if (new_pob[tamNewPob]->esHijo) {
      numExitos++;
      indsEx.push_back(tamNewPob);
    }
    tamNewPob++;

    // Lo eliminamos de la lista de seleccionables:
    indIniCand = pobTot.erase(itBest);    

    if (indIniCand == pobTot.end()) indIniCand = pobTot.begin(); 

  } // FIN WHILE tamNewPob < pob.size()

  Solucion *solElim = NULL;
  // Actualizar matriz
  for(int i=0; i < indsH.size(); i++) {
    solElim = pobTot.front();
    if (!solElim->esHijo) { // hay que actualizar la matriz
      actualizarMatriz( new_pob[indsEx.front()]->indPob, solElim->indPob );
      new_pob[indsEx.front()]->indPob = solElim->indPob;
      indsEx.pop_front();
    }
    hijos[ indsH[i] ] = solElim;
    hijos[ indsH[i] ]->cluster.clear();
    hijos[ indsH[i] ]->indPob = tamPob + indsH[i];
    pobTot.pop_front();
  }

}

int AlgoritmoGenetico::seleccionaIndividuos(int tipoSeleccion, double D, int indHijo) {

  this->distPromPadHijos=0;
  numCruzas = 0;
  vector<int> indsH;

  // Unimos hijos y padres
  list<Solucion *> pobTot;

  for(int i=0; i < pob.size(); i++ ) {
    pobTot.push_back(pob[i]);
    pob[i]->cluster.clear();
    pob[i]->esHijo = false;
  }

  if (indHijo < 0) {
    for(int i=0; i < hijos.size(); i++) {
      pobTot.push_back(hijos[i]);
      hijos[i]->cluster.clear();
      hijos[i]->esHijo = true;

      calcularDistanciasH(i, 0, tamPob-1+i);
      indsH.push_back(i);

      if (hijos[i]->generacion == numIter) {
        this->distPromPadHijos += hijos[i]->distPadres;
        numCruzas++;
      }
    }
  } else {
    pobTot.push_back(hijos[indHijo]);
    hijos[indHijo]->cluster.clear();
    hijos[indHijo]->esHijo = true;

    if (hijos[indHijo]->generacion == numIter) {
      this->distPromPadHijos += hijos[indHijo]->distPadres;
      numCruzas++;
    }

    calcularDistanciasH(indHijo, 0, tamPob-1);
    indsH.push_back(indHijo);
  }
  
  this->distPromPadHijos = numCruzas != 0 ? this->distPromPadHijos/numCruzas : this->distPromPadHijos;

//  cout << "# TIPO Seleccion: " << tipoSeleccion << "\n";

  if (tipoSeleccion == SELEC_MEJOR_NO_PENALIZADO) {
    selBNP(D, pobTot, indsH, 1);
  } else if (tipoSeleccion == SELECCION_CLUSTER) {
    selBNP(D, pobTot, indsH, tamPob*0.1 + 1);
  } else { // Seleccionar al Mejor individuo
    selBNP(-1, pobTot, indsH, tamPob);
  }

  return pob[0]->getMakespan();;
}

Solucion * AlgoritmoGenetico::optimizador(int maxGen, double maxTimeSearch, 
  int tipoSeleccion, int tipoCruza, bool guardarPob, string nameFileOut, 
  double porTimePobIni, double factAjInten) {

  if (factAjInten <= 0) factAjInten = 1.0;

  if (tipoSeleccion == SELECCION_CLUSTER) {
    // pSelecc se utilizara para definir la probabilidad de cruza individuos
    // dentro del mismo cluster
    this->pSelecc = 0;
  }

  cout << "# Porcentaje de Tiempo para la pob Ini : " << porTimePobIni << "\n"
    << "# Porcentaje D inicial: " << porDInicial << "\n"
    << "# MAximo Numero de Procesos: " << totalProcesos << "\n"
    << "# Tipo Diversidad: " << tipoDiversidad << "\n" << std::flush;

  this->guardaPob = guardarPob;
  if (guardaPob) {
    this->nameFile = nameFileOut;
  }

  struct timeval start, end; 
  gettimeofday(&start, NULL);

  tIni = clock();
  this->maxTime = maxTimeSearch;
  clock_t t = clock() - tIni;

  this->numIter = 0;
  this->maxGeneraciones = maxGen;

  clock_t tIniReem, tFinReem, tIniHs, tFinHs;
  double timeHs, timeReem, timeHTS;

  int indBestHijo = 0;
  int numHijos = 0;
  int limHijos = hijos.size();

  cout << "# Num de Hijos por generacion: " << limHijos << "\n" << std::flush;

  int posP1 = 0;
  int posP2 = 0;

  Solucion *tmpSol = NULL;

  // Mejorar la poblacion:
  int indBest = this->mejorarPoblacion(0, 0, tamPob-1, porTimePobIni*maxTimeSearch);
  int bestEval = pob[indBest]->getMakespan();

  this->calcularMatrizDistancias();

  double fitProm = 0;
  fitPromHijos = 0;
  fitPromPobM = 0;
  distPromPadHijos= 0;
  
  for(int i=0; i < tamPob; i++) {
    fitProm += pob[i]->getMakespan();
  }
  
  fitProm /= tamPob;

  int numHijosGenerados = 0;

  cout << "#Gen Best GenBest NumExs disPromPadEx fitProm fitPromHij fitPromHijMej BestHijo disPromPadHij entrProm diverProm time Dval \n" << std::flush;

  double entropia = Diversidad::calcularEntropiaPob(pob);
  double diversidad_0 = Diversidad::calcularDiversidadPob(pob, pob.size(), 0);
  double diversidad_1 = Diversidad::calcularDiversidadPob(pob, pob.size(), 1);
  double diversidad_2 = Diversidad::calcularDiversidadPob(pob, pob.size(), 2);
  double diversidad = diversidad_0;

  t = clock() - tIni;

  if (tipoDiversidad == 1) {
    diversidad = diversidad_1;
  } else {
    if (tipoDiversidad == 2)
      diversidad = diversidad_2;
    else
      diversidad = diversidad_0;
  }

  double Dini = porDInicial*diversidad;
  double D = Dini;

  cout << "# Dini " << Dini << "\n" << std::flush; 
  cout << "# DiversidadIni: " << diversidad_0 
    << " " << diversidad_1 << " " << diversidad_2 << "\n" << std::flush;

  double time_muestreo = 0;
  double time_iterAnt = 0;

  gettimeofday(&end, NULL); 
  time_search = (end.tv_sec - start.tv_sec) * 1e6; 
  time_search = (time_search + (end.tv_usec -  
                              start.tv_usec)) * 1e-6; 

  int totalOps = numMaquinas*numTrabajos;
  #ifdef AG_PARALLEL
  int tamMsg = totalOps*2+UtilMPI::NUM_DATA_EXTRA;
  #endif

  timeBest = time_search;
  iterBest=0;
  int bestMaksepan = pob[indBest]->getMakespan();

  numExitos = tamPob;
  bestHijo = pob[indBest]->getMakespan();

  int numGenSinMejora = 0;

  double tmpRand = 0.0;

  numProcesos=1;

  while (
    time_search < maxTime
    && pob[indBest]->getMakespan() > valOpt) {

    gettimeofday(&end, NULL); 
    time_search = (end.tv_sec - start.tv_sec) * 1e6; 
    time_search = (time_search + (end.tv_usec -  
                              start.tv_usec)) * 1e-6;

    tIniHs = clock();

    this->pSelecc = (time_search/maxTime);
    
    if (time_search > time_muestreo) {
      imprimirPoblacion();
      time_muestreo += maxTimeSearch/50;
    }
    time_iterAnt = time_search;

    numHijos = 0;
    fitPromHijos = 0;
    difPromHijos = 0;
    numCruzas = 0;

    indBestHijo = 0;
    
    // Generamos a los hijos
    while (numHijos < limHijos) {
      // Torneo Binario para generar padres
      posP1 = this->seleccionaPadre();
      posP2 = this->seleccionaPadre();

      cout << "#PadresIni, posP1: " << posP1 << ", posP2: " << posP2 << "\n";

      tmpRand = ((double) rand() / (RAND_MAX));
      cout << "#tmpRand: " << tmpRand << ", pSelecc: " << pSelecc << "\n" << std::flush;
      if (tipoSeleccion == SELECCION_CLUSTER && tmpRand < pSelecc 
        && pob[posP1]->cluster.size() > 1 ) {
        // Elegimos un aleatorio del cluster
        posP2 = pob[posP1]->cluster.size() > 1 ? (rand() % pob[posP1]->cluster.size()) : 0;
        posP2 = posP2 == 0 ? posP1 : pob[posP1]->cluster[posP2];
        cout << "#PadresCluster, posP1: " << posP1 << ", posP2: " << posP2 << "\n";
      }

      // Cruza para generar a nuevo
      switch(tipoCruza) {
        case CRUZA_MC:
          numHijosGenerados = this->cruzaPorMaquinas(posP1, posP2, numHijos);
          numHijos += numHijosGenerados;
          if (numHijos < limHijos && numHijosGenerados < 2)
            numHijos += this->cruzaPorMaquinas(posP2, posP1, numHijos);
          break;
        case CRUZA_ORD:
          numHijosGenerados = this->cruzaPorOrden(posP1, posP2, numHijos);
          numHijos += numHijosGenerados;
          if (numHijos < limHijos && numHijosGenerados < 2)
            numHijos += this->cruzaPorOrden(posP2, posP1, numHijos);
          break;
        default:
        case CRUZA_LCS:
          numHijosGenerados = this->cruza(posP1, posP2, numHijos);
          numHijos += numHijosGenerados;

          if (numHijos < limHijos && numHijosGenerados < 2 ) 
            numHijos += this->cruza(posP2, posP1, numHijos);
          break;
      }
    } // While generacion de hijos

    tFinHs = clock();
    timeHs = ((double)(tFinHs - tIniHs)/CLOCKS_PER_SEC); 

    if (totalProcesos == 1) { // Ejecucion en secuencial
        indBestHijo = mejorarPoblacion(1, 0, 1);
        fitPromHijos = numHijos != 0 ? fitPromHijos/numHijos : fitPromHijos;
        difPromHijos = numHijos != 0 ? difPromHijos/numHijos : difPromHijos;

        tIniReem = clock();
        numIter++;

        gettimeofday(&end, NULL); 
        time_search = (end.tv_sec - start.tv_sec) * 1e6; 
        time_search = (time_search + (end.tv_usec -  
                                      start.tv_usec)) * 1e-6;

        D = Dini - Dini*(time_search/(maxTime*fracTiemFinPen));
        D = D < 0 ? 0 : D;

        this->seleccionaIndividuos(tipoSeleccion, D);

        entropia = Diversidad::calcularEntropiaPob(pob);
        diversidad_0 = Diversidad::calcularDiversidadPob(pob, pob.size(), 0);
        diversidad_1 = Diversidad::calcularDiversidadPob(pob, pob.size(), 1);
        diversidad_2 = Diversidad::calcularDiversidadPob(pob, pob.size(), 2);

        distPromPadres = numExitos != 0 ? distPromPadres/numExitos : distPromPadres;

        if (numExitos == 0) numGenSinMejora++;
        else numGenSinMejora = 0;

        if( numGenSinMejora >= tamPob) {
          numGenSinMejora = 0;
          max_itersTS *= factAjInten;
        }

        fitProm = 0;
        indBest = 0;
        // bool error=false;

        for(int i=0; i < tamPob; i++) {
          fitProm += pob[i]->getMakespan();
          if ( Diversidad::comparaMakespanSols(pob[i], pob[indBest])) {
            indBest = i;
          }
          cout << "#Ind[" << i << " , " << pob[i]->indPob 
            << "] " << pob[i]->getMakespan() 
            << " " << pob[i]->distMinPob0 
            << " " << pob[i]->distMinPob1
            << " " << pob[i]->distMinPob2;

          if (pob[i]->cluster.size() > 1) {
            cout << " c=[ ";
            for(int j=0; j < pob[i]->cluster.size(); j++ ) {
              cout << pob[i]->cluster[j] << " ";
            }
            cout << "]";
          }

          // if (tipoDiversidad == 0 && pob[i]->distMinPob0 < D ) error=true;

          cout << "\n" << std::flush;
        }

        /* if (error) {
            cout << "\nERROR!! individuo mal seleccionado....\n";
            cout << "Factor de Penalizacion Actual: " << D << "\n";
            exit(0);
          } */

        fitProm /= tamPob;

        if (pob[indBest]->getMakespan() < bestMaksepan) {
          t = clock() - tIni;
          gettimeofday(&end, NULL); 
          time_search = (end.tv_sec - start.tv_sec) * 1e6; 
          time_search = (time_search + (end.tv_usec -  
            start.tv_usec)) * 1e-6;

          bestMaksepan = pob[indBest]->getMakespan();
          timeBest = time_search;
          iterBest = numIter;
        }
        tFinReem = clock();
        timeReem = ((double)(tFinReem - tIniReem)/CLOCKS_PER_SEC);

        cout <<  numIter 
          << " " << pob[indBest]->getMakespan() << " " << pob[indBest]->generacion 
          << " " << iterBest << " " << timeBest << " " << time_search 
          << " " << to_string(time_search - time_iterAnt) // timeIter
          << " " << fitProm << " " << numExitos << " " << distPromPadres  
          << " " << fitPromHijos << " " << fitPromPobM << " " << bestHijo << " " << distPromPadHijos
          << " " << max_itersTS
          << " " <<  entropia << " " << diversidad_0 << " " << diversidad_1 << " " << diversidad_2
          << " " << D << " " << this->pSelecc 
          << " " << timeHs << " " << timeReem
          << "\n" << std::flush;
    }

    #ifdef AG_PARALLEL
    /* PARALELIZACION */
    for(int iH=0; iH < numHijos; iH++) {
      /* MPI............ */
      vector<int> buffMsgRes;
      if (numProcesos < totalProcesos) {
        buffMsgRes = UtilMPI::make_msg_send_Ind(hijos[iH], iH);
        // Enviar Datos al proceso numProcesos
        MPI_Send( buffMsgRes.data(), buffMsgRes.size(), MPI_INT, numProcesos, TASK_TS, MPI_COMM_WORLD );

        /* PENDIENTE: 
          El anterior podría hacerse con una comunicacion no Bloqueante
          Pero se debe tener cuidado con los buffers que se utilizaran.
        */
        numProcesos++;
      } else { // se alcanzo el max de procesos      

        vector<int> buffMsg(tamMsg);
        MPI_Status status;
        // Esperar a que un hijo termine
        // Esperar MENSAJE de Cualquier fuente (ANY_SOURCE)
        MPI_Recv(&buffMsg[0], buffMsg.size(), MPI_INT, MPI_ANY_SOURCE, RES_TS, MPI_COMM_WORLD, &status);

        buffMsgRes = UtilMPI::make_msg_send_Ind(hijos[iH], iH);
        // Enviar Datos al proceso del que se recibe el mensaje
        MPI_Send( buffMsgRes.data(), buffMsgRes.size(), MPI_INT, status.MPI_SOURCE, TASK_TS, MPI_COMM_WORLD );

        int indH = buffMsg[0];

        timeHTS = UtilMPI::upd_Ind_msg_rcv(hijos[0], buffMsg);

        cout << "#\tHP[" << indH << "] " << getPrintInfo(hijos[0]) 
          << " " << timeHTS << "\n" << std::flush; 

        /* **************** REEMPLAZAMIENTO...... */
        tIniReem = clock();
        numIter++;
        // Actualizamos el valor de D, penalizacion de diversidad
        gettimeofday(&end, NULL); 
        time_search = (end.tv_sec - start.tv_sec) * 1e6; 
        time_search = (time_search + (end.tv_usec -  
                                      start.tv_usec)) * 1e-6;

        D = Dini - Dini*(time_search/(maxTime*fracTiemFinPen));
        D = D < 0 ? 0 : D;

        numExitos = 0;
        distPromPadres=0;
        this->seleccionaIndividuos(tipoSeleccion, D, 0);

        entropia = Diversidad::calcularEntropiaPob(pob);
        diversidad_0 = Diversidad::calcularDiversidadPob(pob, pob.size(), 0);
        diversidad_1 = Diversidad::calcularDiversidadPob(pob, pob.size(), 1);
        diversidad_2 = Diversidad::calcularDiversidadPob(pob, pob.size(), 2);

        distPromPadres = numExitos != 0 ? distPromPadres/numExitos : distPromPadres;

        if (numExitos == 0) numGenSinMejora++;
        else numGenSinMejora = 0;

        if( numGenSinMejora >= tamPob) {
          numGenSinMejora = 0;
          max_itersTS *= factAjInten;
        }

        fitProm = 0;
        indBest = 0;

        for(int i=0; i < tamPob; i++) {
          fitProm += pob[i]->getMakespan();
          if ( Diversidad::comparaMakespanSols(pob[i], pob[indBest])) {
            indBest = i;
          }
          cout << "#Ind[" << i << " , " << pob[i]->indPob 
            << "] " << pob[i]->getMakespan() 
            << " " << pob[i]->distMinPob0 
            << " " << pob[i]->distMinPob1
            << " " << pob[i]->distMinPob2;

          if (pob[i]->cluster.size() > 1) {
            cout << " c=[ ";
            for(int j=0; j < pob[i]->cluster.size(); j++ ) {
              cout << pob[i]->cluster[j] << " ";
            }
            cout << "]";
          }

          cout << "\n" << std::flush;
        }
        fitProm /= tamPob;

        if (pob[indBest]->getMakespan() < bestMaksepan) {
          t = clock() - tIni;
          gettimeofday(&end, NULL); 
          time_search = (end.tv_sec - start.tv_sec) * 1e6; 
          time_search = (time_search + (end.tv_usec -  
            start.tv_usec)) * 1e-6;

          bestMaksepan = pob[indBest]->getMakespan();
          timeBest = time_search;
          iterBest = numIter;
        }
        tFinReem = clock();
        timeReem = ((double)(tFinReem - tIniReem)/CLOCKS_PER_SEC);

        cout <<  numIter 
          << " " << pob[indBest]->getMakespan() << " " << pob[indBest]->generacion 
          << " " << iterBest << " " << timeBest << " " << time_search 
          << " " << to_string(time_search - time_iterAnt) // timeIter
          << " " << fitProm << " " << numExitos << " " << distPromPadres  
          << " " << fitPromHijos << " " << fitPromPobM << " " << bestHijo << " " << distPromPadHijos
          << " " << max_itersTS
          << " " <<  entropia << " " << diversidad_0 << " " << diversidad_1 << " " << diversidad_2
          << " " << D << " " << this->pSelecc 
          << " " << timeHs << " " << timeReem
          << "\n" << std::flush;

      } // FIN ELSE, se alcanzo el numero max de procesos 
    } // FIN FOR para iterar hijos generados

    /* ****************** FIN PARALELIZAR**************** */
    #endif
  } // END WHILE OPTIMIZADOR....

  #ifdef AG_PARALLEL
  while (numProcesos > 1) {
    vector<int> buffMsg(tamMsg);
    MPI_Status status;
    // Esperar MENSAJE de Cualquier fuente (ANY_SOURCE)
    MPI_Recv(&buffMsg[0], buffMsg.size(), MPI_INT, MPI_ANY_SOURCE, RES_TS, MPI_COMM_WORLD, &status);

    int indH = buffMsg[0];
    timeHTS = UtilMPI::upd_Ind_msg_rcv(hijos[0], buffMsg);
    cout << "#\tHP[" << indH << "] " << getPrintInfo(hijos[0]) 
      << " " << timeHTS << "\n" << std::flush; 
    numProcesos--;

    /* **************** REEMPLAZAMIENTO...... */
    tIniReem = clock();
    numIter++;
    // Actualizamos el valor de D, penalizacion de diversidad
    gettimeofday(&end, NULL); 
    time_search = (end.tv_sec - start.tv_sec) * 1e6; 
    time_search = (time_search + (end.tv_usec -  
                                  start.tv_usec)) * 1e-6;

    D = Dini - Dini*(time_search/(maxTime*fracTiemFinPen));
    D = D < 0 ? 0 : D;

    numExitos = 0;
    distPromPadres=0;
    this->seleccionaIndividuos(tipoSeleccion, D, 0);

    entropia = Diversidad::calcularEntropiaPob(pob);
    diversidad_0 = Diversidad::calcularDiversidadPob(pob, pob.size(), 0);
    diversidad_1 = Diversidad::calcularDiversidadPob(pob, pob.size(), 1);
    diversidad_2 = Diversidad::calcularDiversidadPob(pob, pob.size(), 2);

    distPromPadres = numExitos != 0 ? distPromPadres/numExitos : distPromPadres;

    if (numExitos == 0) numGenSinMejora++;
    else numGenSinMejora = 0;

    if( numGenSinMejora >= tamPob) {
      numGenSinMejora = 0;
      max_itersTS *= factAjInten;
    }

    fitProm = 0;
    indBest = 0;

    for(int i=0; i < tamPob; i++) {
      fitProm += pob[i]->getMakespan();
      if ( Diversidad::comparaMakespanSols(pob[i], pob[indBest])) {
        indBest = i;
      }
      cout << "#Ind[" << i << "] " << pob[i]->getMakespan() 
        << " " << pob[i]->distMinPob0 
        << " " << pob[i]->distMinPob1
        << " " << pob[i]->distMinPob2;

      if (pob[i]->cluster.size() > 1) {
        cout << " c=[ ";
        for(int j=0; j < pob[i]->cluster.size(); j++ ) {
          cout << pob[i]->cluster[j] << " ";
        }
        cout << "]";
      }

      cout << "\n" << std::flush;
    }
    fitProm /= tamPob;

    if (pob[indBest]->getMakespan() < bestMaksepan) {
      t = clock() - tIni;
      gettimeofday(&end, NULL); 
      time_search = (end.tv_sec - start.tv_sec) * 1e6; 
      time_search = (time_search + (end.tv_usec -  
        start.tv_usec)) * 1e-6;

      bestMaksepan = pob[indBest]->getMakespan();
      timeBest = time_search;
      iterBest = numIter;
    }
    tFinReem = clock();
    timeReem = ((double)(tFinReem - tIniReem)/CLOCKS_PER_SEC);

    cout <<  numIter 
      << " " << pob[indBest]->getMakespan() << " " << pob[indBest]->generacion 
      << " " << iterBest << " " << timeBest << " " << time_search 
      << " " << to_string(time_search - time_iterAnt) // timeIter
      << " " << fitProm << " " << numExitos << " " << distPromPadres  
      << " " << fitPromHijos << " " << fitPromPobM << " " << bestHijo << " " << distPromPadHijos
      << " " << max_itersTS
      << " " <<  entropia << " " << diversidad_0 << " " << diversidad_1 << " " << diversidad_2
      << " " << D << " " << this->pSelecc 
      << " 0 " << timeReem
      << "\n" << std::flush;
  }
  #endif

  entropia = Diversidad::calcularEntropiaPob(pob);
  diversidad_0 = Diversidad::calcularDiversidadPob(pob, pob.size(), 0);
  diversidad_1 = Diversidad::calcularDiversidadPob(pob, pob.size(), 1);
  diversidad_2 = Diversidad::calcularDiversidadPob(pob, pob.size(), 2);

  cout << std::setprecision(17) <<  numIter 
    << " " << pob[indBest]->getMakespan() << " " << pob[indBest]->generacion 
    << " " << iterBest << " " << timeBest << " " << time_search 
    << " " << to_string(time_search - time_iterAnt) // timeIter
    << " " << fitProm << " " << numExitos << " " << distPromPadres  
    << " " << fitPromHijos << " " << fitPromPobM << " " << bestHijo << " " << distPromPadHijos
    << " " << max_itersTS
    << " " <<  entropia << " " << diversidad_0 << " " << diversidad_1 << " " << diversidad_2
    << " " << D << " 1 0 0 \n" << std::flush;
  imprimirPoblacion();

  cout << "#Best Makespan AG: " << pob[indBest]->getMakespan() << "\n";
  cout << "#Total de Generaciones: " << numIter << "\n";
  cout << "#Best, Es Factible: " << pob[indBest]->esFactible() << "\n" << std::flush;
  cout << "#Time del Best: " << timeBest << "\n";
  cout << "#Iter del Best: " << iterBest << "\n";

  return pob[indBest];

}

Solucion * AlgoritmoGenetico::TSPR(double maxTimeSearch, int maxSinMejora) {

  tIni = clock();
  this->maxTime = maxTimeSearch;
  clock_t t = clock() - tIni;
  time_search = 0;

  this->numIter = 0;

  // Mejorar la poblacion:
  int indBest=0;
  int evalAct = pob[0]->getMakespan();

  int totalOps = numMaquinas*numTrabajos;
  #ifdef AG_PARALLEL
  int tamMsg = totalOps*2 + UtilMPI::NUM_DATA_EXTRA;
  double timeHTS=0;
  #endif

  double fitProm=0;
  double fitMin=evalAct;
  double fitMax=evalAct;
  int numInd = 0;
  numProcesos=1;

  cout << "#totalProcesos: "  << totalProcesos << "\n" << flush;
  cout << "# Optimizador:  PRTS\n" << std::flush;
  cout << "# MAX_TIME: "  << maxTimeSearch << "\n" << flush;
  cout << "# maxSinMejora TS: " << maxSinMejora 
    << "\n# maxSinMejora TS ligera (candidatos): " << (maxSinMejora/25) << "\n" << std::flush;

  indBest = this->mejorarPoblacion(0, 0, tamPob-1);

  for(int i=0; i < tamPob; i++ )  {

    evalAct = pob[i]->getMakespan();
    pob[i]->generacion = 0;

    if (evalAct < pob[indBest]->getMakespan()) {
      indBest = i;
    }

    fitProm += evalAct;
    fitMin = evalAct < fitMin ? evalAct : fitMin;
    fitMax = evalAct > fitMax ? evalAct : fitMax;
    numInd++;
  }

  fitProm = numInd > 0 ? fitProm/numInd : fitProm;

  int bestEval = pob[indBest]->getMakespan();
  numIter=0;

  pob.push_back(new Solucion(new GJobShop(p), valOpt));
  if (totalProcesos == 1)
    pob.push_back(new Solucion(new GJobShop(p), valOpt));

  // Posiciones de los padres
  int pos1 = 0;
  int pos2 = 0;

  double entropia = 10000;
  double diversidad_0 = 0;
  double diversidad_1 = 0;
  double diversidad_2 = 0;
  int bestHijo = bestEval ;
  double distPromPadres=0;
  double fitPromHijos=fitProm;
  numExitos=0;

  double time_iterAnt = 0;

  timeBest = 0;
  iterBest = 0;
  numProcesos=1;

  t = clock() - tIni;
  time_search = ((double)t)/(CLOCKS_PER_SEC);

  while (time_search < maxTime
    && pob[indBest]->getMakespan() > valOpt) {

    time_iterAnt = time_search;

    // Seleccionar padres...
    pos1 = rand() % tamPob;
    pos2 = rand() % tamPob;
    while (pos2 == pos1 ) pos2 = rand() % tamPob;

    if (totalProcesos==1) {
      evalAct = pob[0]->getMakespan();
      fitProm=0;
      fitMin=evalAct;
      fitMax=evalAct;
      numInd = 0;

      for(int i=0; i < tamPob; i++ )  {
        evalAct = pob[i]->getMakespan();
        fitProm += evalAct;
        fitMin = evalAct < fitMin ? evalAct : fitMin;
        fitMax = evalAct > fitMax ? evalAct : fitMax;
        numInd++;
      }
      fitProm = numInd > 0 ? fitProm/numInd : fitProm;

      numIter++;
      entropia = Diversidad::calcularEntropiaPob(pob, tamPob);
      diversidad_0 = Diversidad::calcularDiversidadPob(pob, tamPob, 0);
      diversidad_1 = Diversidad::calcularDiversidadPob(pob, tamPob, 1);
      diversidad_2 = Diversidad::calcularDiversidadPob(pob, tamPob, 2);

      t = clock() - tIni;
      time_search = ((double)t)/(CLOCKS_PER_SEC);

      // Generar Hijos
      evalAct = PathRelinking::pathRelinking(pob[pos1], pob[pos2], pob[tamPob], maxSinMejora);
      pob[tamPob]->generacion = numIter;
      bestHijo = evalAct;
      distPromPadres = pob[tamPob]->distPadres;
      fitPromHijos = evalAct;

      if (evalAct > valOpt) {
        evalAct = PathRelinking::pathRelinking(pob[pos2], pob[pos1], pob[tamPob+1], maxSinMejora);
        pob[tamPob+1]->generacion = numIter;
        bestHijo = evalAct < bestHijo ? evalAct : bestHijo;
        fitPromHijos += evalAct;
        distPromPadres += pob[tamPob+1]->distPadres;
        distPromPadres /= 2; 
        fitPromHijos /= 2;
      }
      
      numExitos = 0;

      generarValsRandom();
      sort(pob.begin(), pob.end(), Diversidad::comparaMakespanSols );

      numExitos += pob[tamPob]->generacion == numIter ? 0 : 1;
      numExitos += pob[tamPob+1]->generacion == numIter ? 0 : 1;

      indBest=0;

      if (pob[indBest]->getMakespan() < bestEval) {
        bestEval = pob[indBest]->getMakespan();
        timeBest = time_search;
        iterBest = numIter;
      }

      t = clock() - tIni;
      time_search = ((double)t)/(CLOCKS_PER_SEC);

      /* Imprimir datos de la Iteracion ... */
      cout << numIter 
        << " " << pob[indBest]->getMakespan() << " " << pob[indBest]->generacion 
        << " " << iterBest << " " << timeBest << " " << time_search 
        << " " << to_string(time_search - time_iterAnt) // timeIter      
        << " " << fitMax << " " << fitProm << " " << numExitos << " " << distPromPadres  
        << " " << fitPromHijos << " "<< bestHijo 
        << " " << entropia << " " << diversidad_0 
        << " " << diversidad_1 << " " << diversidad_2
        << "\n" << std::flush;
    }
    #ifdef AG_PARALLEL 
    else {
      /* PARALELIZACION */
      vector<int> buffMsgRes;
      int numHijos=0;
      while (numHijos < 2) {
        if (numHijos == 0)
            buffMsgRes = UtilMPI::make_msg_send_IndPR(pob[pos1], pob[pos2], tamPob);
          else
            buffMsgRes = UtilMPI::make_msg_send_IndPR(pob[pos2], pob[pos1], tamPob);
        numHijos++;

        if (numProcesos < totalProcesos) {
          // Enviar Datos al proceso numProcesos
          MPI_Send( buffMsgRes.data(), buffMsgRes.size(), MPI_INT, numProcesos, TASK_PR, MPI_COMM_WORLD );
          numProcesos++;       
        } else {

          vector<int> buffMsg(tamMsg);
          MPI_Status status;
          // Esperar a que un hijo termine
          // Esperar MENSAJE de Cualquier fuente (ANY_SOURCE)
          MPI_Recv(&buffMsg[0], buffMsg.size(), MPI_INT, MPI_ANY_SOURCE, RES_TS, MPI_COMM_WORLD, &status);

          // Enviar Datos al proceso del que se recibe el mensaje
          MPI_Send( buffMsgRes.data(), buffMsgRes.size(), MPI_INT, status.MPI_SOURCE, TASK_PR, MPI_COMM_WORLD );

          int indH = buffMsg[0];

          timeHTS = UtilMPI::upd_Ind_msg_rcv(pob[tamPob], buffMsg);

          cout << "#\tHP[" << indH << "] " << getPrintInfo(pob[tamPob]) 
            << " " << timeHTS << "\n" << std::flush; 

          numExitos = 0;
          bestHijo = pob[tamPob]->getMakespan();
          fitPromHijos = bestHijo;
          distPromPadres = pob[tamPob]->distPadres;

          /* **************** REEMPLAZAMIENTO...... */
          generarValsRandom();
          sort(pob.begin(), pob.begin() + tamPob + 1, Diversidad::comparaMakespanSols );
          numExitos += pob[tamPob]->generacion == numIter ? 0 : 1;

          evalAct = pob[0]->getMakespan();
          fitProm=0;
          fitMin=evalAct;
          fitMax=evalAct;
          numInd = 0;

          for(int i=0; i < tamPob; i++ )  {
            evalAct = pob[i]->getMakespan();
            fitProm += evalAct;
            fitMin = evalAct < fitMin ? evalAct : fitMin;
            fitMax = evalAct > fitMax ? evalAct : fitMax;
            numInd++;
          }
          fitProm = numInd > 0 ? fitProm/numInd : fitProm;

          numIter++;
          entropia = Diversidad::calcularEntropiaPob(pob, tamPob);
          diversidad_0 = Diversidad::calcularDiversidadPob(pob, tamPob, 0);
          diversidad_1 = Diversidad::calcularDiversidadPob(pob, tamPob, 1);
          diversidad_2 = Diversidad::calcularDiversidadPob(pob, tamPob, 2);

          t = clock() - tIni;
          time_search = ((double)t)/(CLOCKS_PER_SEC);

          indBest=0;
          if (pob[indBest]->getMakespan() < bestEval) {
            bestEval = pob[indBest]->getMakespan();
            timeBest = time_search;
            iterBest = numIter;
          }

          t = clock() - tIni;
          time_search = ((double)t)/(CLOCKS_PER_SEC);

          /* Imprimir datos de la Iteracion ... */
          cout <<  numIter 
            << " " << pob[indBest]->getMakespan() << " " << pob[indBest]->generacion 
            << " " << iterBest << " " << timeBest << " " << time_search 
            << " " << to_string(time_search - time_iterAnt) // timeIter      
            << " " << fitMax << " " << fitProm << " " << numExitos << " " << distPromPadres  
            << " " << fitPromHijos << " "<< bestHijo 
            << " " << entropia << " " << diversidad_0 
            << " " << diversidad_1 << " " << diversidad_2
            << "\n" << std::flush;

        }
      }
    } /* FIN PARALELIZACION */
    #endif
    
  }

  #ifdef AG_PARALLEL

  while (numProcesos > 1) {
    cout << "#Esperando a que todos los hijos terminen....\n" << std::flush;
    vector<int> buffMsg(tamMsg);
    MPI_Status status;
    // Esperar a que un hijo termine
    // Esperar MENSAJE de Cualquier fuente (ANY_SOURCE)
    MPI_Recv(&buffMsg[0], buffMsg.size(), MPI_INT, MPI_ANY_SOURCE, RES_TS, MPI_COMM_WORLD, &status);

    numProcesos--;

    int indH = buffMsg[0];

    timeHTS = UtilMPI::upd_Ind_msg_rcv(pob[tamPob], buffMsg);

    cout << "#\tHP[" << indH << "] " << getPrintInfo(pob[tamPob]) 
      << " " << timeHTS << "\n" << std::flush; 

    numExitos = 0;
    bestHijo = pob[tamPob]->getMakespan();
    fitPromHijos = bestHijo;
    distPromPadres = pob[tamPob]->distPadres;

    /* **************** REEMPLAZAMIENTO...... */
    generarValsRandom();
    sort(pob.begin(), pob.begin() + tamPob + 1, Diversidad::comparaMakespanSols );
    numExitos += pob[tamPob]->generacion == numIter ? 0 : 1;

    evalAct = pob[0]->getMakespan();
    fitProm=0;
    fitMin=evalAct;
    fitMax=evalAct;
    numInd = 0;

    for(int i=0; i < tamPob; i++ )  {
      evalAct = pob[i]->getMakespan();
      fitProm += evalAct;
      fitMin = evalAct < fitMin ? evalAct : fitMin;
      fitMax = evalAct > fitMax ? evalAct : fitMax;
      numInd++;
    }
    fitProm = numInd > 0 ? fitProm/numInd : fitProm;

    numIter++;
    entropia = Diversidad::calcularEntropiaPob(pob, tamPob);
    diversidad_0 = Diversidad::calcularDiversidadPob(pob, tamPob, 0);
    diversidad_1 = Diversidad::calcularDiversidadPob(pob, tamPob, 1);
    diversidad_2 = Diversidad::calcularDiversidadPob(pob, tamPob, 2);

    t = clock() - tIni;
    time_search = ((double)t)/(CLOCKS_PER_SEC);

    indBest=0;
    if (pob[indBest]->getMakespan() < bestEval) {
      bestEval = pob[indBest]->getMakespan();
      timeBest = time_search;
      iterBest = numIter;
    }

    t = clock() - tIni;
    time_search = ((double)t)/(CLOCKS_PER_SEC);

    /* Imprimir datos de la Iteracion ... */
    cout <<  numIter 
      << " " << pob[indBest]->getMakespan() << " " << pob[indBest]->generacion 
      << " " << iterBest << " " << timeBest << " " << time_search 
      << " " << to_string(time_search - time_iterAnt) // timeIter      
      << " " << fitMax << " " << fitProm << " " << numExitos << " " << distPromPadres  
      << " " << fitPromHijos << " "<< bestHijo 
      << " " << entropia << " " << diversidad_0 
      << " " << diversidad_1 << " " << diversidad_2
      << "\n" << std::flush;

  }
  #endif

  t = clock() - tIni;
  time_search = ((double)t)/(CLOCKS_PER_SEC);

  cout << "# Actualizando BEST, datos finales..." << "\n" << std::flush;

  sort(pob.begin(), pob.begin() + tamPob + 1, Diversidad::comparaMakespanSols );
  indBest = 0;
  int tamPobTMP = pob.size();
  while (pob.size() > tamPob) {
    delete pob[tamPobTMP-1];
    pob.pop_back(); 
    tamPobTMP = pob.size();
  }

  cout <<  numIter 
      << " " << pob[indBest]->getMakespan() << " " << pob[indBest]->generacion 
      << " " << iterBest << " " << timeBest << " " << time_search 
      << " " << to_string(time_search - time_iterAnt) // timeIter      
      << " " << fitMax << " " << fitProm << " " << numExitos << " " << distPromPadres  
      << " " << fitPromHijos << " "<< bestHijo 
      << " " << entropia << " " << diversidad_0 
      << " " << diversidad_1 << " " << diversidad_2
      << "\n" << std::flush;

  cout << "#Best Makespan PRTS: " << pob[indBest]->getMakespan() << "\n";
  cout << "#Total de Generaciones: " << numIter << "\n";
  cout << "#Time del Best: " << timeBest << "\n";
  cout << "#Iter del Best: " << iterBest << "\n";
  cout << "#Best, Es Factible: " << pob[indBest]->esFactible() << "\n" << std::flush;


  return pob[indBest]; 
}

void AlgoritmoGenetico::imprimirPoblacion() {
  if (!guardaPob) return;

  VPermutaciones permAct = pob[0]->getPermutacionSol();
  int m = permAct.size();
  int n = permAct[0].size();

  ofstream fileOut;
  fileOut.open(nameFile,std::ofstream::out | std::ofstream::app);
  if (fileOut.is_open()) {

    fileOut << "#P " << numIter << " " << tamPob << " " << m << " " << n << "\n" << std::flush;

    for(int k=0; k < tamPob; k++) {
      permAct = pob[k]->getPermutacionSol();
      fileOut << "#I " << pob[k]->getMakespan() << "\n#g";
      for(int i=0; i < m; i++) {
        for(int j=0; j < n; j++) {
          fileOut << " " << permAct[i][j] << " ";
          //cout << ((permAct[i][j]) / m) + 1 << " ";
        }
        // cout << "\n";
      }
      fileOut << "\n";
    }
    fileOut.flush();
    fileOut.close();
  }

}

void AlgoritmoGenetico::generarValsRandom() {
  for(int i=0; i < tamPob; i++) {
    pob[i]->valRdm = ((double) rand() / (RAND_MAX));
  }

  for(int i=0; i < hijos.size(); i++) {
    hijos[i]->valRdm = ((double) rand() / (RAND_MAX));
  }
}

string  AlgoritmoGenetico::getPrintInfo(Solucion *s) {
  string res;
  res = to_string(s->getMakespan()) + " " + to_string(s->numItTS) 
    + " " + to_string(s->generacion) + " " + to_string((int)s->distPadres)
    + " " + to_string(s->mksP1) + " " + to_string(s->mksP2)
    + " " + to_string((int)s->distAlosPadres)
    + " " + to_string((int)Diversidad::contribucionDivInd(s, pob, tamPob, 0))
    + " " + to_string((int)Diversidad::contribucionDivInd(s, pob, tamPob, 1))
    + " " + to_string((int)Diversidad::contribucionDivInd(s, pob, tamPob, 2))
    + " " + to_string(s->timeTS);
  return res;
}