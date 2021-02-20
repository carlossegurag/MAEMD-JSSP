#ifndef MOVIMIENTO_HPP
#define MOVIMIENTO_HPP

#include <string>
#include <deque>
#include "Operacion.hpp"

using namespace std;

class Movimiento {

  public:
    int tipo;
    Operacion *op1;
    Operacion *op2;
    int eval;

    int maquina;
    deque<Operacion *> seq;
    int indOp1;
    int numIter;

    int tAnt_U;
    int rtAnt_U;

    int tAnt_V;
    int rtAnt_V;

  public:
    Movimiento(int tipo, Operacion *op1, Operacion *op2);
    //~Movimiento();  
    bool operator<(const Movimiento &m) const;

    void actualizarValoresUV();

    string toString();
};

#endif