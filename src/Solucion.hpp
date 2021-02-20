#ifndef SOLUCION_HPP
#define SOLUCION_HPP

#include <string>
#include <vector>
#include <set>
#include <deque>
#include <utility> // std:pair  
#include <map>

using namespace std;

#include "Operacion.hpp"
#include "GJobShop.hpp"
#include "Movimiento.hpp"

typedef vector<vector<int>> VPermutaciones;

class Solucion {

  public:
    long numNoFactible=0;
    long numFactibles=0;
    long numMovs=0;
    long numEvalsCI=0;
    long numEvals=0;

    deque<Movimiento>  listaTabu;

    vector< int > x;
    
    vector<vector<int>> permTS;
    int numItTS=0;
    double timeTS=0;

    // Var para AG
    int generacion=0;
    double distPadres=0;
    double distAlosPadres=0;
    int mksP1=0;
    int mksP2=0;
    double score=0;
    double distMinPob0=0;
    double distMinPob1=0;
    double distMinPob2=0;
    bool esHijo=true;
    int indPob=0;
    double valRdm=0;

    //vector<Solucion *> cluster;
    vector<int> cluster;

    // Vars para TS
    vector< set<pair<int, int>> > bloques;

    /* Ruta Critica */
    deque<Operacion *> rutaC;

    /* Evaluacion de movimientos */
    static bool calcularCotas; // false
    static bool soloFact; // FALSE
    static bool evalExacta;
    static int tipoReparacion;

    int valOptimo=0;

  //private:
    int numMaquinas;
    GJobShop *g;
    
    /* m permutaciones del orden de los trabajos */
    /* cada permutacion determina el orden en que se deben
      ejecutar los trabajos en la maquina correspondiente */
    vector<Operacion *> *perms;

    /* Almacena la permutacion, de los ids de las operaciones,
    para cada maquina */
    vector<vector<int>> permSol;

    int makespan;

    vector<deque<Operacion *>> rutas;

    set<int> opsCriticas;

  public:
    Solucion(GJobShop *g, int valOpt, VPermutaciones perms);
    Solucion(GJobShop *g, int valOptimo);
    ~Solucion();
 
    int getNumMaquinas();
    int getNumTrabajos();
    int getMakespan();
    int getValOptimo();
    int getNumRutas();
    double getTamPromBloques();
    int getNumBloques();
    int getPesoOpsCriticas();

    void recalcularBloques();

    vector<Operacion *> & getOperaciones();

    int aplicarCambio(Movimiento mov); 

    int calcularMakespan(bool RC);

    vector<vector<int>> getPermutacionSol();
    void setPermutacionSol(vector<vector<int>> & pSol, bool calcularMkspn=true);

    string toString();

    string getPrintPerms();

    bool esFactible();

    int reparar();

    int validarPermutaciones();

    void calcularRutaCritica(Operacion *op, int val,bool actualizaRutas=true);
    void calcularRutaAlFinal(Operacion *opIni, bool actualizaBloq);
    void calcularRutaAlInicio(Operacion *opFin, bool actualizaBloq);

    /* Implementacion en SolucionN1.cpp */
    int evalMovIzquierda(Movimiento & mov,
      bool calcularMkspn, bool cambiar, bool checkFact,
      bool & checkTabu, deque<Movimiento> &listaTabu, int numIter,
      int evalBestN);

    int evalMovDerecha(Movimiento & mov,
      bool calcularMkspn, bool cambiar, bool checkFact,
      bool & checkTabu, deque<Movimiento> &listaTabu, int numIter,
      int evalBestN);

    void movIzquierda(Operacion * opU, Operacion * opV);
    void movDerecha(Operacion * opU, Operacion * opV);

    Movimiento getBestN1( deque<Movimiento > &listaTabu, 
      int itActual, int tamLT, int bestVal, bool bestAllN);

    Movimiento getBestN(deque<Movimiento> &listaTabu, 
      int itActual, int tamLT, int bestVal, int currentVal, bool bestAllN, int tipoN);

    vector<Movimiento> getMovimientosN1();

    bool esTabu( deque<Movimiento> & listaTabu, Movimiento & mov, 
        int itActual );

    int calcularCotaInferiorSeq(Movimiento &mov);


    /* Implementacion en SolucionN5.cpp : */
    Movimiento getBestN5(
      map<Movimiento, int > & listaTabu, int itActual, int tamLT,
      int bestVal, bool bestAllN )  ;

    Movimiento evaluarMejores(vector<Movimiento> movsBest);

    vector<Movimiento> getMovimientosN5();

    int calcularCotaInferiorSwap(Operacion *opAct, Operacion *opSig);

    /* Implementacion en SolucionN7.cpp : */
    Movimiento getBestN7(
     deque<Movimiento > &listaTabu, int itActual, int tamLT,
    int bestVal, bool bestAllN);

    vector<Movimiento> getMovimientosN7(bool extender=false);
    vector<Movimiento> getMovimientosN6();
    vector<Movimiento> getMovimientosN4(bool extender=false);

  private:
    void intercambiarOperaciones(Operacion *opI, Operacion *opJ);

    int recalcularST(Operacion *opIni, bool actualiza);
    int recalcularRT(Operacion *opIni, bool actualiza);

    void calcularRts();
/*    void calcularSts();*/

};

#endif