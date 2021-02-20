#include "GJobShop.hpp"

#include <iostream>
#include <cstdlib>

GJobShop::GJobShop(JobShop *p) {
  int idOperacion = 0;

  this->numTrabajos = p->getNumTrabajos();
  this->numMaquinas = p->getNumMaquinas();

  int totalOps = this->numTrabajos*this->numMaquinas + 1;

  jobs = new vector<Operacion *>[this->numTrabajos];

  Operacion *opIni = new Operacion(-1, -1, 0, idOperacion++);
  opIni->setTInicio(0);

  N.push_back(opIni);

  Operacion *opFin = new Operacion(-1, -1, 0, totalOps);

  int **operaciones = p->getOperaciones();
  int ** tiempos = p->getTiempos();

  Operacion *opAnt = opIni;
  Operacion *opAct = NULL;
  Operacion *opSig = NULL;

  ops.resize(this->numMaquinas);
  for(int i=0; i < this->numMaquinas; i++) {
    ops[i].resize(this->numTrabajos);
  }

  for(int i=0; i < this->numTrabajos; i++) {
    opAnt = opIni;

    // Los id's se asignan por trabajos... 1-m para el trabajo 1, 
    // del (m+1)-(2m) para el trabajo 2, etc.
    for(int j=0; j < this->numMaquinas; j++) {
      opAct = new Operacion(i, operaciones[i][j], tiempos[i][j], idOperacion++);
      N.push_back(opAct);
      
      jobs[i].push_back(opAct);

      // ops[i][j], es el id de la operacion del trabajo j, en la maquina i
      ops[ operaciones[i][j] ][i] = opAct->getId();

      opAct->setJobAnterior(opAnt);

      if (opAnt->getId() != 0) {
        opAnt->setJobSiguiente(opAct);
      } else { // j = 0
        this->opsIni.push_back(opAct);
      }

      if(j == (this->numMaquinas - 1)) {
        
        opAct->setJobSiguiente(opFin);
        opsFin.push_back(opAct);
      }

      opAnt = opAct;

    }
  }

  N.push_back(opFin);
}

GJobShop::~GJobShop() {
  Operacion *op;

  delete [] jobs;

  while(this->N.size() > 0) {
    op = N.back();
    N.pop_back();
    delete op;
  }

}

vector<Operacion *> * GJobShop::getJobs() {
  return this->jobs;
}

vector<Operacion *>& GJobShop::getN() {
  return N;
}

Operacion * GJobShop::getLast() {
  int totalOps = this->numTrabajos*this->numMaquinas + 1;
  return N[totalOps];
}

vector<Operacion *>& GJobShop::getOpsIni() {
  return opsIni;
}

vector<Operacion *>& GJobShop::getOpsFin() {
  return opsFin;
}

int GJobShop::getNumTrabajos() {
  return this->numTrabajos;
}

int GJobShop::getNumMaquinas() {
  return this->numMaquinas;
}

string GJobShop::toString() {
  string res = "N = { " ;

  for(int i =0; i < this->N.size(); i++) {
    res = res + " " + N[i]->toString() + ", ";
  }
  res = res + "}\n";

/*  res += "Operaciones por Maquina: \n";
  for(int i =0; i < this->numMaquinas; i++) {
    res = res + "Maquina " + to_string(i+1) + ": \n";
    for(int j=0; j < this->numTrabajos; j++) {
      res = res + "\t(J " + to_string(this->E[i][j]->getJob()+1) 
        + ", id " + to_string(this->E[i][j]->getId())
        + ", orden " + to_string(this->E[i][j]->getId() - this->E[i][j]->getJob()*this->numMaquinas ) + ")\n";
    }
    res = res + "\n";
  }*/

  return res;
}