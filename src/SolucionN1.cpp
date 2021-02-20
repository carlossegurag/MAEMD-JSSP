/* Vecindad N1
  Intercambiar dos operaciones i, j en la ruta critica.
  Tal que (i, j) son operaciones consecutivas en una maquina
 */
Movimiento Solucion::getBestN1( deque<Movimiento > &listaTabu, 
  int itActual, int tamLT,
  int bestVal, bool bestAllN) {

 return getBestN(listaTabu, itActual, tamLT, bestVal, bestVal, bestAllN, 1);
}

Movimiento Solucion::getBestN(
  deque<Movimiento> &listaTabu, 
  int itActual, int tamLT,
  int bestVal, int currentVal,
  bool bestAllN, int tipoN) {

  Movimiento bestMov(0, NULL, NULL);
  Movimiento factMov(0, NULL, NULL);

  int lower = 0;

  map<Movimiento, int>::iterator iterLt;

  int makespanIni = this->getMakespan();

  int makespanMov = INT_MAX;
  Operacion *opU = NULL;
  Operacion *opV = NULL;

  bool checkFact = true;
  vector<Movimiento> movs;

  switch(tipoN) {
    case 1:
      movs = getMovimientosN1();
      checkFact = false;
      break;
    case 4:
      movs = getMovimientosN4();
      break;
    case 5:
      movs = getMovimientosN5();
      break;
    case 6:
      movs = getMovimientosN6();
      break;
    case 7:
    default:
      movs = getMovimientosN7( false ); //rand() % 1000 == 0 );
      break;
  }

//  cout << "# getBestN, IterAct: " << itActual << ", num movs: " << movs.size() << "\n";

  std::random_shuffle( movs.begin(), movs.end());

  vector<Movimiento> movsBest;
  numMovs=movs.size();
  numEvalsCI=0;
  numEvals=0;

  bool esTabu = false;
  bool calcularMks = true;
  bool cambiar = false;

  bool bestFact = false;

  for(int i=0; i < movs.size(); i++) {
    opU = movs[i].op1; // opU
    opV = movs[i].op2; // opV

    makespanMov = INT_MAX;
    esTabu = true; // sirve para hacer el check en la evaluacion

/*    if (opU->getMachSiguiente() == opV)
      movs[i].tipo = 0;  */

    // Evaluamos el movimiento
    switch(movs[i].tipo) {
      case 0: 
      {
        // Calculamos una cota inferior para el cambio:
        lower = this->calcularCotaInferiorSwap(opU, opV);
        makespanMov = lower;
        if (lower < bestMov.eval 
          && (Solucion::evalExacta || makespanMov < bestMov.eval) ) {
          this->intercambiarOperaciones(opU, opV);
          // calcular Makespan, sin recalcular rutaCritica
          if (Solucion::evalExacta)
            makespanMov = recalcularST(opV, false);
          //makespanMov = recalcularRT(opU, false);
          if (makespanMov < bestMov.eval)
            esTabu = this->esTabu(listaTabu, movs[i], itActual);
          this->intercambiarOperaciones(opV, opU);
        }
        break;
      }
      case 4:
      case 1: { // Forward
        makespanMov = evalMovDerecha(movs[i], calcularMks, cambiar, 
          checkFact, esTabu, listaTabu, itActual, bestMov.eval);
        break;
      } 
      case 3:
      case 2: { // Backward
        makespanMov = evalMovIzquierda(movs[i], calcularMks, cambiar, 
          checkFact, esTabu, listaTabu, itActual, bestMov.eval);
        break;
      } 
    }
    movs[i].eval = makespanMov;

    if (factMov.eval == INT_MAX && makespanMov != INT_MAX )
        factMov = movs[i];

    // cout << "#\t\tMovAct, esTabu: " << esTabu << ", " << movs[i].toString() << "\n";

    // El mov se acepta si:
    if (
        (makespanMov < bestMov.eval // es el mejor de la vecindad
          && !esTabu)  // No esta prohibido

        || (makespanMov == bestMov.eval // es el mejor de la vecindad
          && !esTabu && !bestFact ) // es igual al de la vecindad

        || (makespanMov < bestVal // supera al best de la busqueda
          && makespanMov < bestMov.eval ) // mejor de la vecindad
      ) {

        movsBest.clear( );

        if (makespanMov < bestVal && !esTabu) bestFact = true;

        bestMov = movs[i];
        
        // Si solo queremos el primero que mejore
        if (makespanMov < currentVal && !bestAllN) break;
      }

      if (bestMov.eval == makespanMov) {
        movsBest.push_back( bestMov );
      }
    // cout << "#\tbestMov: " << bestMov.toString() << "\n";
  }

  /* Si todos los mov esta prohibidos, elegimos el primero factible que se encontro (que fue aleatorio) */
  if ( bestMov.eval == INT_MAX && factMov.eval != INT_MAX) {
    movsBest.clear();
    bestMov = factMov;
    movsBest.push_back(bestMov);
  }

/*  if (movsBest.size() > 1) {
    int k_min = 0;
    for(int iB=1; iB < movsBest.size(); iB++) {
//      cout << "iB: " << iB << "\n";
//      cout << "OP1: " << movsBest[iB].op1->getId() << ", ";
//      cout  << "OP2: " << movsBest[iB].op2->getId() << ". X.size()" << x.size() << "\n" ;
      if ( (this->x[ movsBest[iB].op1->getId() ]
      +  this->x[ movsBest[iB].op2->getId() ]) < 
        ( this->x[ movsBest[k_min].op1->getId() ]
      +  this->x[ movsBest[k_min].op2->getId() ] ) )
        k_min = iB;
    }
    if (k_min != 0) {
      cout << "Num Best Movs: " << movsBest.size() << "\n";
    }
    bestMov = movsBest[k_min];
  } */

//  cout << "#\tbestMov: " << bestMov.toString() << "\n";

  return bestMov;
} // FIN GET_BEST_N

vector<Movimiento> Solucion::getMovimientosN1() {

  //cout <<  "#\t getBestN1, num rutas: " << this->rutas.size() << "\n";

  int numJobs = g->getNumTrabajos();
  set<Movimiento> movs;

  // Recorremos todas las rutas
  for(int i=0; i < rutas.size(); i++) {
    rutaC = rutas[i];

    // Para cada ruta calculamos los movimientos
    for(int j=1; j < rutaC.size(); j++) {
      if (rutaC[j-1]->getMachine() != rutaC[j]->getMachine()) continue;
      Movimiento mSwap(0, rutaC[j-1], rutaC[j]);
      mSwap.maquina = rutaC[j]->getMachine();
      mSwap.indOp1 = rutaC[j-1]->getIndMach();
      mSwap.seq.push_back(rutaC[j-1]);
      mSwap.seq.push_back(rutaC[j]);
      movs.insert(mSwap);
    }
  }

  vector<Movimiento> res(movs.begin(), movs.end());

  return res;
}

bool Solucion::esTabu( deque<Movimiento> & listaTabu, Movimiento & mov, 
  int itActual ) {

  bool tabu = false;

  for(int i=0; i < listaTabu.size() && !tabu; i++ ) {

    // Mov ya no esta prohibudo
    if (listaTabu[i].numIter < itActual ) continue;

    // el mov se da en maquinas diferentes
    if (listaTabu[i].maquina != mov.maquina) continue;
   
    // la sec prohibida esta antes de la sec del mov actual.
   // if (listaTabu[i].indOp1 < mov.indOp1 ) continue;

    // la sec prohibida esta despues de la sec del mov actual.
   // if (listaTabu[i].indOp1 >= ( mov.indOp1 + mov.seq.size() ) ) continue;

    tabu = true;
    // Verificamos toda la secuencia:
    for(int j=listaTabu[i].indOp1, k=0; k < listaTabu[i].seq.size() && tabu; j++, k++ ) {
      tabu = listaTabu[i].seq[k]->getId() == this->perms[mov.maquina][j]->getId();
    }

  }  
  return tabu;
}

void Solucion::movIzquierda(Operacion * opU, Operacion * opV) {
  if (opU == NULL || opV == NULL) return;

  Operacion *opUAnt = opU->getMachAnterior();
  Operacion *opVAnt = opV->getMachAnterior();
  Operacion *opVSig = opV->getMachSiguiente();

  int numMach = opV->getMachine();
  int posU = opU->getIndMach();
  int posV = opV->getIndMach();

  if (opVAnt != NULL)
    opVAnt->setMachSiguiente(opVSig);

  if (opVSig != NULL)
    opVSig->setMachAnterior(opVAnt);

  opV->setMachSiguiente(opU);
  opV->setMachAnterior(opUAnt);

  if (opUAnt != NULL)
    opUAnt->setMachSiguiente(opV);

  opU->setMachAnterior(opV);

  for(int i=posV; i > posU; i--) {
    this->perms[numMach][i] = this->perms[numMach][i-1]; 
    this->perms[numMach][i]->setIndMach(i);

    this->permSol[numMach][i] = this->perms[numMach][i]->getId();
  }
  this->perms[numMach][posU] = opV;
  opV->setIndMach(posU);
  this->permSol[numMach][posU] = opV->getId();
}

void Solucion::movDerecha(Operacion * opU, Operacion * opV) {
  if (opU == NULL || opV == NULL) return;

  Operacion *opVSig = opV->getMachSiguiente();
  Operacion *opUSig = opU->getMachSiguiente();
  Operacion *opUAnt = opU->getMachAnterior();

  int numMach = opV->getMachine();
  int posV = opV->getIndMach();
  int posU = opU->getIndMach();

  /* Realizar movimiento */
  opV->setMachSiguiente(opU);

  if (opVSig != NULL)
    opVSig->setMachAnterior(opU);

  opU->setMachSiguiente(opVSig);
  opU->setMachAnterior(opV);

  if (opUSig != NULL) 
    opUSig->setMachAnterior(opUAnt);

  if (opUAnt != NULL)
    opUAnt->setMachSiguiente(opUSig);

  for(int j=posU; j < posV; j++) {
    this->perms[numMach][j] = this->perms[numMach][j+1];
    this->perms[numMach][j]->setIndMach(j);
    this->permSol[numMach][j] = this->perms[numMach][j]->getId();
  }
  this->perms[numMach][posV] = opU;
  this->perms[numMach][posV]->setIndMach(posV);
  this->permSol[numMach][posV] = this->perms[numMach][posV]->getId();
}

// Tipo 1 y 4 (Forward)
int Solucion::evalMovDerecha(Movimiento &mov,
  bool calcularMkspn, bool cambiar, bool checkFact,
  bool & checkTabu, deque<Movimiento> &listaTabu, int numIter,
  int evalBestN) {

  //cout << "********************EvalDer..\n";

  Operacion *opU = mov.op1;
  Operacion *opV = mov.op2;

  if (opU == NULL || opV == NULL) return INT_MAX;

  Operacion *opUSig = opU->getMachSiguiente();

  /* Realizar mov */  
  movDerecha(opU, opV);

  int nMakespan = INT_MAX;

  /* Calcular makespan */
  if (calcularMkspn) {
    // checar factibilidad
    bool fact = true;
    if (checkFact) {

      // L(v, n) >= L (JS[u], n)
      fact = mov.tipo == 0 || 
      ( opV->getRT() >=
      ( opU->getJobSiguiente() != NULL ? opU->getJobSiguiente()->getRT() : 0 ) );       

      if (!fact && !Solucion::soloFact)
        fact = esFactible();
    }

    //cout << "********************EvalDer, es fact: " << fact << " ..\n";

    nMakespan = INT_MAX;
    if (Solucion::calcularCotas)
      nMakespan = !fact ? INT_MAX : calcularCotaInferiorSeq(mov);
   

    //cout << "********************EvalDer, cota: " << nMakespan << " ..\n";

    /*
    int cota = nMakespan;
    if (fact) {
      nMakespan = recalcularST(opUSig , false);
      cout << "#Cota y valor: " << cota << " " << nMakespan << endl;
      if (nMakespan < cota){
        cout << "##Error interno evalMovDerecha" << endl;
        cout << nMakespan << " " << cota << endl;
//        cout << "True Makespan: " << this->calcularMakespan(true) << endl;
        cout << "checkFact: " << checkFact << endl;

        cout << "ultimo mov: \n\t" << mov.toString() << endl;
        cout << "Sol Act: \n" << this->getPrintPerms() << endl;

        movIzquierda(opUSig, opU);

        cout << "Sol Ant: \n" << this->getPrintPerms() << endl;
        exit(-1);
      }
    } */  

    if (fact && Solucion::evalExacta  
        &&  (!Solucion::calcularCotas || nMakespan < evalBestN ) )
      nMakespan = recalcularST(opUSig, cambiar);

/*    if (Solucion::evalExacta && fact && cambiar)
      nMakespan = recalcularRT(opU, cambiar);  */ 
  } 

  if (checkTabu && nMakespan < evalBestN)
    checkTabu = esTabu(listaTabu, mov, numIter);

  /* Regresar el cambio */
  if (!cambiar) {
    movIzquierda(opUSig, opU);
  }

  return nMakespan;
} 

// Backward
int Solucion::evalMovIzquierda(Movimiento & mov,
  bool calcularMkspn, bool cambiar, bool checkFact,
  bool & checkTabu, deque<Movimiento> &listaTabu, int numIter,
  int evalBestN ) {

  Operacion *opU = mov.op1;
  Operacion *opV = mov.op2;

  if (opU == NULL || opV == NULL) return INT_MAX;

  Operacion *opVAnt = opV->getMachAnterior();

  /* Realizar mov */  
  movIzquierda(opU, opV);

  int nMakespan = INT_MAX;

  /* Calcular makespan */
  if (calcularMkspn) {
    // checar factibilidad
    bool fact = true;
    if (checkFact) {

      // L(0, u) + p_u >= L(0, PJ[v]) + p_PJ[v]
      fact = mov.tipo == 0 || 
      (opU->getTInicio() + opU->getDuration()) >=
      ( opV->getJobAnterior() != NULL ? 
          opV->getJobAnterior()->getTInicio() + opV->getJobAnterior()->getDuration() 
          : 0 ); 

      if (!fact && !Solucion::soloFact)
        fact = esFactible();
    }

    nMakespan = INT_MAX;
    if (Solucion::calcularCotas)
      nMakespan = !fact ? INT_MAX : calcularCotaInferiorSeq(mov);

    if (fact && Solucion::evalExacta 
        &&  (!Solucion::calcularCotas || nMakespan < evalBestN ) ) {
      nMakespan = recalcularST(opV, cambiar);
    }
  }
 
  if (checkTabu && nMakespan < evalBestN)
    checkTabu = esTabu(listaTabu, mov, numIter);

  /* Regresar el cambio */
  if (!cambiar) {
    movDerecha(opV, opVAnt);
  }

  return nMakespan;
} 

int Solucion::calcularCotaInferiorSeq(Movimiento &mov) {
  numEvalsCI++;
//  if (mov.tipo == 0) return this->calcularCotaInferiorSwap(mov.op1, mov.op2);

  int lower = 0;

  Operacion *opTmp = NULL;
  if (mov.tipo == 1 || mov.tipo == 4) { // Forward
    opTmp = mov.seq.front();
    mov.seq.pop_front();
    mov.seq.push_back(opTmp);
  } else { // Backward
    opTmp = mov.seq.back();
    mov.seq.pop_back();
    mov.seq.push_front(opTmp);
  }

  Operacion *opA = mov.seq[0];
  Operacion *opPJ_A = opA->getJobAnterior();
  Operacion *opPM_A = opA->getMachAnterior();
  Operacion *opSJ_A = NULL;

  int tamSeq = mov.seq.size();

  vector<int> ra(tamSeq); // tiempos de inicio
  vector<int> ta(tamSeq); // ruta mas larga desde el nodo hasta el final

  ra[0] = max( 
    opPJ_A != NULL ? (opPJ_A->getTInicio() + opPJ_A->getDuration()) : 0 ,
    opPM_A != NULL ? (opPM_A->getTInicio() + opPM_A->getDuration()) : 0 );

  Operacion *opB = NULL;
  Operacion *opPJ_B = NULL;
  Operacion *opSJ_B = NULL;
  Operacion *opSM_B = NULL;

  for(int i=1; i < mov.seq.size(); i++) {
    opB = mov.seq[i];
    opPJ_B = opB->getJobAnterior();

    ra[i] = max(
      opPJ_B != NULL ? opPJ_B->getTInicio() + opPJ_B->getDuration() : 0,
      ra[i-1] + opA->getDuration()
      );
    opA = opB;
  }

  opB = mov.seq.back();
  opSJ_B = opB->getJobSiguiente();
  opSM_B = opB->getMachSiguiente();
  ta[tamSeq-1] = max(
    opSJ_B != NULL ? opSJ_B->getRT() : 0 ,
    opSM_B != NULL ? opSM_B->getRT() : 0 ) 
  + opB->getDuration();

  lower = ta[tamSeq-1] + ra[tamSeq-1];

  for(int i= tamSeq-2; i >= 0; i--) {
    opA = mov.seq[i];
    opSJ_A = opA->getJobSiguiente();

    ta[i] = max(
      opSJ_A != NULL ? opSJ_A->getRT() : 0,
      ta[i+1]
      ) + opA->getDuration();

    lower = max(lower, ta[i]+ra[i]);

    opB = opA;
  }

  if (mov.tipo == 1 || mov.tipo == 4) {
    opTmp = mov.seq.back();
    mov.seq.pop_back();
    mov.seq.push_front(opTmp);
  } else {
    opTmp = mov.seq.front();
    mov.seq.pop_front();
    mov.seq.push_back(opTmp);
  }

  return lower;
}