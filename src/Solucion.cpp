#include <iostream>
#include <cstdlib>
#include <algorithm>    // std::random_shuffle
#include <climits>  // INT_MAX
#include <set>
#include <map>

#include "Operacion.hpp"
#include "Solucion.hpp"

bool Solucion::evalExacta=false;
bool Solucion::calcularCotas=true;
bool Solucion::soloFact=true;
int Solucion::tipoReparacion=2;

Solucion::Solucion(GJobShop *g, int valOpt, VPermutaciones perms) {
  this->numMaquinas = g->getNumMaquinas();
  this->g = g;
  this->perms = new vector<Operacion *>[this->numMaquinas];

  this->valOptimo = valOpt;
  this->bloques.resize(numMaquinas);

  this->permSol.resize(this->numMaquinas);

  for(int i=0; i < numMaquinas; i++) {
    this->permSol[i].resize(g->getNumTrabajos());
    this->perms[i].resize(g->getNumTrabajos());
  }

  this->setPermutacionSol(perms, true);
}

Solucion::Solucion(GJobShop *g, int valOpt) {
  this->numMaquinas = g->getNumMaquinas();
  this->g = g;
  this->perms = new vector<Operacion *>[this->numMaquinas];

  this->valOptimo = valOpt;

  vector<Operacion *> ops(g->getOpsIni());
//  vector<Operacion *> opsFin = g->getOpsFin();

  int indice = 0;

  int indMach = 0;

  Operacion *op = NULL;

  this->makespan = INT_MAX;

  while (ops.size() > 0) {
    // Elegimos una operacion al azar:
    indice = rand() % ops.size();

    // Eligimos la operacion con menor tiempo
    // PENDIENTE: implementar con un heap
/*    indice = 0;
    int numMin=1;
    for(int i=0; i < ops.size(); i++) {
      if (ops[i]->getDuration() < ops[indice]->getDuration() ) {
        indice = i;
        numMin=1;
      }
      else if (ops[i]->getDuration() == ops[indice]->getDuration()) {
        numMin++;
        // de manera aleatoria elegimos entre los minimos encontrados
        if (rand() % numMin == 0) indice = i;
      }

    } */

    op = ops[indice];

    // La eliminamos del conjunto de pendientes
    ops.erase(ops.begin() + indice); 

    // Asignamos el orden en que se procesara en la maquina
    indMach = this->perms[ op->getMachine() ].size();
    op->setIndMach(indMach);

    // La planificamos en la maquina correspondiente
    this->perms[ op->getMachine() ].push_back(op);

    // Actualizamos referencias de MAquinas
    if (indMach > 0) {
      op->setMachAnterior(perms[op->getMachine()][indMach - 1] );
      perms[op->getMachine()][indMach-1]->setMachSiguiente(op);
    }

    // Agregamos la siguiente operacion disponible para planificar
    if ( op->getJobSiguiente() != NULL && op->getJobSiguiente()->getJob() != -1) {
      ops.push_back(op->getJobSiguiente());
    }
  }

  this->permSol.resize(this->numMaquinas);

  for(int i=0; i < numMaquinas; i++) {
    this->permSol[i].resize(g->getNumTrabajos());
    for(int j=0; j < g->getNumTrabajos(); j++) {
      permSol[i][j] = perms[i][j]->getId();
    }
  }

  this->bloques.resize(numMaquinas);

  // Calculamos makespan y rutas criticas 
  this->makespan = this->calcularMakespan(true);

  numNoFactible = 0;
  numFactibles = 0;
}

Solucion::~Solucion() {
  delete [] this->perms;  
  delete g;
} 

int Solucion::getNumMaquinas() {
  return this->numMaquinas;
}

int Solucion::getNumTrabajos() {
  return this->g->getNumTrabajos();
}

int Solucion::getValOptimo() {
  return this->valOptimo;
}

int Solucion::getMakespan() {
  return this->makespan;
}

int Solucion::getNumRutas() {
  return this->rutas.size();
}

int Solucion::getNumBloques() {
  int numBloq = 0;
  for(int i=0; i < this->bloques.size(); i++) {
    numBloq += bloques[i].size();
  }
  return numBloq;
}

void Solucion::recalcularBloques() {
  Operacion * JP_U;
  Operacion * JS_V;

  for(int i=0; i < bloques.size(); i++) {
    bloques[i].clear();
    bool estaEnBloque=false;
    int tamBloque=0;
    vector<int> indIni;
    vector<int> indFin;
    for(int j=0; j < perms[i].size(); j++) {
      if (perms[i][j]->getTInicio() + perms[i][j]->getRT() == this->makespan) {
        estaEnBloque=true;
        tamBloque++;

        JP_U = perms[i][j]->getJobAnterior();
        JS_V = perms[i][j]->getJobSiguiente();

        if (JP_U != NULL && JP_U->getJob() != -1 
          && JP_U->getTInicio() + JP_U->getRT() == this->makespan ) {
          indIni.push_back(j);
        }

        if (tamBloque > 1 && JS_V != NULL && JS_V->getJob() != -1
          && JS_V->getTInicio() + JS_V->getRT() == this->makespan ) {
          indFin.push_back(j);
        }
      } else {
        estaEnBloque = false;
        tamBloque = 0;

        //if (indIni.size() != 0 && indFin.size() != 0)
          //cout << "indIni.size()=" << indIni.size() << ", indFin.size()=" << indFin.size() << "\n";

        // Procesar todas las combinaciones de inicio y fin...
        for(int k=0; k < indIni.size(); k++) {
          for(int l=0; l < indFin.size(); l++) {
            if (indIni[k] < indFin[l]) {
              cout << "#\tnb M" << i << ", bloque["<< indIni[k] << ", " << indFin[l] << "]\n";
              bloques[i].insert(make_pair( indIni[k], indFin[l] ));
            }
          }
        }
        indIni.clear();
        indFin.clear();
      }
    }
    // combinar
    // Procesar todas las combinaciones de inicio y fin...
    for(int k=0; k < indIni.size(); k++) {
      for(int l=0; l < indFin.size(); l++) {
        if (indIni[k] < indFin[l]){
          cout << "#\tnb2 M" << i << ", bloque["<< indIni[k] << ", " << indFin[l] << "]\n";
          bloques[i].insert(make_pair( indIni[k], indFin[l] ));
        }
      }
    }
  }
}

double Solucion::getTamPromBloques() {
  double prom = 0;

  int numBloq = 0;

  for(int i=0; i < this->bloques.size(); i++) {
    for (set<pair<int, int>>::iterator it=bloques[i].begin(); it!=bloques[i].end(); ++it) {
      pair<int, int> bloq = *it;
      prom += 1 + (bloq.second - bloq.first);
      numBloq++;
    }
  }
  prom = numBloq != 0 ? prom/numBloq : prom;

  return prom;
}

int Solucion::getPesoOpsCriticas() {
  int peso = 0;

  vector<Operacion *> N = this->g->getN();

  for (set<int>::iterator it=opsCriticas.begin(); it!=opsCriticas.end(); ++it) {
      int idOp = *it;
      peso += N[idOp]->getDuration();
  }

  return peso;
}

vector<Operacion *> & Solucion::getOperaciones() {
  return this->g->getN();
}

#include "SolucionN1.cpp"
#include "SolucionN5.cpp"
#include "SolucionN7.cpp"

vector<vector<int>> Solucion::getPermutacionSol() {
  return this->permSol;
}

void Solucion::setPermutacionSol(vector<vector<int>> & pSol, bool calcularMkspn) {
 
  vector<Operacion *> N = this->g->getN();

  for(int i=0; i < this->numMaquinas; i++) {
    for(int j=0; j < g->getNumTrabajos(); j++) {
      this->permSol[i][j] = pSol[i][j];
      this->perms[i][j] = N[pSol[i][j]];

      this->perms[i][j]->setMachSiguiente(NULL);
      this->perms[i][j]->setMachAnterior(NULL);
      this->perms[i][j]->setIndMach(j);

      if (j > 0 ) {
        perms[i][j-1]->setMachSiguiente(perms[i][j]);
        perms[i][j]->setMachAnterior(perms[i][j-1]);
      }
    }
  }

  if (calcularMkspn)
    this->calcularMakespan(true);
}

int Solucion::aplicarCambio(Movimiento mov) {
  Operacion *opU = mov.op1;
  Operacion *opV = mov.op2;

  int nMakespan = this->makespan;

  if (opU == NULL || opV == NULL) {
    return this->calcularMakespan(true);
  }

  deque<Movimiento> listaTabu;

  bool checkTabu = false;
  bool calcularMks = true;
  bool cambiar = true;
  bool checkFact = false;

  switch(mov.tipo) {
    case 1: // insertarDerecha(opU, opV);
    case 4: // moverDerecha(opU, opV)
      nMakespan = evalMovDerecha(mov, calcularMks, cambiar, 
        checkFact, checkTabu, listaTabu, 0, INT_MAX);
      break;
    case 2: // moverIzquierda(opU, opV)
    case 3: // insertarIzquierda(opU, opV)
      nMakespan = evalMovIzquierda(mov, calcularMks, cambiar, 
        checkFact, checkTabu, listaTabu, 0, INT_MAX);
      break;
    default:
    case 0:
      this->intercambiarOperaciones(opU, opV);
      break;
  }
  nMakespan = this->calcularMakespan(true);
  return nMakespan;
}

int Solucion::calcularMakespan(bool RC) {
  numEvals++;
  // ops : sera el conjunto de operaciones 
  // tales que sus predecesores (maquina y trabajo)
  // ya se han sido planificadas
  deque<Operacion *> ops;

  // Operaciones inciales para cada trabajo (Jobs)
  vector<Operacion *> opsIni = g->getOpsIni();
  Operacion *opAux = NULL;

  // ops = {opsIni Jobs} INTERSECCION {opsIni Maquinas}
  // Calculamos las operaciones iniciales que no tienen predecesores
  // (ni en el trabajo ni en la maquina )
  for(int i=0; i < opsIni.size(); i++) {
    opAux = opsIni[i];
    if ( opAux->getId() == perms[opAux->getMachine()][0]->getId() ) {
      ops.push_back(opAux);
    }
  }

  this->makespan = 0;

  int totalOps = (g->getNumTrabajos())*(g->getNumMaquinas()+2); 
  int etiquetas[totalOps] = {0};
  for(int i=0; i < totalOps; i++ ) {
    etiquetas[i] = 0;
  }

  Operacion *op = NULL;
  Operacion *opRC = NULL;

  int tJob = 0;
  int tMach = 0;

  vector<Operacion *> opsFinMach;

  while (ops.size() > 0) {
    // Seleccionamos un elemento de ops
    op = ops.front();
    op->setRC(false);

    // y lo eliminamos
    ops.pop_front();

    // Lo etiquetamos 
    etiquetas[op->getId()] = 1;

    // Calculamos los tiempos:
    tJob = op->getJobAnterior() == NULL || op->getJobAnterior()->getId() == 0 ? 0 :
      op->getJobAnterior()->getTInicio() + op->getJobAnterior()->getDuration();

    tMach = op->getMachAnterior() == NULL || op->getMachAnterior()->getId() == 0 ? 0 :
      op->getMachAnterior()->getTInicio() + op->getMachAnterior()->getDuration();

    // Calculamos ri = st = tiempoInicio
    op->setTInicio( max( tMach, tJob ) );

    // Actualizamos makespan = max (tInicio + duracion)
    if (makespan <= (op->getTInicio() + op->getDuration()) ) {
      if (makespan < (op->getTInicio() + op->getDuration()) ) {
        this->makespan = op->getTInicio() + op->getDuration();
        opsFinMach.clear();
        opRC = op;
      }

      // Actualizamos operaciones finales
      if ( op->getId() == perms[op->getMachine()][g->getNumTrabajos()-1]->getId() ) {
        // Si es una operacion final, la agregamos a la lista
        opsFinMach.push_back(op);
      }
    }

    // Predecesor maquina del sucesor en el trabajo de la operacion actual 
    // PM[ SJ[op] ]
    opAux = op->getJobSiguiente();
    if (opAux != NULL  && !etiquetas[opAux->getId()]) {
      opAux = opAux->getMachAnterior();

      if (( opAux == NULL || opAux->getId() == 0 || etiquetas[opAux->getId()] ) 
        && (op->getJobSiguiente() != NULL) ) {
        ops.push_back(op->getJobSiguiente());
      }
    }

    // Predecesor en el trabajo del sucesor en la maquina actual
    // PJ[ SM[op] ]
    opAux = op->getMachSiguiente();
    if (opAux != NULL && !etiquetas[opAux->getId()]) {
      opAux = opAux->getJobAnterior();

      if ( (opAux == NULL || opAux->getId() == 0 || etiquetas[opAux->getId()]) 
        && (op->getMachSiguiente() != NULL) ) {
        ops.push_back(op->getMachSiguiente());
      }
    } 
  }

  Operacion * last = g->getLast();
  last->setTInicio(makespan);

  if (RC) {
    // Calcular lontigud mas larga desde cada operacion
    this->calcularRts();

    opsCriticas.clear();
    rutas.clear();
    for(int i=0; i < numMaquinas; i++) {
      bloques[i].clear();
    }

    //cout << "calcularRC, Makespan: " << this->makespan << "\n";
    // CAlcular operaciones finales
    for(int i=0; i < opsFinMach.size(); i++) {
      opRC = opsFinMach[i];
      //cout << "OpFin Mach, tInicio: " << opRC->getTInicio() << ", dur: " << opRC->getDuration() << "\n";
      if ( (opRC->getTInicio() + opRC->getDuration()) == this->makespan) {
        rutaC.clear();
        //cout << "Calculando RC desde op " << opRC->getId() << "\n";
        this->calcularRutaCritica(opRC, this->makespan);
      }
    } 

    // imprimir Bloques:
    /* cout << "\n\tCalcular Makespan, Bloques: \n";

    for(int i=0; i < numMaquinas; i++) {
      if (bloques[i].size() > 0) {
        cout << "\tNum bloques en maquina " << (i+1) << ": " << bloques[i].size() << "\n";        
        for(auto bloq : bloques[i]) {
          cout << "\t\t[" << bloq.first << "," << bloq.second << "]\n";
        }
      }
    } */

  }

  return this->makespan;
} // FIN CALCULAR_MAKESPAN

void Solucion::calcularRts() {

  deque<Operacion *> ops;
  Operacion *opAux = NULL;
  Operacion *opSig = NULL;

  // Calculamos las operaciones finales que no tienen sucesores
  // (ni en el trabajo ni en la maquina )
  for(int i=0; i < this->numMaquinas; i++) {
    opAux = perms[i][ perms[i].size()-1]; // ultima operacion maquina
    opSig = opAux->getJobSiguiente();

    // verificamos si tiene operacion sucesora en el trabajo
    if (opSig == NULL || opSig->getJob() == -1 ) {
      ops.push_back(opAux);
    }
  }

  int totalOps = (g->getNumTrabajos())*(g->getNumMaquinas()+2);
  //int etiquetas[totalOps] = {0};
  vector<int> etiquetas(totalOps, 0);

  Operacion *op = NULL;

  int tJob = 0;
  int tMach = 0;

  int rt  = 0;
  int maxRt = 0;

  while(ops.size() > 0 ) {
    // Seleccionamos un elemento de ops
    op = ops.front();

    // Lo etiquetamos 
    etiquetas[op->getId()] = 1;

    // y lo eliminamos
    ops.pop_front();

    // Calculamos los tiempos:
    tJob = op->getJobSiguiente() == NULL || op->getJobSiguiente()->getId() == -1 ? 0 :
      op->getJobSiguiente()->getRT();

    tMach = op->getMachSiguiente() == NULL || op->getMachSiguiente()->getId() == -1 ? 0 :
      op->getMachSiguiente()->getRT();

    rt = max( tMach, tJob ) + op->getDuration();

    // Calculamos rt = ruta mas larga... desde esta op.
    op->setRT( rt );

    maxRt = rt > maxRt ? rt : maxRt;

    // sucesor maquina del predecesor en el trabajo de la operacion actual 
    // SM[ PJ[op] ]
    opAux = op->getJobAnterior();
    if (opAux != NULL  && !etiquetas[opAux->getId()]) {
      opAux = opAux->getMachSiguiente();

      if (( opAux == NULL || opAux->getJob() == -1 || etiquetas[opAux->getId()] ) ) {
        ops.push_back(op->getJobAnterior());
      }
    }

    // sucesor en el trabajo del predecesor en la maquina actual
    // SJ[ PM[op] ]
    opAux = op->getMachAnterior();
    if (opAux != NULL && !etiquetas[opAux->getId()]) {
      opAux = opAux->getJobSiguiente();

      if ( (opAux == NULL || opAux->getJob() == -1 || etiquetas[opAux->getId()]) ) {
        ops.push_back(op->getMachAnterior());
      }
    }

  }
  
} // FIN_CALCULAR_RTS

// Calcula rutas (y bloques) desde opRC hacia la opInicio (Id=0)
void Solucion::calcularRutaCritica(Operacion *opRC, int val, bool actualizaRutas ) {
  bool actualiza = true;

  if (opRC != NULL && opRC->getJob() != -1 
    && (val - opRC->getDuration()) == ( opRC->getTInicio() ) ) {

    int iniBloque = 0;
    int finBloque = 0;

    int machAct = -1;

    deque<Operacion *> ruta;
    Operacion * opAct = opRC;

    ruta.push_front(opAct);

    bool agregaRuta = true;

    while (ruta.size() > 0) {
      opAct = ruta.front();
      ruta.pop_front();

      agregaRuta = true;
      if (opAct != NULL && opAct->getJob() != -1) {

        agregaRuta = false;

        rutaC.push_front(opAct);
        if (actualiza) {
          opAct->setRC(true);
          opsCriticas.insert(opAct->getId());
        }

        /* CALCULAMOS LOS BLOQUES EN LA RC.... */

        // El bloque continua
        if (opAct->getMachine() == machAct ) {
          iniBloque = opAct->getIndMach();
          if (actualiza && iniBloque == 0 && (finBloque - iniBloque) >= 1) {
            bloques[machAct].insert(make_pair(iniBloque, finBloque));
            //cout << "\tRC, nuevo bloque: [" << iniBloque << "," << finBloque << "]\n"; 
            iniBloque = finBloque = 0; 
            machAct = -1;
          }
        } else {
          // Agregamos el bloque
          if (actualiza && (finBloque -  iniBloque) >= 1 ) {
            bloques[machAct].insert(make_pair(iniBloque, finBloque));
            //cout << "\tRC, nuevo bloque: [" << iniBloque << "," << finBloque << "]\n";
          }
          machAct = opAct->getMachine();
          iniBloque = finBloque = opAct->getIndMach();
          // Tenemos el fin de un nuevo bloque:
        }

        /* agregamos a los predecesores en la RC */
        if (opAct->getJobAnterior() != NULL && 
          ((opAct->getJobAnterior()->getTInicio() + opAct->getJobAnterior()->getDuration())
            == opAct->getTInicio()) ) {
          // cout << "\tAgregando PJ[" << opAct->getId() <<  "]=" << opAct->getJobAnterior()->getId() << "\n";
          ruta.push_front(opAct->getJobAnterior());
        }

        if (opAct->getMachAnterior() != NULL && 
          (opAct->getMachAnterior()->getTInicio() + opAct->getMachAnterior()->getDuration())
            == opAct->getTInicio() ) {
          ruta.push_front(opAct->getMachAnterior());
          // cout << "\tAgregando PM[" << opAct->getId() <<  "]=" << opAct->getMachAnterior()->getId() << "\n";
        }

       /* if (opAct->getTInicio() == 0) {
          agregaRuta = true;
        }  */

      } 
      if (agregaRuta) {
        if (actualizaRutas)
          rutas.push_back(rutaC);
        // cout << "Inicio de RC encontrado en la OP: " << opAct->getId() << "\n";
        // cout << "Tam RC: " << rutaC.size() << ", opFrente: " << rutaC.front()->getId() << "\n";
       if (ruta.size() > 0) {
          // cout << "Tam rutaAct: " <<  ruta.size() << ", opFrente: " << ruta.front()->getId() << "\n";
          opAct = rutaC.front();
          if (opAct != NULL && opAct->getJob() != -1) {
            while ( rutaC.size() > 0 && 
              (  opAct->getMachAnterior() != ruta.front()
                && opAct->getJobAnterior() != ruta.front() )
              ) {
              //cout << "\t RC, descartando op " << opAct->getId() << "\n";
              rutaC.pop_front();
              opAct = rutaC.front();
            }
          }
        } else {
          rutaC.clear();
        }

        machAct = -1;
        iniBloque = finBloque = 0;
      } // Fin Agrega Ruta
    } // Fin While Ruta 
  }
} // FIN CALCULAR_RUTA_CRITICA

// Calcula ruta critica (y bloques) desde opRC hacia la opFinal (Id=N)
void Solucion::calcularRutaAlFinal(Operacion *opIni, bool actualizaBloq) {
  if (opIni == NULL) return;

  int iniBloque = 0;
  int finBloque = 0;

  int machAct = -1;
  int tFin_opAct = 0;

  deque<Operacion *> pilaOps;

  Operacion * opAct = opIni;

  pilaOps.push_front(opAct);

  while (pilaOps.size() > 0) {
    opAct = pilaOps.front();
    pilaOps.pop_front();

    if (opAct == NULL || opAct->getJob() == -1) continue;

    // El bloque continua
    if (opAct->getMachine() == machAct ) {
      finBloque = opAct->getIndMach();
      if (finBloque == (numMaquinas-1) && (finBloque - iniBloque) >= 1) {
        if (actualizaBloq)
          bloques[machAct].insert(make_pair(iniBloque, finBloque));
        //cout << "\tRC, nuevo bloque: [" << iniBloque << "," << finBloque << "]\n"; 
        iniBloque = finBloque = 0; 
        machAct = -1;
      }
    } else {
      // Agregamos el bloque
      if ((finBloque -  iniBloque) >= 1 ) {
        if (actualizaBloq)
          bloques[machAct].insert(make_pair(iniBloque, finBloque));
            //cout << "\tRC, nuevo bloque: [" << iniBloque << "," << finBloque << "]\n";
      }
      machAct = opAct->getMachine();
      // Tenemos el Inicio de un nuevo bloque:
      iniBloque = finBloque = opAct->getIndMach();
    }

    tFin_opAct = opAct->getTInicio() + opAct->getDuration();

    /* agregamos a los sucesores en la RC */
    if (opAct->getJobSiguiente() != NULL && opAct->getJobSiguiente()->getIsRC() &&
        tFin_opAct == opAct->getJobSiguiente()->getTInicio() ) {
        pilaOps.push_front(opAct->getJobSiguiente());
    }

    if (opAct->getMachSiguiente() != NULL && opAct->getMachSiguiente()->getIsRC() &&
      tFin_opAct == opAct->getMachSiguiente()->getTInicio() ) {
        pilaOps.push_front(opAct->getMachSiguiente());
    }

  } // FIN WHILE pilaOps

}

// Calcula ruta critica (y bloques) desde opRC hacia la opInicial (Id=0)
void Solucion::calcularRutaAlInicio(Operacion *opFin, bool actualizaBloq) {
  if (opFin == NULL) return;

  int iniBloque = 0;
  int finBloque = 0;
  int machAct = -1;

  Operacion * opAct = opFin;

  deque<Operacion *> pilaOps;
  pilaOps.push_front(opAct);

  while (pilaOps.size() > 0) {
    opAct = pilaOps.front();
    pilaOps.pop_front();

    if (opAct == NULL || opAct->getJob() == -1) continue;

    // El bloque continua
    if (opAct->getMachine() == machAct ) {
      iniBloque = opAct->getIndMach();
      if (iniBloque == 0 && (finBloque - iniBloque) >= 1) {
        if (actualizaBloq)
          bloques[machAct].insert(make_pair(iniBloque, finBloque));
        //cout << "\tRC, nuevo bloque: [" << iniBloque << "," << finBloque << "]\n"; 
        iniBloque = finBloque = 0; 
        machAct = -1;
      }
    } else {
      // Agregamos el bloque
      if ((finBloque -  iniBloque) >= 1 ) {
        if (actualizaBloq)
          bloques[machAct].insert(make_pair(iniBloque, finBloque));
            //cout << "\tRC, nuevo bloque: [" << iniBloque << "," << finBloque << "]\n";
      }
      machAct = opAct->getMachine();
      // Tenemos el Inicio de un nuevo bloque:
      iniBloque = finBloque = opAct->getIndMach();
    }


    /* agregamos a los predecesores en la RC */
    if (opAct->getJobAnterior() != NULL && opAct->getJobAnterior()->getIsRC() &&
      ((opAct->getJobAnterior()->getTInicio() + opAct->getJobAnterior()->getDuration())
            == opAct->getTInicio()) ) {
      pilaOps.push_front(opAct->getJobAnterior());
    }

    if (opAct->getMachAnterior() != NULL && opAct->getMachAnterior()->getIsRC() &&
      (opAct->getMachAnterior()->getTInicio() + opAct->getMachAnterior()->getDuration())
            == opAct->getTInicio() ) {
      pilaOps.push_front(opAct->getMachAnterior());
    }
  } // FIN WHILE pilaOps

}

void Solucion::intercambiarOperaciones(Operacion *opI, Operacion *opJ) {
  if (opI == NULL || opI->getMachine() == -1 ) return;
  if (opJ == NULL || opJ->getMachine() == -1 ) return;

  Operacion *opAntI = opI->getMachAnterior();
  Operacion *opSigJ = opJ->getMachSiguiente();

  if (opAntI)
    opAntI->setMachSiguiente(opJ);    
  
  opI->setMachSiguiente(opSigJ);
  opJ->setMachSiguiente(opI);

  if (opSigJ)
    opSigJ->setMachAnterior(opI);
  
  opI->setMachAnterior(opJ);
  opJ->setMachAnterior(opAntI);

  int indiceMach = opI->getIndMach();
  
  this->perms[opI->getMachine()][indiceMach] = opJ;
  this->perms[opI->getMachine()][indiceMach+1] = opI;

  this->permSol[opI->getMachine()][indiceMach] = opJ->getId();
  this->permSol[opI->getMachine()][indiceMach+1] = opI->getId();

  opI->setIndMach(indiceMach+1);
  opJ->setIndMach(indiceMach);

}

int Solucion::reparar() {
  // indice de la pos en la seq del trabajo
  int indPosJobs[g->getNumTrabajos()] = {0};

  // indice de la pos en la seq de la maq
  int indPosMach[g->getNumMaquinas()] = {0};

  // Indices de los trabajos, para determinar el
  // trabajo que se va a reparar
  vector<int> numJobs(g->getNumTrabajos());

  vector<int> numMachs(g->getNumMaquinas());

  // Secuencia de ops en los trabajos
  vector<Operacion *> * jobs = g->getJobs();

  // Secuencia de ops en las maquinas
  vector<Operacion *> * machs = this->perms;

  int pos = 0;
  int indMach = 0;
  int id = 0;
  int offset=0;
  int tmp;

  int totalOperaciones = g->getNumTrabajos()*g->getNumMaquinas();
  int totalVerificadas = 0;

  int totalReparaciones = 0;

  while (totalVerificadas < totalOperaciones) {
    pos = 0;
    // Verificamos la secuencia para cada trabajo
    while(pos < g->getNumTrabajos()) {

      // si ya se han procesado todas las ops de la seq del trabajo 
      if (indPosJobs[pos] >= this->numMaquinas) {
        pos++;
        continue;
      }

      // ind de la maq para la sig op en la seq del trabajo
      indMach = jobs[pos][indPosJobs[pos]]->getMachine();

      //  id de la sig op en la seq del trabajo
      id = jobs[pos][indPosJobs[pos]]->getId();

      // Si la sig Op del Trabajo coincide con la sig op de la maq
      if (  machs[indMach][indPosMach[indMach]]->getId() == id ) {
        indPosMach[indMach]++;
        indPosJobs[pos]++;
        pos = -1;
        totalVerificadas++;
      }
      pos++;

    }

    // Si hay alguna operacion bloqueda:
    if (totalVerificadas < totalOperaciones ) {
      //cout << "Aplicando Repacion " << totalReparaciones << "...\n";
      if (Solucion::tipoReparacion == 1) {
      /* ************* OPCION 1 de REPERACION ************ */
      /* Reperacion considerando la secuencia de trabajo */ 
       numJobs.clear();
      // elegimos un trabajo al azar
      for(int i =0; i < g->getNumTrabajos(); i++) {
        // Verificamos si aun hay operaciones pendientes 
        // para el trabajo
        if (indPosJobs[i] < this->numMaquinas)
          numJobs.push_back(i);
      }
      random_shuffle(numJobs.begin(), numJobs.end());

      // Busca alguna operacion que no tiene 
      // predecesores pendientes en el job
      pos = 0;

      pos = numJobs[pos];
      indMach = jobs[pos][indPosJobs[pos]]->getMachine();
      //id = jobs[pos][indPosJobs[pos]]->getId();

      //intercambiar esta operacion
      movIzquierda(machs[indMach][indPosMach[indMach]], jobs[pos][indPosJobs[pos]]);
      } else {

      /* ************* OPCION 2 de REPERACION ************ */
      /* Reperacion considerando la secuencia de la Maquina */ 
      numMachs.clear();
      // Buscamos las maquinas que aun tienen operaciones pendientes por procesar
      for(int i=0; i < g->getNumMaquinas(); i++) {
        if (indPosMach[i] < machs[i].size())
          numMachs.push_back(i);
      }
      random_shuffle(numMachs.begin(), numMachs.end());

      offset = 0;
      bool cambio = false;

      while(!cambio && offset < g->getNumTrabajos()) {
        offset++;
        for (int i = 0; i < numMachs.size(); i++) {
          indMach = numMachs[i]; // Se elige una maquina

          // Si ya no hay mas operaciones para planificar, saltamos la iteracion
          // PENDIENTE: verificar si esta condicion es necesario,
          // en principio, nunca se deberia cumplir...
          if (indPosMach[indMach] + offset >= machs[indMach].size() ) continue;

          pos = machs[indMach][indPosMach[indMach] + offset]->getJob();

          // Si la operacion ya coincide con el flujo de la maquina:
          if (machs[indMach][ indPosMach[indMach] + offset ]
              == jobs[pos][indPosJobs[pos]]) {
            // Movemos la operaciones para que pueda ser planificada antes

            //intercambiar esta operacion
            movIzquierda(machs[indMach][indPosMach[indMach]], machs[indMach][indPosMach[indMach] + offset]);
            cambio = true;

            break;
          }
        }
      }

      } // Fin Repacion Opcion 2
      
      totalReparaciones++;
    }
  }

//  cout << "#\tTotal de Reparaciones : " << totalReparaciones << ", esFactible: " << esFactible() << "\n";

  return totalReparaciones;
}

bool Solucion::esFactible() {

  int totalOps =g->getNumTrabajos()*this->numMaquinas + 2; 
  int N[totalOps] = {0};
  N[0] = 1;
  N[totalOps-1] = 1;

  vector<Operacion *> OJ(g->getOpsIni());
  vector<Operacion *> OM(this->numMaquinas);

  for(int i =0; i < this->numMaquinas; i++) {
    OM[i] =this->perms[i][0];
  }

  vector<Operacion *> diff;
  sort(OJ.begin(), OJ.end());
  sort(OM.begin(), OM.end());

  vector<Operacion *> K;
  set_intersection(OJ.begin(), OJ.end(),
    OM.begin(), OM.end(), inserter(K, K.begin())
    //, ComparaIdOperacion()
    );

  //int it=0;

  while (K.size() > 0) {
    diff.clear();
    set_difference(OJ.begin(), OJ.end(),
      K.begin(), K.end(), inserter(diff, diff.begin())
      //, ComparaIdOperacion() 
    );
    OJ = diff;

    diff.clear();
    set_difference(OM.begin(), OM.end(),
      K.begin(), K.end(),  inserter(diff, diff.begin())
      //, ComparaIdOperacion() 
      );
    OM = diff; 

    for(int i=0; i < K.size(); i++) {      
      N[ K[i]->getId() ] = 1;
      
      if (K[i]->getJobSiguiente() != NULL
//        && N[ K[i]->getJobSiguiente()->getId() ] == 0 
        ) {
        /* if (N[ K[i]->getJobSiguiente()->getId() ] != 0) {
          cout <<"It = " << it << "\n";
          cout << "ERROR (?) EN Verificacion de Factibilidad! JS \n";
          //cout << "Operacion Actual: " << K[i]->toString() << "\n";
          //cout << "# Sol Actual: \n" << this->getPrintPerms() << "\n";
          //exit(1);
        } */
        OJ.push_back(K[i]->getJobSiguiente());
      }

      if (K[i]->getMachSiguiente() != NULL 
        //&& N[ K[i]->getMachSiguiente()->getId() ] == 0 
        ) {
        /* if (N[ K[i]->getMachSiguiente()->getId() ] == 0) {
          cout <<"It = " << it << "\n";
          cout << "ERROR (?) EN Verificacion de Factibilidad! MS \n";
          //cout << "Operacion Actual: " << K[i]->toString() << "\n";
          //cout << "# Sol Actual: \n" << this->getPrintPerms() << "\n";
          //exit(1);
        } */
        OM.push_back(K[i]->getMachSiguiente());
      }
    }

    diff.clear();
    sort(OJ.begin(), OJ.end());
    sort(OM.begin(), OM.end());
    set_intersection(OJ.begin(), OJ.end(),
      OM.begin(), OM.end(), back_inserter(diff)
    );
    K = diff;
    /* it++;
    if (it > 1000) {
      cout << "ERROR!!! Algoritmo ciclado...\n";
      exit(0);
    } */
  }

  numNoFactible += 1 - ( (OJ.size() + OM.size()) == 0);
  numFactibles += ( (OJ.size() + OM.size()) == 0 );

/*  if ((OJ.size() + OM.size()) != 0) {
    cout << "No es Factible!!!! \n";
    exit(1);
  } */

  return (OJ.size() + OM.size()) == 0;
} // FIN ES_FACTIBLE

string Solucion::getPrintPerms() {
  string res = "";

  for(int i=0; i < this->numMaquinas; i++) {
    //res = res + "M" + to_string(i) + "\t";
    for(int j=0; j < this->g->getNumTrabajos(); j++) {
      res = res + " " + to_string(this->perms[i][j]->getJob());

      // Marcar operaciones en rutas criticas
      //res = res + 
      //  ( this->perms[i][j]->getIsRC() ? "*" : "" );

      // Imprimir tiempos de rutas***
      /* res = res + "(" + to_string(this->perms[i][j]->getTInicio()) 
        + "," + to_string(this->perms[i][j]->getDuration()) 
        + "," + to_string(this->perms[i][j]->getRT()) + ")"; */
    }
    res = res + "\n";
  }

  res = res + "\n";

  return res;
}

string Solucion::toString() {
  string res = "";
/*  res = "Operaciones por Maquina: \n";
  for(int i =0; i < this->numMaquinas; i++) {
    res = res + "Maquina " + to_string(i+1) + ": \n";
    for(int j=0; j < this->g->getNumTrabajos(); j++) {
      res = res + "\t(J " + to_string(this->perms[i][j]->getJob()+1) 
        + ", id " + to_string(this->perms[i][j]->getId()) + " [idPerSol: " + to_string(permSol[i][j]) + " ]"
        + ", orden " + to_string(this->perms[i][j]->getId() - this->perms[i][j]->getJob()*this->numMaquinas ) + ")\n";
    }
    res = res + "\n";
  } */

  res = res + "Total de rutas: " + to_string(rutas.size()) + "\n";

  for(int i=0; i < rutas.size(); i++) {
    rutaC = rutas[i];
    res = res + "Ruta critica (" + to_string(rutaC.size()) + ") : \n";
    for (deque<Operacion *>::iterator it = rutaC.begin(); it!=rutaC.end(); ++it) {
      res = res + '\t' +  (*it)->toString() + "\n";
    }
    res = res + "\n";
  }

//  res = res + "MAKESPAN: " + to_string(makespan) + "\n";

  return res;
}

int Solucion::recalcularRT(Operacion *opIni, bool actualiza) {
  //numEvals++;
  if (opIni==NULL) return INT_MAX;
  if (opIni->getJob()==-1) return makespan;

  int res=0;

  deque<Operacion *> ops;
  Operacion *opAux = NULL;
  Operacion *op = NULL;

  // Etiquetar todos los nodos como visitados
  int etiquetas[(g->getNumTrabajos())*(g->getNumMaquinas()+2)] = {1};
  int rts[(g->getNumTrabajos())*(g->getNumMaquinas()+2)] = {-1};

  for (Operacion *op : g->getN()) {
    etiquetas[op->getId()] = 1;
    rts[op->getId()] = op->getRT();
  }

  // Calcular conjunto RT:
  // operaciones desde las cuales op es alcanzable.
  // Etiquetar a los nodos del conjunto RT, como
  // no etiquetados....
  ops.push_front(opIni);

  while (ops.size() > 0 ) {
    op = ops.front();
    ops.pop_front();
    if (etiquetas[op->getId()] == 0) continue;
    etiquetas[op->getId()] = 0;

    if (op->getMachAnterior() != NULL && op->getMachAnterior()->getJob() != -1
      && etiquetas[op->getMachAnterior()->getId()] == 1 ) {
      ops.push_back(op->getMachAnterior());
    }
    if (op->getJobAnterior() != NULL && op->getJobAnterior()->getJob() != -1
      && etiquetas[op->getJobAnterior()->getId()] == 1 ) {
      ops.push_back(op->getJobAnterior());
    }
  }

  int tJob = 0;
  int tMach = 0;

  ops.clear();
  ops.push_front(opIni);

  while(ops.size() > 0 ) {
    // Seleccionamos un elemento de ops
    op = ops.front();

    // Lo etiquetamos 
    etiquetas[op->getId()] = 1;

    // y lo eliminamos
    ops.pop_front();

    // Recalcular RT = ruta mas larga... desde esta op.
    tJob = op->getJobSiguiente() == NULL || op->getJobSiguiente()->getJob() == -1 ? 
      0 : rts[op->getJobSiguiente()->getId()];

    tMach = op->getMachSiguiente() == NULL || op->getMachSiguiente()->getJob() == -1 ? 
      0 : rts[op->getMachSiguiente()->getId()] ; 

    rts[op->getId()] = max( tMach, tJob ) + op->getDuration();

    if (actualiza) {
      op->setRT( rts[op->getId()] );
    }

    if (rts[op->getId()] > res) {
      res = rts[op->getId()];
    }

    // sucesor maquina del predecesor en el trabajo de la operacion actual 
    // SM[ PJ[op] ]
    opAux = op->getJobAnterior();
    if (opAux != NULL  && !etiquetas[opAux->getId()]) {
      opAux = opAux->getMachSiguiente();

      if ( opAux == NULL || opAux->getJob() == -1 || etiquetas[opAux->getId()] ) {
        ops.push_back(op->getJobAnterior());
      }
    }

    // sucesor en el trabajo del predecesor en la maquina actual
    // SJ[ PM[op] ]
    opAux = op->getMachAnterior();
    if (opAux != NULL && !etiquetas[opAux->getId()]) {
      opAux = opAux->getJobSiguiente();

      if ( opAux == NULL || opAux->getJob() == -1 || etiquetas[opAux->getId()] )  {
        ops.push_back(op->getMachAnterior());
      }
    }

  }

  /* Calculamos el maximo RT, considerando todas las operaciones iniciales */
  vector<Operacion *> & opsIni = g->getOpsIni();
  for (Operacion *op : opsIni) {
    rts[op->getId()] = rts[op->getId()] == -1 ? op->getRT() : rts[op->getId()];
    res = res < rts[op->getId()] ? rts[op->getId()] : res;
  }  

  if (actualiza) {
    this->makespan = res;
    
    opsCriticas.clear();
    rutas.clear();
    for(int i=0; i < numMaquinas; i++) {
      bloques[i].clear();
    }

    // RECALCULAR RUTAS CRITICAS
    for (Operacion *op : g->getOpsFin()) { 
      if (op->getTInicio() + op->getDuration() == this->makespan) {
        rutaC.clear();
        this->calcularRutaCritica(op, this->makespan);
      }
    }
  }

//  cout << "MAX RT: " << res << "\n";
  return res;
}

int Solucion::recalcularST(Operacion *opIni, bool actualiza) {
  //numEvals++;
  if (opIni==NULL) return INT_MAX;
  if (opIni->getJob()==-1) return makespan;
  numEvals++;

  int res=0;

  deque<Operacion *> ops;
  Operacion *opAux = NULL;
  Operacion *op = NULL;

  // Etiquetar todos los nodos como visitados
  int etiquetas[(g->getNumTrabajos())*(g->getNumMaquinas()+2)];
  int sts[(g->getNumTrabajos())*(g->getNumMaquinas()+2)];

  for (Operacion *op : g->getN()) {
    etiquetas[op->getId()] = 1;
    sts[op->getId()] = op->getTInicio();
  } 

  // Calcular conjunto ST:
  // operaciones alcanzables desde la opIni.
  // Marcar a los nodos del conjunto ST, como
  // no etiquetados....
  ops.push_front(opIni);
  
  while (ops.size() > 0) {
    op = ops.front();
    ops.pop_front();
    if (op == NULL) continue;
    
    etiquetas[op->getId()] = 0;

    if (op->getMachSiguiente() != NULL && op->getMachSiguiente()->getJob() != -1
      && etiquetas[op->getMachSiguiente()->getId()] == 1
      ) {
        ops.push_back(op->getMachSiguiente() );
    }
    if (op->getJobSiguiente() != NULL && op->getJobSiguiente()->getJob() != -1
      && etiquetas[op->getJobSiguiente()->getId()] == 1
      ) {
        ops.push_back(op->getJobSiguiente());
    }
  }

  int tJob = 0;
  int tMach = 0;

  ops.clear();
  ops.push_front(opIni);

  while(ops.size() > 0 ) {
    // Seleccionamos un elemento de ops
    op = ops.front();
    // y lo eliminamos
    ops.pop_front();

    if (op == NULL) continue;

    // Lo etiquetamos 
     etiquetas[op->getId()] = 1;
    
    // Recalcular ST = ruta mas larga... hasta esta op.
    tJob = op->getJobAnterior() == NULL || op->getJobAnterior()->getJob() == -1 ? 0 :
      sts[op->getJobAnterior()->getId()] + op->getJobAnterior()->getDuration();

    tMach = op->getMachAnterior() == NULL || op->getMachAnterior()->getJob() == -1 ? 0 :
      sts[op->getMachAnterior()->getId()] + op->getMachAnterior()->getDuration(); 

    sts[op->getId()] = max( tMach, tJob );

    if (actualiza) {
      op->setTInicio( max( tMach, tJob ) );
    }

    if (sts[op->getId()] + op->getDuration() > res) {
      res = sts[op->getId()] + op->getDuration();
    }

    // Predecesor maquina del sucesor en el trabajo de la operacion actual 
    // PM[ SJ[op] ]
    opAux = op->getJobSiguiente();
    if (opAux != NULL  && !etiquetas[opAux->getId()] ) {  
      opAux = opAux->getMachAnterior();

      if (( opAux == NULL || opAux->getJob() == -1 || etiquetas[opAux->getId()] ) 
        && (op->getJobSiguiente() != NULL) ) {
        ops.push_back(op->getJobSiguiente());
      }
    }

    // Predecesor en el trabajo del sucesor en la maquina actual
    // PJ[ SM[op] ]
    opAux = op->getMachSiguiente();
    if (opAux != NULL && !etiquetas[opAux->getId()] 
      ) {  
      opAux = opAux->getJobAnterior();

      if ( (opAux == NULL || opAux->getId() == 0 || etiquetas[opAux->getId()]) 
        && (op->getMachSiguiente() != NULL) ) {
        ops.push_back(op->getMachSiguiente());
      }
    }

  }

  /* Calculamos el maximo ST, considerando todas las operaciones finales */
  vector<Operacion *> & opsFin = g->getOpsFin();

  for (Operacion *op : opsFin) {    

    res = res < (sts[op->getId()] + op->getDuration()) ? 
      sts[op->getId()] + op->getDuration() : res;
  }

  if (actualiza) {
    this->makespan = res;
  }

  return res;
}

int Solucion::validarPermutaciones() {

  int valida = 1;

  for(int i=0; i < numMaquinas; i++) {
    for (int j=0; j < g->getNumTrabajos(); j++) {
      valida = perms[i][j]->getId() == permSol[i][j];
      if (!valida) break;
    }
    if (!valida) break;
  }

  return valida;
}