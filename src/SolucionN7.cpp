/* Vecindad N7
  - Mover una operacion interna al inicio o al final de un bloque, o
  - Insertar la primera o la ultima operacion de un bloque critico,
    como una operacion interna (dentro del bloque critico).

    La vecindad considera los siguientes criterios:
    (1) Si u, v y JS(v) estan en la ruta critica,
      insertar u a la derecha de v. [(v, u)]
    (2) Si u, v y JS(v) estan en la ruta critica,
      mover v a la posicion antes de la operacion interna.
    (3) Si u, v y JP(u) estan en la ruta critica,
      insertar v justo antes de u. [(v, u)]
    (4) Si u, v y JP(u) estan en la ruta critica,
      mover u a la posicion despues de la operacion interna.
  
    JS, trabajo siguiente, op->getJobSiguiente()
    JP, trabajo anterior, op->getJobAnterior()
 */
Movimiento Solucion::getBestN7(
  deque<Movimiento > &listaTabu,
  int itActual, int tamLT,
  int bestVal, bool bestAllN) {
  return getBestN(listaTabu, itActual, tamLT, bestVal, bestVal, bestAllN, 7);
} // FIN GET_BEST_N7

vector<Movimiento> Solucion::getMovimientosN7(bool extender) {

  int numJobs = g->getNumTrabajos();
  set<Movimiento> movs;
  deque<Operacion *> seq;

  int numBloques=0;
  int totalBloq=0;

  for(int i=0; i < bloques.size(); i++) {
    totalBloq += bloques[i].size();
    //cout << "\tbloques[" << i <<"].size()=" << bloques[i].size() << "\n";
    if (bloques[i].size() == 0) continue;

    for (auto bloq : bloques[i]) {
      int bIni=bloq.first;
      int bFin=bloq.second;

      numBloques++;

    for(bIni=bloq.first;  (extender && bIni < bloq.second) || (!extender && bIni == bloq.first) ; bIni++) {
      for(bFin=bloq.second; (extender && bFin > bloq.first) || (!extender && bFin == bloq.second) ; bFin--) {

//        cout << "\tM"<< i << ", bloque[" << bIni <<" , " << bFin << " ]" << "\n";

      // validar indices
      if (bIni < 0 || bIni >= (numJobs-1)
        || bFin < 1 || bFin >= numJobs
        || bIni >= bFin ) continue; 

      Operacion *JP_U = perms[i][bIni]->getJobAnterior();
      Operacion *JS_V = perms[i][bFin]->getJobSiguiente();

      // Si es un bloque de tamanio 2, el movimiento sera un swap
/*      if ( (bloq.second - bloq.first) == 1 
          && ( (JP_U != NULL  && JP_U->getJob() != -1 //)
                && JP_U->getIsRC() ) 
            || (JS_V != NULL && JS_V->getJob() != -1  //)
               && JS_V->getIsRC() ) 
            ) 
        ) {
        Movimiento mSwap(0, this->perms[i][bloq.first], this->perms[i][bloq.second]);
        mSwap.maquina = i;
        mSwap.indOp1 = bloq.first;
        mSwap.seq.push_back(mSwap.op1);
        mSwap.seq.push_back(mSwap.op2);
        movs.insert(mSwap);
        //cout << "Agregrando mov de swap a candidatos...\n";
        continue;
      } */

      // JP[U] esta en la ruta critica
      if (JP_U != NULL // && JP_U->getJob() != -1 ) {
        && ( (extender && JP_U->getJob() != -1) || (!extender && JP_U->getIsRC() ) ) ) {

        /* if (!JP_U->getIsRC()) {
           cout << "ERROR, JP[u] no esta en RC\n";
          cout << "M" << i << ", Bloque: [" << bloq.first << "," << bloq.second << "]\n";
          cout << "OP_U: " << perms[i][bloq.first]->getJob() << "\n";
          cout << "JP_U: " << JP_U->getJob() << ", M" << JP_U->getMachine() << "\n";

          cout << "Sol Act: \n" << this->getPrintPerms() << "\n";
          cout << "Rutas: \n" << this->toString() << "\n";
          exit(0); 
        } */

        seq.clear();
        seq.push_back(perms[i][bIni]);
        for(int j=bIni+1; 
          (extender &&  j < perms[i].size() ) || (!extender && j <= bFin)
          ;j++) {
          seq.push_back(perms[i][j]);
          // (3) insertar v a la izquierda de u
          Movimiento mInsertIzq(3, this->perms[i][bIni], this->perms[i][j]);
          mInsertIzq.maquina = i;
          mInsertIzq.indOp1 = bIni;
          mInsertIzq.seq = seq;
          if ( (j - bIni) == 1 ) mInsertIzq.tipo=0;
          movs.insert(mInsertIzq);

          if ( (j - bIni) == 1 ) continue;

          // (4) mover u a la derecha v
          Movimiento mMovDer(4, this->perms[i][bIni], this->perms[i][j]);
          mMovDer.maquina = i;
          mMovDer.indOp1 = bIni;
          mMovDer.seq = seq;
          movs.insert(mMovDer); 
        } 
      }

      // JS[V] esta en la ruta critica
      if (JS_V != NULL // && JS_V->getJob() != -1 ) {
        && ( (extender && JS_V->getJob() != -1) || (!extender && JS_V->getIsRC() ) ) ) {
        seq.clear();
        seq.push_front(perms[i][bFin]);

        for(int j=bFin-1; 
          (!extender && j >= bIni) || (extender && j >= 0 )
          ;j--) {
          // (1) insertar u a la derecha de v
          seq.push_front(perms[i][j]);

          Movimiento mInsertDer(1, this->perms[i][j], this->perms[i][bFin]);
          mInsertDer.maquina = i;
          mInsertDer.indOp1 = j;
          mInsertDer.seq = seq;
          if ( (bFin - j) == 1 ) mInsertDer.tipo=0;
          movs.insert(mInsertDer);

         if ( (bFin - j) == 1 ) continue;

          // (2) mover v a la izquierda de u
          Movimiento mMovIzq(2, this->perms[i][j], this->perms[i][bFin]);
          mMovIzq.maquina = i;
          mMovIzq.indOp1 = j;
          mMovIzq.seq = seq;
          movs.insert(mMovIzq);
        }
      } // fin if js_V 

       } } //fin de recorrer ini, fin...

    } // FIN Procesa  bloques de maquina i
  } // Fin procesa bloques

  vector<Movimiento> res(movs.begin(), movs.end());

  return res;
} // FIN GET_MOVIMIENTOSN7

vector<Movimiento> Solucion::getMovimientosN6() {

  int numJobs = g->getNumTrabajos();
  set<Movimiento> movs;
  deque<Operacion *> seq;

  int numBloques=0;
  int totalBloq=0;

  for(int i=0; i < bloques.size(); i++) {
    totalBloq += bloques[i].size();
    
    if (bloques[i].size() == 0) continue;

    for (auto bloq : bloques[i]) {
      int bIni=bloq.first;
      int bFin=bloq.second;

      numBloques++;

      // validar indices
      if (bIni < 0 || bIni >= (numJobs-1)
        || bFin < 1 || bFin >= numJobs
        || bIni >= bFin ) continue; 

      Operacion *JP_U = perms[i][bIni]->getJobAnterior();
      Operacion *JS_V = perms[i][bFin]->getJobSiguiente();

      // Si es un bloque de tamanio 2, el movimiento sera un swap
      if ( (bloq.second - bloq.first) == 1 
          && ( (JP_U != NULL  && JP_U->getJob() != -1 )
                //&& JP_U->getIsRC() ) 
            || (JS_V != NULL && JS_V->getJob() != -1 )
              // && JS_V->getIsRC() ) 
            ) 
        ) {
        Movimiento mSwap(0, this->perms[i][bloq.first], this->perms[i][bloq.second]);
        mSwap.maquina = i;
        mSwap.indOp1 = bloq.first;
        mSwap.seq.push_back(mSwap.op1);
        mSwap.seq.push_back(mSwap.op2);
        movs.insert(mSwap);
        //cout << "Agregrando mov de swap a candidatos...\n";
        continue;
      }

      // JP[U] esta en la ruta critica
      if (JP_U != NULL // && JP_U->getJob() != -1 ) {
        && JP_U->getIsRC()) {

        seq.clear();
        seq.push_back(perms[i][bIni]);
        for(int j=bIni+1; j <= bFin; j++) {
          seq.push_back(perms[i][j]);

          // (3) insertar v a la izquierda de u
          Movimiento mInsertIzq(3, this->perms[i][bIni], this->perms[i][j]);
          mInsertIzq.maquina = i;
          mInsertIzq.indOp1 = bIni;
          mInsertIzq.seq = seq;
          if ( (j - bIni) == 1 ) mInsertIzq.tipo=0;
          movs.insert(mInsertIzq);
        } 
      }

      // JS[V] esta en la ruta critica
      if (JS_V != NULL // && JS_V->getJob() != -1 ) {
         && JS_V->getIsRC()) {
        seq.clear();
        seq.push_front(perms[i][bFin]);
        for(int j=bFin-1; j >= bloq.first; j--) {
          // (1) insertar u a la derecha de v
          seq.push_front(perms[i][j]);

          Movimiento mInsertDer(1, this->perms[i][j], this->perms[i][bFin]);
          mInsertDer.maquina = i;
          mInsertDer.indOp1 = j;
          mInsertDer.seq = seq;
          if ( (bFin - j) == 1 ) mInsertDer.tipo=0;
          movs.insert(mInsertDer);
        }
      } // fin if js_V 

    } // FIN Procesa  bloques de maquina i
  } // Fin procesa bloques

  vector<Movimiento> res(movs.begin(), movs.end());

  return res;
} // FIN GET_MOVIMIENTOSN6

vector<Movimiento> Solucion::getMovimientosN4(bool extender) {

  int numJobs = g->getNumTrabajos();
  set<Movimiento> movs;
  deque<Operacion *> seq;

  int numBloques=0;
  int totalBloq=0;

  for(int i=0; i < bloques.size(); i++) {
    totalBloq += bloques[i].size();
    
    if (bloques[i].size() == 0) continue;

    for (auto bloq : bloques[i]) {
      int bIni=bloq.first;
      int bFin=bloq.second;

      numBloques++;

      // validar indices
      if (bIni < 0 || bIni >= (numJobs-1)
        || bFin < 1 || bFin >= numJobs
        || bIni >= bFin ) continue; 

      Operacion *JP_U = perms[i][bIni]->getJobAnterior();
      Operacion *JS_V = perms[i][bFin]->getJobSiguiente();

      // Si es un bloque de tamanio 2, el movimiento sera un swap
      if ( (bloq.second - bloq.first) == 1 
          && ( (JP_U != NULL  && JP_U->getJob() != -1 )
                //&& JP_U->getIsRC() ) 
            || (JS_V != NULL && JS_V->getJob() != -1 )
              // && JS_V->getIsRC() ) 
            ) 
        ) {
        Movimiento mSwap(0, this->perms[i][bloq.first], this->perms[i][bloq.second]);
        mSwap.maquina = i;
        mSwap.indOp1 = bloq.first;
        mSwap.seq.push_back(mSwap.op1);
        mSwap.seq.push_back(mSwap.op2);
        movs.insert(mSwap);
        //cout << "Agregrando mov de swap a candidatos...\n";
        continue;
      }

      // JP[U] esta en la ruta critica
      if ( (JP_U != NULL && JP_U->getIsRC() ) 
          || (JS_V != NULL && JS_V->getIsRC()) ) {
        seq.clear();
        seq.push_back(perms[i][bIni]);
        for(int j=bIni+1; 
          (extender &&  j < perms[i].size() ) || (!extender && j <= bFin)
          ; j++) {
          seq.push_back(perms[i][j]);

          // (3) insertar v a la izquierda de u
          Movimiento mInsertIzq(3, this->perms[i][bIni], this->perms[i][j]);
          mInsertIzq.maquina = i;
          mInsertIzq.indOp1 = bIni;
          mInsertIzq.seq = seq;
          if ( (j - bIni) == 1 ) mInsertIzq.tipo=0;

          // Checar Factibilidad:
          /*int op2 = j;
          bool fact=true;
          for(int op1=bIni; op1 < op2; i++) {
            Operacion *JS_U = s->perms[numMaq][op1]->getJobSiguiente();
            Operacion *JP_V = s->perms[numMaq][op2]->getJobAnterior();

            int c_sj = JS_U != NULL ? JS_U->getTInicio() + JS_U->getDuration() : s->makespan;
            int c_pj = JP_V != NULL ? JP_V->getTInicio() : 0;
         
            // Checar Factibilidad:
            if ( c_sj <= c_pj ) {
              fact = false;
              break; // el movimiento podria ser infactible
            }
          }
          if (fact)*/
            movs.insert(mInsertIzq);
        }

        seq.clear();
        seq.push_front(perms[i][bFin]);
        for(int j=bFin-1; 
          (!extender && j >= bIni) || (extender && j >= 0 )
          ; j--) {
          seq.push_front(perms[i][j]);

          // (1) insertar u a la derecha de v
          Movimiento mInsertDer(1, this->perms[i][j], this->perms[i][bFin]);
          mInsertDer.maquina = i;
          mInsertDer.indOp1 = j;
          mInsertDer.seq = seq;
          if ( (bFin - j) == 1 ) mInsertDer.tipo=0;
          movs.insert(mInsertDer);
        }

      }

    } // FIN Procesa  bloques de maquina i
  } // Fin procesa bloques

  vector<Movimiento> res(movs.begin(), movs.end());

  return res;
} // FIN GET_MOVIMIENTOSN4