#ifndef ALGORITMO_GENETICO_HPP
#define ALGORITMO_GENETICO_HPP

#include <list>
#include <vector>
#include <utility> // std:pair  
#include <ctime>
#include <sys/types.h>
#include <unistd.h>

#include "Solucion.hpp"

using namespace std;

#define MEJORA_TS 0
#define MEJORA_IS 1
#define MEJORA_ESCALADA_5 5
#define MEJORA_ESCALADA_7 7
#define MEJORA_ESCALADA_5_7 57

#define TIPO_MEJORA MEJORA_TS

#define SELEC_GENERACIONAL 0
#define SELEC_REEMP_PEORES 1
#define SELEC_MEJOR_NO_PENALIZADO 2
#define SELECCION_CLUSTER 3

#define CRUZA_LCS 0 // Subcadena Comun mas larga
#define CRUZA_MC 1 // Maquinas complementarias
#define CRUZA_PR 2 // Path Relinking considerando NCS
                    // (Elemenos no Comunes ) 
#define CRUZA_ORD 3 // Cruza por Orden de Operaciones 

class AlgoritmoGenetico {

  public:
    double time_search = 0;

    double timeBest=0;
    int iterBest=0;

    clock_t tIni;
    double maxTime;

    int numIter=0;
    int maxGeneraciones=1000;
    int tipoMejora = MEJORA_TS;

    double fracTiemFinPen=1.0;

    int tabuTenure=3;

    // numero de hijos q pasan a la sig generacion
    int numExitos=0;
    int bestHijo=0;
    double distPromPadres=0;
    double distPromPadHijos=0;

    //
    double porDInicial=0.5;
    int tipoDiversidad=0;

//  protected:

    JobShop *p;

    int numMaquinas;
    int numTrabajos;

    int valOpt;

    int tamPob;

    vector<Solucion *> pob;
    vector<Solucion *> hijos;

    double distPromHijos;
    double fitPromHijos;
    double difPromHijos;

    vector< vector<double> > distancias;

    int numCruzas;
    double fitPromPobM;

    bool guardaPob = false;
    string nameFile = "";

    //vector<Solucion *> & pob = arrSol1;
    //vector<Solucion *> & hijos = arrSol2;

    double pcr = 0.8;
    double pSelecc = 0;
    int max_itersTS;
    int deltaIncFP;
    int numGenSinMejora;
    int numPenalizados=0;

    /* PARALELO */
    unsigned int seedIni;   
    int totalProcesos=1;
    int numProcesos=0;
    vector<pid_t> pids;
    map<pid_t, int> pipesRead;
//    vector<int> pipesWrite;

  public:
    AlgoritmoGenetico(JobShop *p, int tamPob, int valOpt, int numHijos=0, int max_itersTS=0, int deltaIncFP=1);
    ~AlgoritmoGenetico();

    void calcularMatrizDistancias();
    void calcularDistanciasH(int iH, int pMin, int pMax);
    double calcularContribucion(vector<Solucion *> & new_pob, int tamNewPob, Solucion *cand, int D);
    void actualizarMatriz(int newInd, int indE);

    void selBNP(int D, list<Solucion *> & pobTot, vector<int> indsH,
      int tamMaxCluster);

    int seleccionaIndividuos(int tipoSeleccion, double D, int indHijo=-1);

    int seleccionaPadre();

    int cruza(int pos1, int pos2, int pos);
    int cruzaPorMaquinas(int pos1, int pos2, int pos);
    int cruzaPorOrden(int pos1, int pos2, int pos);

    int pathRelinking(int pos1, int pos2, int pos);

    int mejorarIndividuo(Solucion *ind);

    int mejorarPoblacion(int t, int pMin, int pMax, double maxTimePob=0);

    Solucion * optimizador(int maxGen, double maxTimeSearch, 
      int tipoSeleccion, int tipoCruza, 
      bool guardarPob = false, string nameFileOut = "", double porTimePobIni=0, double factAjInten=1);

    void imprimirPoblacion();

    Solucion * TSPR(double maxTimeSearch, int maxSinMejora=0);

    string getPrintInfo(Solucion *s);

    void generarValsRandom();

};

#endif