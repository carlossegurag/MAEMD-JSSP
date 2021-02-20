#include "JobShop.hpp"

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>

JobShop::JobShop(string nombreArchivo) {

  ifstream file;
  file.open(nombreArchivo.c_str());

  operaciones = NULL;
  tiempos = NULL;

  if( file.is_open() ) {    
      file >> this->numTrabajos >> this->numMaquinas;
      
      this->operaciones = new int*[this->numTrabajos];
      this->tiempos = new int*[this->numTrabajos];

      for(int i =0; i < this->numTrabajos; i++) {
        this->operaciones[i] = new int[this->numMaquinas];
        this->tiempos[i] = new int[this->numMaquinas];

        // Primero se leen todas las operaciones de un trabajo
        for(int j=0; j < this->numMaquinas; j++) {
          file >> this->operaciones[i][j] >> tiempos[i][j];
        }
      }
      file.close();
  }
}

JobShop::~JobShop() {
  if (this->numTrabajos > 0) {
    for(int i=0; i < this->numTrabajos; i++) {
      delete [] this->operaciones[i];
      delete [] this->tiempos[i];
    }
    delete [] this->operaciones;
    delete [] this->tiempos;

    /*
    for(int i=0; i < this->getNumMaquinas; i++) {
      delete [] this->opsMaquinas[i];
    }
    delete [] this->opsMaquinas;*/
  }
}

int JobShop::getNumMaquinas() {
  return this->numMaquinas;
}

int JobShop::getNumTrabajos() {
  return this->numTrabajos;
}

int ** JobShop::getOperaciones() {
  return this->operaciones;
}

int ** JobShop::getTiempos() {
  return this-> tiempos;
}

string JobShop::toString() {
  string res = "Num Trabajos: " + to_string(this->numTrabajos) + "\n";
  res = res +  "Num Maquinas: " + to_string(this->numMaquinas) + "\n";

  res += "Secuencia de Operaciones\n";
  for(int i =0; i < this->numTrabajos; i++) {
    res = res + "Trabajo " + to_string(i+1) + ": \n";
    for(int j=0; j < this->numMaquinas; j++) {
      res = res + "\t(M " + to_string(this->operaciones[i][j]+1) 
        + ", t " + to_string(this->tiempos[i][j]) + ")\n";
    }
    res = res + "\n";
  }

  return res;
}