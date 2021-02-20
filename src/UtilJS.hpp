#ifndef UTIL_JS_HPP
#define UTIL_JS_HPP

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <fstream>

using namespace std;

class UtilJS {

  public:
    static vector< pair<int, int> > LCS_int(vector<int> & x, vector<int> & y);

    static vector< vector< pair<int, int> > > LCS_vint(
      vector<vector<int>> & s1, vector<vector<int>> & s2);

    static map< int, pair<int, int> > NCS_int(vector<int> & x, vector<int> & y);

    static vector< map< int, pair<int, int> > > NCS_vint(
      vector<vector<int>> & s1, vector<vector<int>> & s2);

    static int similitudPerms(vector<vector<int>> & s1, vector<vector<int>> & s2);
    static int similitudPobPerm(vector<vector<vector<int>>> & pob, 
      vector<vector<int>> & ind, int idx);


    static int difPosPerms(vector<vector<int>> & s1, vector<vector<int>> & s2);
    static int numPosDefPerms(vector<vector<int>> & s1, vector<vector<int>> & s2);

    static double similitudPromPob(vector<vector<vector<int>>> & pob);
    
    static void guardarPerms(vector<vector<int>> & s1, string nombreArchivo);


    static void escribirPermutacion( vector<vector<int>> & s1 , ostream & out);
    static vector<vector<int>> leerPermutacion( istream & in);

    static vector<vector<int>> leerPerms(ifstream & filePerms);

    static vector< vector<int> > leerPerms(ifstream & filePerms, int n, int m,
      vector<vector<int>> & idsOps );

    static void procesarResul(string archivoRes, string archivoSols, int n,
      int BKS);

    static void procesarResulAG(string archivoRes, string archivoSols, int n, int BKS);


    static unsigned int permutationHash( const vector<int> & s1);

    static unsigned int hashPerms(const vector<vector<int>> & s1);
    
    template <typename T>
      static double variance(vector<T> & v);

    template <typename T>
      static double average(vector<T> & v);

};

#endif