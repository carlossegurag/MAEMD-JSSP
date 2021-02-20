#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

#include "GJobShop.hpp"
#include "JobShop.hpp"
#include "Solucion.hpp"
#include "UtilJS.hpp"
#include "TabuSearch.hpp"
#include "Diversidad.hpp"
#include "AlgoritmoGenetico.hpp"

using namespace std;

int main(int argc, char **argv) {

  string nombreArchivoPob(argv[3]);

  cout << "Abriendo archivo: " << nombreArchivoPob << "\n";

  ifstream filePob;
  filePob.open(nombreArchivoPob.c_str());

  string instancia;
  string tmp;

  int numGen=0;
  int numInds=0;

  int numJobs=0;
  int numMachs=0;

  int makespan=0;

  string nombre_archivo(argv[1]);
  int valOpt = atoi(argv[2]);

  Solucion::evalExacta = 0;
  Solucion::calcularCotas = 1;
  Solucion::soloFact = 1;
  TabuSearch::generarLog = true;


  JobShop *p = new JobShop(nombre_archivo);
//  GJobShop *g = new GJobShop(p);

  vector<vector<Solucion *>> pobs;

  int numPobs = 0;

  if( filePob.is_open() ) {
    while(!filePob.eof()) {

      filePob >> tmp;
//      cout << tmp << "\n";
      if (tmp.compare("#P") != 0) {
        cout << "Fin archivo\n";
        break;
      }

      //    #P 1 30 20 50
      filePob >> numGen >> numInds >> numMachs >> numJobs;

//      cout << "Leyendo datos de la generacion: " << numGen 
//        << " " << numInds << " " << numMachs << " " << numJobs << "\n";

      vector<Solucion *> pob;
      pob.resize(numInds);

      for(int i=0; i < numInds; i++) {
        filePob >> tmp >> makespan; // lee #I mks
        filePob >> tmp; // lee #g

        vector<vector<int>> sIni(numMachs);

        for(int m=0; m < numMachs; m++) {
          sIni[m].resize(numJobs);
          for (int n=0; n < numJobs; n++) {
            filePob >> sIni[m][n];
          }
        }

        pob[i] = new Solucion(new GJobShop(p), valOpt, sIni);
//        cout << "datos del ind[" << i << "]" << " " << makespan << ", " << pob[i]->getMakespan() << "\n";

        if (makespan != pob[i]->getMakespan()) {
//          cout << "error, mksp diferentes... \n";
          exit(-1);
        }
      }
      pobs.push_back(pob);
      numPobs++;
    } 
    filePob.close();    
  }

  cout << "Generaciones leidas: " << pobs.size() << "\n";

  //int genSelec=numPobs-1;

  for (int genSelec=0; genSelec < 1; genSelec++) {
  vector<Solucion *> & pobF = pobs[genSelec];

  cout << "Diversidad Poblacion generacion[" << genSelec << "] : "
    << " " << Diversidad::calcularDiversidadPob(pobF, pobF.size(), 0)
    << " " << Diversidad::calcularDiversidadPob(pobF, pobF.size(), 1)
    << " " << Diversidad::calcularDiversidadPob(pobF, pobF.size(), 2)
  << "\n";

  /* Comparar contra una solucion */
  Solucion *s = NULL;
  if (argc > 4) {
    string archivoSol(argv[4]);
    GJobShop *g = new GJobShop(p);
    s = new Solucion(g, valOpt);    

    ifstream filePerms;
    filePerms.open(archivoSol.c_str());

    cout << "# Cargando archivo con sol inicial: " << archivoSol << "\n";
    vector<vector<int>> sIni;

    if (argc > 5 && atoi(argv[5]) == 1 ) 
      sIni = UtilJS::leerPerms(filePerms, s->getNumTrabajos(), s->getNumMaquinas(), g->ops);
    else
      sIni = UtilJS::leerPerms(filePerms);
    
    s->setPermutacionSol(sIni); 
    filePerms.close();

//    UtilJS::guardarPerms(sIni, "solBK_dmu80.txt");

      //cout << "# Sol INI: \n" << s->getPrintPerms() << "\n";
    cout << "Makespan Sol Leida: " << s->getMakespan() << "\n";

    cout << "Comparacion del la Sol contra la poblacion:" 
      << " " << Diversidad::contribucionDivInd(s, pobF, pobF.size(), 0)
      << " " << Diversidad::contribucionDivInd(s, pobF, pobF.size(), 1)
      << " " << Diversidad::contribucionDivInd(s, pobF, pobF.size(), 2)
      << "\n";

    for(int i=0; i < pobF.size(); i++ ) {
      cout << "Ind["<< i << "], mks=" << pobF[i]->getMakespan() << ", ContriDiv:"
        << " " << Diversidad::distanciaInds(s, pobF[i], 0)
        << " " << Diversidad::distanciaInds(s, pobF[i], 1)
        << " " << Diversidad::distanciaInds(s, pobF[i], 2)
        << "\n";
    }
  }

  }


  /* Aplicar busqueda Tabu a un ind 
  Solucion *s = pobF[rand() % pobF.size() ];
  TabuSearch ts;
  ts.tabuSearch(s, 7, 0, 0, 48000); */

  /* Cruzar individuos de la poblacion */
/*  AlgoritmoGenetico AG(p, pobF.size(), valOpt, 2, 12000);
  AG.pob = pobF;

  TabuSearch::generarLog = false;

  for(int i=0; i < pobF.size() ; i++) {
    for(int j=i+1; j < pobF.size(); j++) {
      double disPadres = Diversidad::distanciaInds(pobF[i], pobF[j]);

      AG.cruza(i, j, 0);
      AG.cruza(j, i, 1);

      double disPadrHijo0 = 
        min( Diversidad::distanciaInds(pobF[i], AG.hijos[0]) , 
          Diversidad::distanciaInds(pobF[j], AG.hijos[0]) );

      double disPadrHijo1 = 
        min( Diversidad::distanciaInds(pobF[i], AG.hijos[1]) , 
          Diversidad::distanciaInds(pobF[j], AG.hijos[1]) );

      int makesIniH0 = AG.hijos[0]->getMakespan();
      int makesIniH1 = AG.hijos[1]->getMakespan();

      int disIniHijos = Diversidad::distanciaInds(AG.hijos[0], AG.hijos[1]);

      TabuSearch TB0;
      TB0.tabuSearch(AG.hijos[0], 7, 0, 0, 12000);

      TabuSearch TB1;
      TB1.tabuSearch(AG.hijos[1], 7, 0, 0, 12000);

      double dis2PadrHijo0 = 
        min( Diversidad::distanciaInds(pobF[i], AG.hijos[0]) , 
          Diversidad::distanciaInds(pobF[j], AG.hijos[0]) );

      double dis2PadrHijo1 = 
        min( Diversidad::distanciaInds(pobF[i], AG.hijos[1]) , 
          Diversidad::distanciaInds(pobF[j], AG.hijos[1]) );

      cout << "Mks Padres: " << pobF[i]->getMakespan() << " " << pobF[j]->getMakespan()
        << ", MksIni hijos: " << makesIniH0 << " " << makesIniH0
        << ", Mks hijos: " << AG.hijos[0]->getMakespan() << " " << AG.hijos[1]->getMakespan()
        << ", Dist Padres: " << disPadres << ", Dist ini Hijos: " << disIniHijos
        << ", Dist Hijos: " << Diversidad::distanciaInds(AG.hijos[0], AG.hijos[1]) 
        << ", disPadHijo_0: " << disPadrHijo0 << " , disPadHijo_1: " << disPadrHijo1 
        << ", dis2PadHijo_0: " << dis2PadrHijo0 << " , dis2PadHijo_1: " << dis2PadrHijo1 << "\n" << std::flush;
    }
  }
*/

/* Aplicar Busca Tabu a toda la poblacion 
  for(int i=0; i < pobF.size(); i++) {
    makespan = pobF[i]->getMakespan();
//    cout << "datosF del ind[" << i << "]" << " " << makespan << ", " << pobF[i]->getMakespan() << "\n";

    cout << "ind[" << i << "] " << pobF[i]->getMakespan() << " ";
    TabuSearch TB;
    TB.tabuSearch(pobF[i], 7, 0, 0, 48000);
    cout << " " << pobF[i]->getMakespan() << "\n";
  } 
*/

  delete p;
  
  return 0;
} 