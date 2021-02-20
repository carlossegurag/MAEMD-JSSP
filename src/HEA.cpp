#include <iostream>
#include <iomanip>  // std::setprecision
#include "HEA.hpp"
#include "Diversidad.hpp"

#ifdef AG_PARALLEL
#include "UtilMPI.hpp"
#include <mpi.h>
#endif

using namespace std;

HEA::HEA(JobShop *p, int tamPob, int valOpt, int max_itersTS) : AlgoritmoGenetico(p, tamPob, valOpt, 2, max_itersTS) {
  int tamTot = tamPob+2; 
  sP.resize(tamTot);
  for(int i=0; i < tamTot; i++) {
    sP[i].resize(tamTot);
  }

  tabuTenure=3;
}

HEA::~HEA() {

}

void HEA::calcularMatrizSP() {
  sPMax = -1-0;
  sPMin = this->numMaquinas*this->numTrabajos+1.0;

  mksMax=pob[0]->getMakespan();
  mksMin=pob[0]->getMakespan();

  for(int i=0; i < tamPob; i++) {
    pob[i]->score = 0;
    mksMax = pob[i]->getMakespan() > mksMax ? pob[i]->getMakespan() : mksMax;
    mksMin = pob[i]->getMakespan() < mksMax ? pob[i]->getMakespan() : mksMin;

    for(int j=i+1; j < tamPob; j++) {
      sP[i][j] = Diversidad::similitudInds(pob[i], pob[j]);
      sP[j][i] = sP[i][j];
      sPMax = sP[i][j] > sPMax ? sP[i][j] : sPMax;
      sPMin = sP[i][j] < sPMin ? sP[i][j] : sPMin;
    }
    sP[i][i] = -1;
    for(int j=0; j < tamPob; j++) {
      if (j==i) continue;
      sP[i][i] = sP[i][j] > sP[i][i] ? sP[i][j] : sP[i][i];
    }
  }
}

Solucion * HEA::optimizador(double beta,
  double maxTimeSearch, bool guardarPob, string nameFileOut) {

  this->guardaPob = guardarPob;
  if (guardaPob)
    this->nameFile = nameFileOut;

  tIni = clock();
  this->maxTime = maxTimeSearch;
  clock_t t = clock() - tIni;
  time_search = 0;

  this->numIter = 0;

  tipoMejora = MEJORA_TS;
  // Mejorar la poblacion:
  int indBest = this->mejorarPoblacion(0, 0, tamPob-1);
  calcularMatrizSP();

  // Guardar al Best
  int bestEval = pob[indBest]->getMakespan();
  vector<vector<int>> bestSol = pob[indBest]->getPermutacionSol();

  int posP1 = 0;
  int posP2 = 0;

  int indBestHijo = 0;
  int numHijos = 2;

  Solucion *tmpSol = NULL;

  double fitProm = 0;
  fitPromHijos = 0;
  fitPromPobM = 0;
  distPromPadHijos= 0;
  
  for(int i=0; i < tamPob; i++) fitProm += pob[i]->getMakespan();
  
  fitProm /= tamPob;

  double entropia = Diversidad::calcularEntropiaPob(pob);
  double diversidad_0 = 0;
  double diversidad_1 = 0;
  double diversidad_2 = 0;

  double time_muestreo = 0;
  double time_iterAnt = 0;
  timeBest = 0;
  iterBest = 0;

  numExitos = tamPob;
  bestHijo = pob[indBest]->getMakespan();

  numProcesos=1;  
  int totalOps = numMaquinas*numTrabajos;
  #ifdef AG_PARALLEL
  int tamMsg = totalOps*2+UtilMPI::NUM_DATA_EXTRA;
  double timeHTS=0;
  #endif

  while (time_search < maxTime
//    && (entropia > 0 )  
    && bestEval > valOpt) {

    t = clock() - tIni;
    time_search = ((double)t)/(CLOCKS_PER_SEC);

    fitPromHijos = 0;
    difPromHijos = 0;
    
    // Generamos a los hijos
      // Torneo Binario para generar padres
      //posP1 = this->seleccionaPadre();
      //posP2 = this->seleccionaPadre();
      posP1 = rand() % tamPob;  
      posP2 = rand() % tamPob;
//      while(posP2==posP1) posP2 = rand() % tamPob;

      // Cruza para generar a nuevo
      this->cruza(posP1, posP2, 0);

      if (posP1 == posP2) {
        posP1 = rand() % tamPob;
        posP2 = rand() % tamPob;
      }
      this->cruza(posP2, posP1, 1); 

    fitPromHijos = fitPromHijos/2;
    difPromHijos = difPromHijos/2;

    if (totalProcesos==1) {
      numIter++;
      // Mejoramos a todos los hijos:
      indBestHijo = mejorarPoblacion(1, 0, 1);

      // Actualizar al Best
      if (hijos[indBestHijo]->getMakespan() < bestEval ) {
        bestEval = hijos[indBestHijo]->getMakespan();
        bestSol = hijos[indBestHijo]->getPermutacionSol();

        timeBest = time_search;
        iterBest = numIter;
      }

      // Seleccion de nuevos individuos
      this->seleccionaIndividuos(beta);

      // Generar Log....
      entropia = Diversidad::calcularEntropiaPob(pob);
      diversidad_0 = Diversidad::calcularDiversidadPob(pob, pob.size(), 0);
      diversidad_1 = Diversidad::calcularDiversidadPob(pob, pob.size(), 1);
      diversidad_2 = Diversidad::calcularDiversidadPob(pob, pob.size(), 2);

      distPromPadres = numExitos != 0 ? distPromPadres/numExitos : distPromPadres;

      fitProm = 0;
      indBest = 0;

      for(int i=0; i < tamPob; i++) {
        fitProm += pob[i]->getMakespan();
        if ( Diversidad::comparaMakespanSols(pob[i], pob[indBest])) {
          indBest = i;
        }
      }
      fitProm /= tamPob;

      cout <<  numIter 
        << " " << pob[indBest]->getMakespan() << " " << pob[indBest]->generacion 
        << " " << iterBest << " " << timeBest << " " << time_search 
        << " " << to_string(time_search - time_iterAnt) // timeIter
        << " " << fitProm << " " << numExitos << " " << distPromPadres  
        << " " << fitPromHijos << " " << fitPromPobM << " " << bestHijo << " " << distPromPadHijos
        << " " <<  entropia << " " << diversidad_0 << " " << diversidad_1 << " " << diversidad_2
        << "\n" << std::flush;

    }// FIN SECUENCIAL
    #ifdef AG_PARALLEL
    else {
      // Mejorar a los hijos en paralelo
      for(int iH=0; iH < numHijos; iH++) {  
          /* MPI............ */
        vector<int> buffMsgRes;
        if (numProcesos < totalProcesos) {
          buffMsgRes = UtilMPI::make_msg_send_Ind(hijos[iH], iH);
          // Enviar Datos al proceso numProcesos
          MPI_Send( buffMsgRes.data(), buffMsgRes.size(), MPI_INT, numProcesos, TASK_TS, MPI_COMM_WORLD );

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

          t = clock() - tIni;
          time_search = ((double)t)/(CLOCKS_PER_SEC);

          // Actualizar al Best
          if (hijos[0]->getMakespan() < bestEval ) {
            bestEval = hijos[0]->getMakespan();
            bestSol = hijos[0]->getPermutacionSol();

            timeBest = time_search;
            iterBest = numIter;
          }

          /* **************** REEMPLAZAMIENTO...... */
          numIter++;
          // Seleccion de nuevos individuos
          this->seleccionaIndividuos(beta, 0);

          // Generar Log....
          entropia = Diversidad::calcularEntropiaPob(pob);
          diversidad_0 = Diversidad::calcularDiversidadPob(pob, pob.size(), 0);
          diversidad_1 = Diversidad::calcularDiversidadPob(pob, pob.size(), 1);
          diversidad_2 = Diversidad::calcularDiversidadPob(pob, pob.size(), 2);

          distPromPadres = numExitos != 0 ? distPromPadres/numExitos : distPromPadres;

          fitProm = 0;
          indBest = 0;

          for(int i=0; i < tamPob; i++) {
            fitProm += pob[i]->getMakespan();
            if ( Diversidad::comparaMakespanSols(pob[i], pob[indBest])) {
              indBest = i;
            }
          }
          fitProm /= tamPob;  

          cout <<  numIter 
            << " " << pob[indBest]->getMakespan() << " " << pob[indBest]->generacion 
            << " " << iterBest << " " << timeBest << " " << time_search 
            << " " << to_string(time_search - time_iterAnt) // timeIter
            << " " << fitProm << " " << numExitos << " " << distPromPadres  
            << " " << fitPromHijos << " " << fitPromPobM << " " << bestHijo << " " << distPromPadHijos
            << " " <<  entropia << " " << diversidad_0 << " " << diversidad_1 << " " << diversidad_2
            << "\n" << std::flush;

        }

      } // Fin for hijos paralelo 

    } // FIN ELSE: CODIGO EN PARALELO 
    #endif
    
  } // Fin WHile optimizador

  #ifdef AG_PARALLEL

  while(numProcesos > 1) {

    vector<int> buffMsg(tamMsg);
    MPI_Status status;
    // Esperar a que un hijo termine
    // Esperar MENSAJE de Cualquier fuente (ANY_SOURCE)
    MPI_Recv(&buffMsg[0], buffMsg.size(), MPI_INT, MPI_ANY_SOURCE, RES_TS, MPI_COMM_WORLD, &status);

    int indH = buffMsg[0];

    timeHTS = UtilMPI::upd_Ind_msg_rcv(hijos[0], buffMsg);

    cout << "#\tHP[" << indH << "] " << getPrintInfo(hijos[0]) 
      << " " << timeHTS << "\n" << std::flush; 

    t = clock() - tIni;
    time_search = ((double)t)/(CLOCKS_PER_SEC);

    bestHijo = hijos[0]->getMakespan();

    // Actualizar al Best
    if (hijos[0]->getMakespan() < bestEval ) {
      bestEval = hijos[0]->getMakespan();
      bestSol = hijos[0]->getPermutacionSol();

      timeBest = time_search;
      iterBest = numIter;
    }

    /* **************** REEMPLAZAMIENTO...... */
    numIter++;
    // Seleccion de nuevos individuos
    this->seleccionaIndividuos(beta, 0);

    // Generar Log....
    entropia = Diversidad::calcularEntropiaPob(pob);
    diversidad_0 = Diversidad::calcularDiversidadPob(pob, pob.size(), 0);
    diversidad_1 = Diversidad::calcularDiversidadPob(pob, pob.size(), 1);
    diversidad_2 = Diversidad::calcularDiversidadPob(pob, pob.size(), 2);

    distPromPadres = numExitos != 0 ? distPromPadres/numExitos : distPromPadres;

    fitProm = 0;
    indBest = 0;

    for(int i=0; i < tamPob; i++) {
      fitProm += pob[i]->getMakespan();
      if ( Diversidad::comparaMakespanSols(pob[i], pob[indBest])) {
        indBest = i;
      }
    }
    fitProm /= tamPob;  

    cout <<  numIter 
      << " " << pob[indBest]->getMakespan() << " " << pob[indBest]->generacion 
      << " " << iterBest << " " << timeBest << " " << time_search 
      << " " << to_string(time_search - time_iterAnt) // timeIter
      << " " << fitProm << " " << numExitos << " " << distPromPadres  
      << " " << fitPromHijos << " " << fitPromPobM << " " << bestHijo << " " << distPromPadHijos
      << " " <<  entropia << " " << diversidad_0 << " " << diversidad_1 << " " << diversidad_2
      << "\n" << std::flush;

    numProcesos--;
  }

  #endif

  pob[0]->setPermutacionSol(bestSol, true);
  indBest=0;

  cout << std::setprecision(17) <<  numIter 
    << " " << pob[indBest]->getMakespan() << " " << pob[indBest]->generacion 
    << " " << iterBest << " " << timeBest << " " << time_search 
    << " " << to_string(time_search - time_iterAnt) // timeIter
    << " " << fitProm << " " << numExitos << " " << distPromPadres  
    << " " << fitPromHijos << " " << fitPromPobM << " " << bestHijo << " " << distPromPadHijos
    << " " <<  entropia << " " << diversidad_0 << " " << diversidad_1 << " " << diversidad_2
    << "\n" << std::flush;

  imprimirPoblacion();

  cout << "#Best Makespan AG: " << pob[indBest]->getMakespan() << "\n";
  cout << "#Total de Generaciones: " << numIter << "\n";
  cout << "#Best, Es Factible: " << pob[indBest]->esFactible() << "\n" << std::flush;

  return pob[indBest];

}

void HEA::seleccionaIndividuos(double beta, int indH) {

  int totalHijos = 2;
  int tamTot = tamPob+totalHijos;
  numExitos=0;

  vector<Solucion *> pobTot = pob;

  if (indH < 0) {
    pobTot.push_back(hijos[0]);
    pobTot.push_back(hijos[1]);
  } else {
    totalHijos = 1;
    pobTot.push_back(hijos[indH]);
  }
  tamTot = pobTot.size();


  sPMax = -1-0;
  sPMin = this->numMaquinas*this->numTrabajos+1.0;

  mksMax = pobTot[0]->getMakespan();
  mksMin = pobTot[0]->getMakespan();

  /* Calcular matriz con similitud entre individuos */
  for(int i=0; i < tamTot; i++) {
    for(int j=tamPob; j < tamTot; j++) {
      if (i==j) continue;
      sP[i][j] = Diversidad::similitudInds(pobTot[i], pobTot[j]);
      sP[j][i] = sP[i][j];
    }
    sP[i][i] = -1;
    for(int j=0; j < tamTot; j++) {
      if (j==i) continue;
      sP[i][i] = sP[i][j] > sP[i][i] ? sP[i][j] : sP[i][i];
    }
    sPMax = sP[i][i] > sPMax ? sP[i][i] : sPMax;
    sPMin = sP[i][i] < sPMin ? sP[i][i] : sPMin;

    mksMax = pobTot[i]->getMakespan() > mksMax ? pobTot[i]->getMakespan() : mksMax;
    mksMin = pobTot[i]->getMakespan() < mksMin ? pobTot[i]->getMakespan() : mksMin;
  }

  int indMin = 0;

  /* Calcular SCORE de cada individuo... */
  for(int i=0; i < tamTot; i++) {
    pobTot[i]->score = beta * (( mksMax - pobTot[i]->getMakespan() ) / ( mksMax - mksMin + 1.0));
    pobTot[i]->score += (1.0-beta) * (( sPMax - sP[i][i] ) / ( sPMax - sPMin + 1.0));

/*    cout << "#\t PobTot[" << i << "], score=" << pobTot[i]->score 
      << ", mks=" << pobTot[i]->getMakespan() << ", sP=" << sP[i][i] << "\n"; */

    indMin = pobTot[i]->score < pobTot[indMin]->score ? i : indMin; 
  }

//  cout << "#  ->>IndMin=" << indMin<<"\n";

  Solucion *tmp = NULL;

  if (indH < 0) {
    if (indMin < tamPob) numExitos++;
    this->actualizarMatrizSP(indMin, tamPob+1);
    tmp = pobTot[indMin];
    pobTot[indMin] = pobTot[tamPob+1];
    pobTot[tamPob+1] = tmp;


    // Se debe recalcular sP[i][i], y el score para todos...
    mksMin = pobTot[0]->getMakespan();
    mksMax = pobTot[0]->getMakespan();

    sPMax = -1-0;
    sPMin = this->numMaquinas*this->numTrabajos+1.0;

    for(int i=0; i < tamTot-1; i++) {

      sP[i][i] = -1;

      for(int j=0; j < tamTot-1; j++) {
        if (i==j) continue;
        sP[i][i] = sP[i][j] > sP[i][i] ? sP[i][j] : sP[i][i];
      }

      sPMax = sP[i][i] > sPMax ? sP[i][i] : sPMax;
      sPMin = sP[i][i] < sPMin ? sP[i][i] : sPMin;

      mksMax = pobTot[i]->getMakespan() > mksMax ? pobTot[i]->getMakespan() : mksMax;
      mksMin = pobTot[i]->getMakespan() < mksMin ? pobTot[i]->getMakespan() : mksMin;
    }

    indMin = 0;

    for(int i=0; i < tamTot-1; i++) {
      pobTot[i]->score = beta * (( mksMax - pobTot[i]->getMakespan() ) / ( mksMax - mksMin + 1.0));
      pobTot[i]->score += (1.0-beta) * (( sPMax - sP[i][i] ) / ( sPMax - sPMin + 1.0));

      indMin = pobTot[i]->score < pobTot[indMin]->score ? i : indMin; 
    }
  }

//  cout << "#  ->>IndMin=" << indMin<<"\n";
  if (indMin < tamPob) numExitos++;
  this->actualizarMatrizSP(indMin, tamPob);
  tmp = pobTot[indMin];
  pobTot[indMin] = pobTot[tamPob];
  pobTot[tamPob] = tmp;

  int indPob = 0;

  for(int i=0; i < tamPob; i++) {
    pob[indPob++] = pobTot[i];
  }

  if (indH < 0 ) {
    hijos[0] = pobTot[tamPob];
    hijos[1] = pobTot[tamPob+1];
  } else {
    hijos[indH] = pobTot[tamPob];
  }

}

/* 
ind1: columna a reemplazar por ind2
ind2: columna a eliminar (su contenido ya no sera relevante)
*/
void HEA::actualizarMatrizSP(int ind1, int ind2) {
  if (ind1 >= tamPob) return;

  for(int i=0; i < tamPob; i++) {
    if (i == ind1) continue;
    sP[i][ind1] = sP[i][ind2];
    sP[ind1][i] = sP[i][ind1];
  }
}