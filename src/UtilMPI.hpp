#ifndef UTIL_MPI_HPP
#define UTIL_MPI_HPP

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <fstream>
#include <mpi.h>

#include "Solucion.hpp"

#define FINALIZE 1
#define RES_TS 2
#define TASK_TS 3
#define TASK_PR 4

using namespace std;

class UtilMPI {

  public:
    static vector<int> make_msg_send_Ind(Solucion *s, int ind);
    static double upd_Ind_msg_rcv(Solucion *s, vector<int> msg);

    static vector<int> make_msg_send_IndPR(Solucion *s, Solucion *s2, int ind); 
    static double upd_Ind_msg_rcv_PR(Solucion *s1, Solucion *s2, vector<int> msg);

    static int NUM_DATA_EXTRA;

};

#endif