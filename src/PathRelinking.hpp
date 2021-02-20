#ifndef PATH_RELINKING_HPP
#define PATH_RELINKING_HPP

#include <vector>
#include <deque>
#include <map>

using namespace std;

#include "Solucion.hpp"

class PathRelinking{
  public:
    //PathRelinking();
    //~PathRelinking();
    static int pathRelinking(Solucion *s1, Solucion *s2, Solucion *res, int maxSinMejora=0);
 
    static VPermutaciones generarSolCandidata(
      VPermutaciones & p1, VPermutaciones & p2, 
      int dist, vector< pair<int,int> > & keysMap, 
      vector< map<int,pair<int,int>> > & ncss);

    static bool generarLog;

};

#endif