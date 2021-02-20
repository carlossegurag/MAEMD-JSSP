#include <sys/time.h>
#include "UtilMPI.hpp"

int UtilMPI::NUM_DATA_EXTRA=10;

vector<int> UtilMPI::make_msg_send_Ind(Solucion *s, int ind) {
  int totalOps = s->getNumMaquinas()*s->getNumTrabajos();

  int tamRes = totalOps*2 + UtilMPI::NUM_DATA_EXTRA;
  vector<int> buffMsgRes(tamRes);
  int k=0;
  buffMsgRes[k++] = ind;

  vector< vector<int> > buffSol = s->getPermutacionSol();
  for(int i=0; i < s->getNumMaquinas(); i++) {
    for(int j=0; j < s->getNumTrabajos(); j++) {
      buffMsgRes[k++] = buffSol[i][j];
    }
  }

  buffMsgRes[k++] = s->numItTS;

  buffSol = s->permTS;
  if (buffSol.size() == 0 || buffSol[0].size() == 0) {
    buffSol = s->getPermutacionSol();
  }
  for(int i=0; i < s->getNumMaquinas(); i++) {
    for(int j=0; j < s->getNumTrabajos(); j++) {
      buffMsgRes[k++] = buffSol[i][j];
    }
  }
  buffMsgRes[k++] = s->timeTS*1000;

  buffMsgRes[k++] = s->generacion;
  buffMsgRes[k++] = s->distPadres;
  buffMsgRes[k++] = s->distAlosPadres;
  buffMsgRes[k++] = s->mksP1;
  buffMsgRes[k++] = s->mksP2;

  struct timeval startH;
  gettimeofday(&startH, NULL);

  buffMsgRes[k++] = startH.tv_sec;
  buffMsgRes[k++] = startH.tv_usec;

  return buffMsgRes;
}

double UtilMPI::upd_Ind_msg_rcv(Solucion *s, vector<int> msg) {

  int totalOps = s->getNumMaquinas()*s->getNumTrabajos();

  vector<vector<int>> buffSol(s->getNumMaquinas(), vector<int>(s->getNumTrabajos()));

  int k=1;
  for(int i=0; i < s->getNumMaquinas(); i++) {
    for(int j=0; j < s->getNumTrabajos(); j++) {
      buffSol[i][j] = msg[k++];
    }
  }
  s->setPermutacionSol(buffSol);
  s->numItTS = msg[k++];
  for(int i=0; i < s->getNumMaquinas(); i++) {
    for(int j=0; j < s->getNumTrabajos(); j++) {
      buffSol[i][j] = msg[k++];
    }
  }
  s->permTS = buffSol;

  s->timeTS = msg[k++]/1000.0;

  s->generacion = msg[k++];
  s->distPadres = msg[k++];
  s->distAlosPadres = msg[k++];
  s->mksP1 = msg[k++];
  s->mksP2 = msg[k++];

  struct timeval startH, endH;
  gettimeofday(&endH, NULL);

  startH.tv_sec = msg[k++];
  startH.tv_usec = msg[k++];

  double time_search = endH.tv_sec - startH.tv_sec; 
  time_search += (endH.tv_usec -  startH.tv_usec)* 1e-6;

  return time_search;
}

double UtilMPI::upd_Ind_msg_rcv_PR(Solucion *s1, Solucion *s2, vector<int> msg) {

  int totalOps = s1->getNumMaquinas()*s1->getNumTrabajos();

  vector<vector<int>> buffSol(s1->getNumMaquinas(), vector<int>(s1->getNumTrabajos()));

  int k=1;
  for(int i=0; i < s1->getNumMaquinas(); i++) {
    for(int j=0; j < s1->getNumTrabajos(); j++) {
      buffSol[i][j] = msg[k++];
    }
  }
  s1->setPermutacionSol(buffSol);
  s1->numItTS = msg[k++];
  for(int i=0; i < s2->getNumMaquinas(); i++) {
    for(int j=0; j < s2->getNumTrabajos(); j++) {
      buffSol[i][j] = msg[k++];
    }
  }
  s2->setPermutacionSol(buffSol);

  s1->timeTS = msg[k++]/1000.0;

  s1->generacion = msg[k++];
  s1->distPadres = msg[k++];
  s1->distAlosPadres = msg[k++];
  s1->mksP1 = msg[k++];
  s1->mksP2 = msg[k++];

  struct timeval startH, endH;
  gettimeofday(&endH, NULL);

  startH.tv_sec = msg[k++];
  startH.tv_usec = msg[k++];

  double time_search = endH.tv_sec - startH.tv_sec; 
  time_search += (endH.tv_usec -  startH.tv_usec)* 1e-6;

  return time_search;
}


vector<int> UtilMPI::make_msg_send_IndPR(Solucion *s1, Solucion *s2, int ind) {
  int totalOps = s1->getNumMaquinas()*s1->getNumTrabajos();

  int tamRes = totalOps*2 + UtilMPI::NUM_DATA_EXTRA;
  vector<int> buffMsgRes(tamRes);
  int k=0;
  buffMsgRes[k++] = ind;

  vector< vector<int> > buffSol = s1->getPermutacionSol();
  for(int i=0; i < s1->getNumMaquinas(); i++) {
    for(int j=0; j < s1->getNumTrabajos(); j++) {
      buffMsgRes[k++] = buffSol[i][j];
    }
  }

  buffMsgRes[k++] = s1->numItTS;

  buffSol = s2->getPermutacionSol();  
  for(int i=0; i < s2->getNumMaquinas(); i++) {
    for(int j=0; j < s2->getNumTrabajos(); j++) {
      buffMsgRes[k++] = buffSol[i][j];
    }
  }

  // Estos datos no se usan en PR, se envian 
  // para mantener los msgs del mismo tamanio
  buffMsgRes[k++] = s1->timeTS*1000;
  buffMsgRes[k++] = s1->generacion;
  buffMsgRes[k++] = s1->distPadres;
  buffMsgRes[k++] = s1->distAlosPadres;
  buffMsgRes[k++] = s1->mksP1;
  buffMsgRes[k++] = s1->mksP2;

  struct timeval startH;
  gettimeofday(&startH, NULL);

  buffMsgRes[k++] = startH.tv_sec;
  buffMsgRes[k++] = startH.tv_usec;

  return buffMsgRes;
}