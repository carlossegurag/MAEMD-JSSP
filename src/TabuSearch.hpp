#ifndef TABU_SEARH_HPP
#define TABU_SEARH_HPP

#include <iostream>     // std::cout, std::ostream, std::io
#include <string>
#include <vector>
#include <deque>

using namespace std;

#include "Operacion.hpp"
#include "Solucion.hpp"
#include "Movimiento.hpp"

class TabuSearch {

  public:
    // numero de iteraciones realizadas
    int numIt;

    // tamanio lista tabu
    int tamLT;

    // numero de iteraciones sin que se haya podido superar
    // el mejor valor
    int sinMejora;

    // numero de iteraciones en las cuales todos los
    // movimientos estan bloqueados por la lista tabu
    int totalBloqueos;

    // Tiempo total de busqueda
    double time_search;

    // Minimo numero de veces que una solucion se repite en la busqueda
    int minRep;

    // Maximo numero de veces que una solucion se repite en la busqueda
    int maxRep;

    // Numero promedio de veces que una solucion se repite en la busqueda
    double promRep;

    // Numero de veces que se verifico factibilidad con resultado verdadero
    int numFactibles=0;

    // Numero de veces que se verifico factibilidad con resultado falso
    int numNoFactible=0;

    // Tipo de tabu ternure (criterio para tiempo de prohibicion)
    int tabuTenure=0;

    long long totalVecinos;
    long long totalEvals;
    long long totalEvalsCI;

    static bool generarLog;
    static int simpl1;
    static int simpl2;

  public:
    int tabuSearch(Solucion * sol, int tipoN, double maxTime, 
        int maxIters=-1, int maxSinMejora=-1, bool aplicarPerturb=false);
    static int escalada(Solucion *sol, int tipoN);

    static void actualizarListaTabu( deque<Movimiento> & listaTabu, 
        Movimiento & bestMov, int numIt, int tamLT );

    static bool simplificarVecindad(Solucion *s, Movimiento & bestMov);
};

#endif
