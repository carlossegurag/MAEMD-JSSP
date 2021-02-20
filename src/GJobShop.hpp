#ifndef GJOB_SHOP_HPP
#define GJOB_SHOP_HPP

#include <string>
#include <vector>
#include <utility> // std:pair  

using namespace std;

#include "Operacion.hpp"
#include "JobShop.hpp"

/* clase base para la representacion
* de soluciones. Se genera el modelo
 * de disjuntiva, (pero aun sin considerar
 las relaciones de operaciones en las maquinas,
 ya que esto depende de la solucion)
 */
class GJobShop {

    public:

    int numTrabajos;
    int numMaquinas;

    /* Nodos: lista de operaciones */
    vector<Operacion *> N;

    vector<Operacion *> *jobs;

    /* Operaciones iniciales */
    vector<Operacion *> opsIni;

    /* Operaciones finales */
    vector<Operacion *> opsFin;

    /* Almacena los ids correspondientes a cada trabajo,
        ordernados por # de Maquina. 
        ops[i][j], es el id de la operacion del trabajo j, en la maquina i */
    vector< vector<int> > ops;

  public:
    GJobShop(JobShop *p);
    ~GJobShop();

    int getNumTrabajos();
    int getNumMaquinas();

    vector<Operacion *> & getN();

    Operacion * getLast();

    vector<Operacion *> * getJobs();

    vector<Operacion *> & getOpsIni();
    vector<Operacion *> & getOpsFin();

    string toString();

};

#endif