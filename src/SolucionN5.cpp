
/* Vecindad N5
  Intercambiar dos operaciones i, j en la ruta critica.
  Tal que i es la primera operacion de un bloque
  o j es la ultima operacion de un bloque
 */
Movimiento Solucion::getBestN5(
  map<Movimiento, int > &listaTabu, 
  int itActual, int tamLT,
  int bestVal, bool bestAllN) {

  int bestMakespan = INT_MAX;
  int bestEvalRutas = INT_MAX;
  int bestNumBloqs = INT_MAX;

  Movimiento bestMov(0, NULL, NULL);

  Operacion *opAct = NULL;
  Operacion *opSig = NULL;

  int makespanAct = INT_MAX;
  int makespanIni = this->getMakespan();
  int lower = INT_MAX;

  map<Movimiento, int>::iterator iterLt;

  vector<Movimiento> movs = this->getMovimientosN5();
  std::random_shuffle( movs.begin(), movs.end());

  vector<Movimiento> movsBest;

//  cout << "\tN5 Num rutas: " << rutas.size() << "\n"; 
//  cout << "\tN5 Movs generados: " << movs.size() << ":\n";
  if (movs.size()==0) {
    cout << "EROR N5, no hay movimientos disp...\n";
    exit(0);
  }
  for(int i=0; i < movs.size(); i++) {
//    cout << i << ": " << movs[i].toString() << "\n";
    opAct = movs[i].op1;
    opSig = movs[i].op2;

    // Calculamos una cota inferior para el cambio:
    lower = this->calcularCotaInferiorSwap(opAct, opSig);

    if (lower < makespanAct ) {

      this->intercambiarOperaciones(opAct, opSig);
      // calcular Makespan, sin recalcular rutaCritica
      makespanAct = recalcularST(opSig, false);
//      makespanAct = recalcularRT(opAct, false);
      movs[i].eval = makespanAct;

      // verificamos si esta en la lista tabu:
      //iterLt = listaTabu.find(make_pair(opAct->getId(), opSig->getId()));
      iterLt = listaTabu.find(movs[i]);

      if (makespanAct == bestMakespan && makespanAct == bestVal) {
        movsBest.push_back(movs[i]);
      }

      // comparar nuevoMakespan con el bestMakespan
      if ( (makespanAct < bestMakespan // Se encontro a un vecino mejor (de los visitados)
 //         || (makespanAct == bestMakespan && bestEvalRutas < this->getNumRutas())
 //         || (makespanAct == bestMakespan && bestEvalRutas == this->getNumRutas()
 //             && bestNumBloqs < this->getNumBloques() ) 
        )
        && ( makespanAct < bestVal // El nuevo supera al best de la busqueda
          || iterLt == listaTabu.end() // El mov no esta en la lista tabu
          || itActual > (iterLt->second + tamLT) // el mov ya no es prohibido
          ) ) {
        bestMakespan = makespanAct; // best de la vecindad
        bestEvalRutas = this->getNumRutas();
        bestNumBloqs = this->getNumBloques();

        bestMov = movs[i];

        movsBest.clear();
        movsBest.push_back(movs[i]);
      }

      // regresar cambio
      this->intercambiarOperaciones(opSig, opAct);

      // si encontramos un mov que mejora el makeInicial, 
      // detenemos la busqueda (bestAllN = false), o 
      // si (betAllN = true) se explora toda la vecindad
      if (bestMakespan < makespanIni && !bestAllN) break; 
    }
  }

  if (movsBest.size() > 1) {
    bestMov = evaluarMejores(movsBest);
  }

//  cout << "N5, makespan Ini: " << makespanIni << "\n";
//  cout << "Mejor Encontrado : " << bestMakespan << "\n";

  //return make_pair(bestMakespan, bestN);
  return bestMov;
}

Movimiento Solucion::evaluarMejores(vector<Movimiento> movsBest) {

//  cout << "\tNum rutas: " << this->rutas.size() << "\n";
//    cout << "\tNum movs best: " << movsBest.size() << "\n";

    int indBest = 0;
    //int numRutasBest = INT_MAX;
    //int numBloquesBest = INT_MAX;
    int pesoOpsCriticasBest = INT_MAX;
    int pesoAct = 0;

    Operacion *opSigU = NULL;
    Operacion *opAntV = NULL;

    for(int i=0; i < movsBest.size(); i++ ) {

      // Realizar movimiento y calcular rutas criticas
      switch(movsBest[i].tipo) {
        case 0:
          this->intercambiarOperaciones(movsBest[i].op1, movsBest[i].op2);
          movsBest[i].eval = this->calcularMakespan(true);
          break;
/*        case 1:
          this->insertarDerecha(movsBest[i].op1, movsBest[i].op2, true, false);
          break;
        case 2:
          this->moverIzquierda(movsBest[i].op1, movsBest[i].op2, true, false);
          break;
        case 3:
          this->insertarIzquierda(movsBest[i].op1, movsBest[i].op2, true, false);
          break;
        case 4:
          this->moverDerecha(movsBest[i].op1, movsBest[i].op2, true, false);
          break;  */
     }

     pesoAct = this->getPesoOpsCriticas();

     if (pesoAct < pesoOpsCriticasBest) {
        indBest = i;
        pesoOpsCriticasBest = pesoAct;
      }

      if (movsBest[i].tipo == 0) {
        // regresa el cambio
        this->intercambiarOperaciones(movsBest[i].op2, movsBest[i].op1);
      }

      // Recalcular rutas criticas:
      this->calcularMakespan(true);

    }
//    exit(0);
  return movsBest[indBest];
}

vector<Movimiento> Solucion::getMovimientosN5() {

  int numJobs = g->getNumTrabajos();
  set<Movimiento> movs;

//  cout << "\tN5 getMovs, numBloques: " << bloques.size() << "\n";
  for(int i=0; i < bloques.size(); i++) {
    if (bloques[i].size() == 0) continue;

    for (set<pair<int, int>>::iterator it=bloques[i].begin(); it!=bloques[i].end(); ++it) {
      pair<int, int> bloq = *it;

/*      cout << "\tN5, Generando movs para el bloque: [" 
        << bloq.first << "," << bloq.second << "], en la maq: "
        << (i+1) << "\n"; */

      // validar indices
      if (bloq.first < 0 || bloq.first >= (numJobs-1)
        || bloq.second < 1 || bloq.second >= numJobs
        || bloq.first >= bloq.second ) {
//        cout << "\tN5, indices invalidos, saltando bloque ... \n";
        continue;
    }

      //validar que no sea la primera op de una RC:
      if ( this->perms[i][bloq.first]->getTInicio() !=0) {
        Movimiento mIniBloq(0, this->perms[i][bloq.first], this->perms[i][bloq.first+1]);
        mIniBloq.maquina = i;
        mIniBloq.indOp1 = bloq.first;
        mIniBloq.seq.push_back(mIniBloq.op1);
        mIniBloq.seq.push_back(mIniBloq.op2);
        movs.insert(mIniBloq); 
      } /* else {
        cout << "\tN5, op ini de bloque es ini de RC, saltando... \n";
      } */

      //validar que no sea la ultima op de una RC
      if ( (bloq.second - bloq.first) > 1 &&
        (this->perms[i][bloq.second]->getTInicio() 
        + this->perms[i][bloq.second]->getDuration()) != this->makespan ) {
        Movimiento mFinBloq(0, this->perms[i][bloq.second-1], this->perms[i][bloq.second]);
        mFinBloq.maquina = i;
        mFinBloq.indOp1 = bloq.second-1;
        mFinBloq.seq.push_back(mFinBloq.op1);
        mFinBloq.seq.push_back(mFinBloq.op2);
        movs.insert(mFinBloq);
      } /* else {
        cout << "\tN5, op fin de bloque es fin de RC, saltando ... \n";
      } */
    }
  }

  vector<Movimiento> res(movs.begin(), movs.end());

  return res;
}

int Solucion::calcularCotaInferiorSwap(Operacion *opAct, Operacion *opSig) {
  numEvalsCI++; // contador para estadisticas de TS
  int ra, qa, rb, qb, lower, rPMa, rPJb, rPJa, qSMb, qSJa, qSJb;
 
  // Calculamos una cota inferior para el cambio:
  rPMa = !opAct->getMachAnterior() || opAct->getMachAnterior()->getId() == -1 ? 0
    : opAct->getMachAnterior()->getTInicio() + opAct->getMachAnterior()->getDuration() ;

  rPJb = !opSig->getJobAnterior() || opSig->getJobAnterior()->getId() == -1 ? 0
    : opSig->getJobAnterior()->getTInicio() + opSig->getJobAnterior()->getDuration() ;

  rPJa = !opAct->getJobAnterior() || opAct->getJobAnterior()->getId() == -1 ? 0
    : opAct->getJobAnterior()->getTInicio() + opAct->getJobAnterior()->getDuration() ;

  qSMb = !opSig->getMachSiguiente() || opSig->getMachSiguiente()->getId() == -1 ? 0
    : opSig->getMachSiguiente()->getRT();

  qSJa = !opAct->getJobSiguiente() || opAct->getJobSiguiente()->getId() == -1 ? 0
    : opAct->getJobSiguiente()->getRT();

  qSJb = !opSig->getJobSiguiente() || opSig->getJobSiguiente()->getId() == -1 ? 0
    : opSig->getJobSiguiente()->getRT();

  rb = max(rPMa, rPJb);
  ra = max(rb + opSig->getDuration(), rPJa);
  qa = max(qSMb, qSJa ) + opAct->getDuration();
  qb = max( qa , qSJb ) +  opSig->getDuration();
  lower = max(rb+qb, ra+qa);

  return lower; 
}

/* */
int calcularCotaInferiorMovs( vector< Operacion * > seq ) {
  int lower = 0;

  Operacion *opA = seq[0];
  Operacion *opPJ = opA->getJobAnterior();
  Operacion *opSJ = opA->getJobSiguiente();

  Operacion *opB = seq[1];

  /* PENDIENTE: corregir referencias */
  Operacion *opPM = opA->getMachAnterior();
  Operacion *opSM = opA->getMachSiguiente();

  int ra = max( opPJ->getTInicio() + opPJ->getDuration(), 
    opPM->getTInicio() + opPM->getDuration() );

  int rb = 0;

  for(int i=1; i < seq.size(); i++) {
    opB = seq[i];

    opPJ = opB->getJobAnterior();

    rb = max( opPJ->getTInicio() + opPJ->getDuration() ,
      ra + opA->getDuration() );
    opA = opB;
  }
  opB = seq.back();

  /* PENDIENTE: corregir referencias */

  int tb = max( opB->getRT() , opSM->getRT() );

  int ta = 0;

  for(int i = ((int) seq.size())-2; i >= 0; i++ ) {
    opA = seq[i];
    opSJ = opA->getJobSiguiente();

    ta = max( opSJ->getRT(), opB->getRT() );
    opB = opA;
    lower = max(lower, ta + opA->getTInicio());
  }

  return lower;
}