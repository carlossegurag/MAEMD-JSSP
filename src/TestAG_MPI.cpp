#include <iostream>
#include <string>
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand
#include <bits/stdc++.h>

#include "AlgoritmoGenetico.hpp"
#include "HEA.hpp"
#include "Solucion.hpp"
#include "GJobShop.hpp"
#include "JobShop.hpp"
#include "UtilJS.hpp"
#include "TabuSearch.hpp"
#include "PathRelinking.hpp"

#ifdef AG_PARALLEL
  #include "UtilMPI.hpp"
  #include <mpi.h>
  #include <sys/time.h>
#endif

using namespace std;

int main(int argc, char **argv) {

  unsigned int seed = time(NULL);

  int rank=0;
  int size=1;
#ifdef AG_PARALLEL
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
#endif
  //seed = 1554331933;
  srand(seed + rank);

  /* Fijamos los Parametros para utilizar Busqueda Tabu con 
  evaluacion por estimacion */
  Solucion::evalExacta = 0;
  Solucion::calcularCotas = 1;
  Solucion::soloFact = 1;
  TabuSearch::generarLog = false;

  int ejecucionLocal = argc > 7 ? atoi(argv[7]) : 1;

  string ruta="../";
  ruta = ejecucionLocal == 1 ? "" : ruta;

  string fileInst(argv[1]);
  string nombre_archivo(ruta + argv[1]);
  int valOpt = atoi(argv[2]);

  string nombreBase = ruta + argv[3];

  int tipoLS = argc > 6 ? atoi(argv[6]) : 0;

  // argv[9] es MaxSinMejora en Main.cpp
  int tipoAG = tipoLS;
  int maxSinMejora = argc > 0 ? atoi(argv[9]) : 0;

  JobShop *p = new JobShop(nombre_archivo);
  GJobShop *g = new GJobShop(p);

#ifdef AG_PARALLEL
  if (rank == 0 ) {
#endif
    cout << "#Seed: " << seed << endl << std::flush;

  auto *coutbuf = std::cout.rdbuf();
  ofstream out(nombreBase + "_" + to_string(seed) + "_" + fileInst.substr(5));
  if (ejecucionLocal != 1) {
    cout.rdbuf(out.rdbuf());
  }
  cout << "#Instancia: " << nombre_archivo << "\n" << std::flush;

  int tamPob = argc > 13 && atoi(argv[13]) > 3 ? atoi(argv[13]) : 30;

  int maxGeneraciones = 1000;

  double maxTime = argc > 8 ? atof(argv[8]) : 60; //60*60*24;

  int tipoSelecc =atoi(argv[4]);
  int tipoCruza = atoi(argv[5]);

  cout << "#Tamanio Pob: " << tamPob << "\n" << std::flush;
//  cout << "#Max Generaciones: " << maxGeneraciones << "\n" << std::flush;
  cout << "#MaxIters Sin Mejora LocalSearch: " << maxSinMejora << "\n" << std::flush;
  cout << "#Max tiempo Ejecucion: " << maxTime << " segs \n" << std::flush;
  cout << "#Tipo Selecc: " << tipoSelecc << "\n" << std::flush; 
  cout << "#Tipo Cruza: " <<  tipoCruza << "\n" << std::flush;

  string fileOutPob = nombreBase + "_" + to_string(seed) + "_pobs.txt";

  Solucion *best = NULL;
  double time_search =  0;
  int numIter = 0;
  double timeBest=0;
  int iterBest=0;

  Solucion::tipoReparacion = 2;//(argc > 12) ? atoi(argv[12]) : 2;

/*  switch(tipoLS) {
    case MEJORA_ESCALADA_5:
      cout << "# Mejora por Escalada N5 ... \n" << std::flush;
      break;
    case MEJORA_ESCALADA_7:
      cout << "# Mejora por Escalada N7 ... \n" << std::flush;
      break;
    case MEJORA_ESCALADA_5_7:
      cout << "# Mejora por Escalada N5, N7 ... \n" << std::flush;
      break;
    case MEJORA_IS:
      cout << "# Mejora por Iterative Search, ... \n" << std::flush;
      break;
    case MEJORA_TS:
    default:
      cout << "# Mejora por Tabu Search, ... \n" << std::flush;
      break;
  } */
  cout << "# Mejora por Tabu Search, ... \n" << std::flush;

  switch(tipoAG) {
    case 2: 
    {
      cout << "# Optimizador:  HEA \n" << std::flush;
      HEA HEA(p, tamPob, valOpt, maxSinMejora);
      HEA.totalProcesos=size;
      double beta = 0.6;
      best = HEA.optimizador(beta, maxTime, ejecucionLocal != 1, fileOutPob );
      best = new Solucion(g, valOpt, best->getPermutacionSol());
      numIter = HEA.numIter;
      time_search = HEA.time_search; 

      timeBest = HEA.timeBest;
      iterBest = HEA.iterBest;
      break;
    }
    case 1: 
    {
      cout << "# Optimizador: PR/TS \n" << std::flush;
      AlgoritmoGenetico AG(p, tamPob, valOpt, 2, maxSinMejora);
      AG.tipoMejora = 0; // TS
      AG.totalProcesos = size;
      
      best = AG.TSPR(maxTime, maxSinMejora);
      best = new Solucion(g, valOpt, best->getPermutacionSol());
      numIter = AG.numIter;
      time_search = AG.time_search;
      timeBest = AG.timeBest;
      iterBest = AG.iterBest;
      break;
    }
    case 0:
    default: 
    {
      cout << "# Optimizador:  AG+TS con manejo de Diversidad \n" << std::flush;

      AlgoritmoGenetico AG(p, tamPob, valOpt, 2, maxSinMejora);
      AG.tipoMejora = 0;

      AG.porDInicial = 0.5; //argc > 10 ? atof(argv[10]) : 0.5;
      AG.tipoDiversidad = argc > 11 ? atoi(argv[11]) : 0;
      //AG.totalProcesos = argc > 12 ? atoi(argv[12]) : 1;
      AG.totalProcesos = size;

      //AG.fracTiemFinPen = argc > 10 ? atof(argv[10]) : 1;
      AG.fracTiemFinPen = 1;
      /* AG.fracTiemFinPen > 0 && AG.fracTiemFinPen <= 1
        ? AG.fracTiemFinPen : 1; */
      // Factor para ajustar intensificacion
      double factAjInten = 1;//argc > 12 ? atof(argv[12]) : 1;

      // Porcentaje de tiempo para poblacion Inicial
      double porTimePobIni = 0; // argc > 11 ? atof(argv[11]) : 0;

      best = AG.optimizador(maxGeneraciones, maxTime, tipoSelecc, tipoCruza, 
        ejecucionLocal != 1, fileOutPob, porTimePobIni , factAjInten );
      best = new Solucion(g, valOpt, best->getPermutacionSol());
      numIter = AG.numIter;
      time_search = AG.time_search;

      timeBest = AG.timeBest;
      iterBest = AG.iterBest;
      break;
    }
  }

  cout << "# NumIter: " << numIter << "\n" << std::flush;
  cout << "# time: " << time_search << "\n" << std::flush;
  cout << "# BestMakespan: " << best->getMakespan() << "\n" << std::flush;

  vector<vector<int>> permBest = best->getPermutacionSol();

  cout << "# Guardando solucion en : " << nombreBase << "_sols.txt\n" << std::flush;  
  UtilJS::guardarPerms( permBest , nombreBase + "_sols.txt");

  cout << "# Guardando resultados en :" << nombreBase << "_res.txt\n" << std::flush;

  ofstream filePerms;
  filePerms.open((nombreBase + "_res.txt").c_str(), 
    std::ofstream::out | std::ofstream::app) ;
  if (filePerms.is_open()) {
    filePerms << argv[1] << " " << seed << " " << p->getNumTrabajos() 
      << "x" << p->getNumMaquinas() << " " << valOpt << " "
      << best->getMakespan() << " " << numIter 
      << " " << iterBest << " " << timeBest 
      << " " << time_search << "\n" << std::flush;

      // Time del Best, generacion del best  

    filePerms.flush();
    filePerms.close();
  }

  #ifdef AG_PARALLEL
  int tmp=0;
  for(int i=1; i < size; i++) {
    MPI_Send( &tmp, 1, MPI_INT, i, FINALIZE, MPI_COMM_WORLD ); 
  }
  #endif

  delete p;
  delete best;

  cout.rdbuf(coutbuf);
  
  if (ejecucionLocal != 1) {
    out.flush();
    out.close();
  }

#ifdef AG_PARALLEL
  } else { // RANK != 0 , MPI
    PathRelinking::generarLog = false;

    Solucion *s = new Solucion(g, valOpt);

    // s1, s2 usados para PR
    Solucion *s1 = new Solucion(new GJobShop(p), valOpt);
    Solucion *s2 = new Solucion(new GJobShop(p), valOpt);

    int totalOps = s->getNumMaquinas()*s->getNumTrabajos();
    vector<vector <int>> buffSol(s->getNumMaquinas(), vector<int>(s->getNumTrabajos(), 0) );
    
    int tamMsg = totalOps*2+UtilMPI::NUM_DATA_EXTRA;

    vector<int> buffMsg(tamMsg);

    // Esperar hasta recibir un mensaje
    MPI_Status status;
    MPI_Recv(&buffMsg[0], buffMsg.size(), MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    struct timeval startH;

    // Mientras TIPO_MENSAJE != SALIR
    while (status.MPI_TAG != FINALIZE) {
      startH.tv_sec = buffMsg[ tamMsg-2];
      startH.tv_usec = buffMsg[ tamMsg-1];

      int evalAct=0;

      switch(status.MPI_TAG) {
        case TASK_PR:
          // Leer solucion
          UtilMPI::upd_Ind_msg_rcv_PR(s1, s2, buffMsg);
          s->listaTabu.clear();

          evalAct = PathRelinking::pathRelinking(s1, s2, s, maxSinMejora);

          break;
        default:
        case TASK_TS:
          // Leer solucion
          UtilMPI::upd_Ind_msg_rcv(s, buffMsg);
          s->listaTabu.clear();

          // Realizar busqueda Tabu
          TabuSearch TS;
          TS.tabuTenure = 3;
          // Aplicamos una TS con criterio de paro por un max num de iters sin mejora
          evalAct = TS.tabuSearch(s, 7, 0, 0,  maxSinMejora);

          s->numItTS = TS.numIt;
          s->timeTS = TS.time_search;
          break;
      }

      vector<int> buffMsgRes = UtilMPI::make_msg_send_Ind(s, buffMsg[0]);

      buffMsgRes[tamMsg-2] = startH.tv_sec;
      buffMsgRes[tamMsg-1] = startH.tv_usec;

      // Enviar solucion al proceso 0
      MPI_Send( buffMsgRes.data(), buffMsgRes.size(), MPI_INT, 0, RES_TS, MPI_COMM_WORLD );
      
      // Esperar hasta recibir un mensaje
      MPI_Recv(&buffMsg[0], buffMsg.size(), MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    }
    delete p;
    delete s;
    delete s1;
    delete s2;
  }
  MPI_Finalize();
#endif 

  return 0;
}
