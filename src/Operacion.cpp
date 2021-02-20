#include "Operacion.hpp"

#include <iostream>
#include <cstdlib>

Operacion::Operacion(int job, int machine, int duration, int id ) {
  this->job = job;
  this->machine = machine;
  this->duration = duration;
  this->id = id;
  this->tInicio = 0;
  this->rt = 0;

  this->indMach = -1;

  this->jobAnt=NULL;
  this->jobSig=NULL;

  this->machAnt = NULL;
  this->machSig = NULL; 

  this->isRC = false;
  this->posRC = -1;
}

Operacion::~Operacion() {
}

int Operacion::getTInicio() {
  return this->tInicio;
}

void Operacion::setTInicio(int t) {
  this->tInicio = t;
}

int Operacion::getRT() {
  return this->rt;
}

void Operacion::setRT(int t) {
  this->rt = t;
}

int Operacion::getIndMach() {
  return this->indMach;
}

void Operacion::setIndMach(int ind) {
  this->indMach = ind;
}

bool Operacion::getIsRC() {
  return this->isRC;
}

void Operacion::setRC(bool val) {
  this->isRC = val;
}

int Operacion::getPosRC() {
  return this->posRC;
}

void Operacion::setPosRC(int pos) {
  this->posRC = pos;
}

int Operacion::getId(){
  return this->id;
}

int Operacion::getJob() {
  return this->job;
}

int Operacion::getMachine() {
  return this->machine;
}

int Operacion::getDuration() {
  return this->duration;
}

Operacion * Operacion::getJobAnterior() {
  return this->jobAnt;
}

void Operacion::setJobAnterior(Operacion *op) {
  this->jobAnt=op;
}

Operacion * Operacion::getJobSiguiente() {
  return this->jobSig;
}

void Operacion::setJobSiguiente(Operacion *op) {
  this->jobSig = op;
}

Operacion * Operacion::getMachAnterior() {
  return this->machAnt;
}

void Operacion::setMachAnterior(Operacion *op) {
  this->machAnt = op;
}

Operacion * Operacion::getMachSiguiente() {
  return this->machSig;
}

void Operacion::setMachSiguiente(Operacion *op) {
  this->machSig = op;
}

Operacion * Operacion::clone() {
  Operacion *res = new Operacion(this->job, this->machine, this->duration, this->id);
  res->setTInicio(this->getTInicio());
  res->setIndMach(this->getIndMach());
  res->setRC(this->getIsRC());
  res->setJobAnterior(this->getJobAnterior());
  res->setJobSiguiente(this->getJobSiguiente());
  res->setMachAnterior(this->getMachAnterior());
  res->setMachSiguiente(this->getMachSiguiente());

  return res;
}

string Operacion::toString() {
  string res = "Op " + to_string(this->id);
  res = res + " (J " + to_string(this->job+1);
  res = res + ",M " + to_string(this->machine+1);
  res = res + ", d  " + to_string(this->duration);
  res = res + ")";
  return res;
}