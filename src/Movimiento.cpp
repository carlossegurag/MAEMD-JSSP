#include <climits>  // INT_MAX
#include <cmath>  // abs
#include "Movimiento.hpp"

using namespace std;

Movimiento::Movimiento(int tipo, Operacion *op1, Operacion *op2) {
  this->tipo = tipo;
  this->op1 = op1;
  this->op2 = op2;
  this->eval = INT_MAX;
}


bool Movimiento::operator<(const Movimiento &m) const {

  if (this->op1 == NULL || m.op1 == NULL
     || this->op2 == NULL || m.op2 == NULL) return this->eval < m.eval;

  if ( this->op1->getMachine() < m.op1->getMachine()) return true;
  if (this->op1->getMachine() > m.op1->getMachine()) return false;

  if (this->op1->getId() < m.op1->getId()) return true;
  if (this->op1->getId() > m.op1->getId()) return false;

  if (this->op2->getId() < m.op2->getId()) return true;
  if (this->op2->getId() > m.op2->getId()) return false;

  if (this->tipo < m.tipo) return true;
  if (this->tipo > m.tipo) return false;

/*  if (this->op1->getId() == m.op1->getId() 
      && this->op2->getId() == m.op2->getId() )  return false; */
  
  return false;

  /* PENDIENTE: verificar otros tipos de comparaciones... */

  /*  if (this->op1->getId() == m.op2->getId() 
      && this->op2->getId() == m.op1->getId() )  return false; */

/*
  if ( ( (this->op1->getId() == m.op2->getId()
          && this->op2->getId() == m.op1->getId()) 
        || (this->op1->getId() == m.op1->getId() 
          && this->op2->getId() == m.op2->getId()) )
      && abs(this->op1->getIndMach() - this->op2->getIndMach() ) == 1   )  return false;
*/

//  if (this->op1->getIndMach() < m.op1->getIndMach()) return true;
//  if (this->op1->getIndMach() > m.op1->getIndMach()) return false;

//  if (this->op2->getIndMach() < m.op2->getIndMach()) return true;
//  if (this->op2->getIndMach() > m.op2->getIndMach()) return false;

//  return this->eval < m.eval;
}

void Movimiento::actualizarValoresUV() {
  tAnt_U = op1->getTInicio();
  rtAnt_U = op1->getRT();

  tAnt_V = op2->getTInicio();
  rtAnt_V = op2->getRT();
}

string Movimiento::toString() {
  string res = "Mov Tipo: " + to_string(tipo) + ", en M";
  if (op1 != NULL && op2 != NULL) {
    res = res + to_string(op1->getMachine()) 
      + " : <indOp_" + to_string(indOp1) 
      + "(J" + to_string(op1->getJob() ) + ")," 
      // + ",indM" + to_string(op2->getIndMach()) 
      + "(J" + to_string(op2->getJob() ) + ")"
      + "; eval:" + to_string(this->eval) + ">" + ", seq:" + to_string(seq.size()) ;
  } else {
    res = res + " _ : _";
  }
  return res;
}