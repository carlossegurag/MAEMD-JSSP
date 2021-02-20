#ifndef ITERATIVE_SEARCH_HPP
#define ITERATIVE_SEARCH_HPP

#include "Solucion.hpp"

class IterativeSearch {

  public:
    // numero de iteraciones realizadas
    int numIt;

    // tamanio lista tabu
    int tamLT = 0;

    // numero de iteraciones sin que se haya podido superar
    // el mejor valor
    int sinMejora;

    // numero de iteraciones en las cuales todos los
    // movimientos estan bloqueados por la lista tabu
    int totalBloqueos = 0;

    // Tiempo total de busqueda
    double time_search = 0;

    // Minimo numero de veces que una solucion se repite en la busqueda
    int minRep = 0;

    // Maximo numero de veces que una solucion se repite en la busqueda
    int maxRep = 0;

    // Numero promedio de veces que una solucion se repite en la busqueda
    double promRep = 0;

    // Numero de veces que se verifico factibilidad con resultado verdadero
    int numFactibles = 0;

    // Numero de veces que se verifico factibilidad con resultado falso
    int numNoFactible = 0;

  public:
    static int perturbarSolucionN(Solucion *s, int tipoN, int maxMovs, bool soloMejora);
    static int perturbarSolucion(Solucion *sol);
    int iterativeSearch(Solucion *sol, double maxTime);
};


#endif