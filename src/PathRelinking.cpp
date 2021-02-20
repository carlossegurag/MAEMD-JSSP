#include <iostream>     // std::cout, std::ostream, std::io
#include <algorithm>    // std::random_shuffle
#include "PathRelinking.hpp"
#include "UtilJS.hpp"
#include "TabuSearch.hpp"

bool PathRelinking::generarLog = true;

int PathRelinking::pathRelinking(Solucion *s1, Solucion *s2, Solucion *res, int maxSinMejora) {

  VPermutaciones p1 = s1->getPermutacionSol();
  VPermutaciones p2 = s2->getPermutacionSol();

  vector< map<int,pair<int,int>> > ncss = UtilJS::NCS_vint(p1, p2);
  vector< pair<int,int> > keysMap;


  int totalOps = s1->getNumMaquinas()*s1->getNumTrabajos();

  vector<vector<int>> permAct = s2->getPermutacionSol();
  int distancia1 = totalOps - UtilJS::similitudPerms(p1, permAct); 
  int distancia2 = UtilJS::difPosPerms(p1, permAct); 
  int distancia3 = UtilJS::numPosDefPerms(p1, permAct);

  if (PathRelinking::generarLog)
    cout << "#\t Distancias Ini : " << distancia1 
      << " " << distancia2 << " " << distancia3 << "\n"; 

  // Recorrer todos los mapas y guardar las claves...
  for(int i=0; i < ncss.size(); i++) {
    if (ncss[i].size() == 0) continue;
    for (map< int, pair<int,int> >::iterator it=ncss[i].begin(); it!=ncss[i].end(); ++it) {
      keysMap.push_back(make_pair(i, it->first));
    }
  }

  random_shuffle( keysMap.begin(), keysMap.end());

//  vector<VPermutaciones> pathSet;

  int distIni = keysMap.size();
  //cout << "#\tPR, distIni: " << distIni << "\n";
  res->distPadres = distIni;


  int alfa = distIni/5;
  int beta = max(2, distIni/10);

  VPermutaciones pC; // sol Candidata
  VPermutaciones bestSol; // sol Candidata

  // Generar solucion candidata desde la sol inicial
  pC = PathRelinking::generarSolCandidata(p1, p2, alfa, keysMap, ncss);

  double probSelec = 1;
  int numBests=1;
  double tmpRand;

  distancia1 = totalOps - UtilJS::similitudPerms( p1, pC); 
  distancia2 = UtilJS::difPosPerms(p1, pC); 
  distancia3 = UtilJS::numPosDefPerms(p1, pC);

  if (PathRelinking::generarLog)
    cout << "#\t Distancias s1 a pC : " << distancia1 
      << " " << distancia2 << " " << distancia3 << "\n";

  res->setPermutacionSol(pC, false);
  res->reparar();
  res->calcularMakespan(true);

  permAct = res->getPermutacionSol();
  distancia1 = totalOps - UtilJS::similitudPerms(p1, permAct); 
  distancia2 = UtilJS::difPosPerms(p1, permAct); 
  distancia3 = UtilJS::numPosDefPerms(p1, permAct);

  if (PathRelinking::generarLog)
    cout << "#\t Distancias s1 a pC (despues Reparar) : " << distancia1 
      << " " << distancia2 << " " << distancia3 << "\n";

  TabuSearch TB;
  res->numItTS = 0;
  res->listaTabu.clear();
  res->permTS.clear();

  TB.tabuTenure = 3;
  int numItsCands = maxSinMejora/25;

  int makespanAct = TB.tabuSearch(res, 7, 0, 0, numItsCands); 
  int bestMakespan = makespanAct;
  bestSol = res->getPermutacionSol();

  // Agregar sol candidata a path set
//  pathSet.push_back(pc);

  int distAct = keysMap.size();

  if (PathRelinking::generarLog)
    cout << "#\tPR, dist(solC, solG) = " << distAct 
      << ", makespan: " << makespanAct << " its " << TB.numIt  << "\n";

// mientra la dist de la sol candidata a la sol final
  // sea mayor que alfa:
  while (distAct > alfa && distAct > 2) {
    // Genera solucion candidata
    pC = PathRelinking::generarSolCandidata(pC, p2, beta, keysMap, ncss);
    //pathSet.push_back(pc);

    // Actualiza distanciaAct
    distAct = keysMap.size();

    permAct = res->getPermutacionSol();
    distancia1 = totalOps - UtilJS::similitudPerms(p1, pC); 
    distancia2 = UtilJS::difPosPerms(p1, pC); 
    distancia3 = UtilJS::numPosDefPerms(p1, pC);

    if (PathRelinking::generarLog)
      cout << "#\t -> Distancias s1 a pC : " << distancia1 
        << " " << distancia2 << " " << distancia3 << "\n";

    // Reparar solucion generada
    res->setPermutacionSol(pC, false);
    res->reparar();
    res->calcularMakespan(true);

    permAct = res->getPermutacionSol();
    distancia1 = totalOps - UtilJS::similitudPerms(p1, permAct); 
    distancia2 = UtilJS::difPosPerms(p1, permAct); 
    distancia3 = UtilJS::numPosDefPerms(p1, permAct);

    if (PathRelinking::generarLog)
      cout << "#\t -> Distancias s1 a pC (despues Reparar) : " << distancia1 
        << " " << distancia2 << " " << distancia3 << "\n"; 

    TabuSearch tbi;
    res->numItTS = 0;
    res->listaTabu.clear();
    res->permTS.clear();

    tbi.tabuTenure = 3;

    // Aplicar busqueda tabu
    makespanAct = tbi.tabuSearch(res, 7, 0, 0, numItsCands);

    if (PathRelinking::generarLog)
      cout << "#\tPR, dist(solC, solG) = " << distAct 
        << ", makespan: " << makespanAct << " its " << tbi.numIt  << "\n";

    if (makespanAct == bestMakespan) {
      // Se actualiza el best de manera aleatoria
      probSelec = numBests/(numBests+1);
      numBests++;

      tmpRand = ((double) rand() / (RAND_MAX));

      // con probalidad (1 -probSelec) seleccionamos al nuevo ind como best
      if (tmpRand > probSelec) 
        bestSol = res->getPermutacionSol();

    }

    // Actualizar best
    if (makespanAct < bestMakespan) {
      bestMakespan = makespanAct;
      bestSol = res->getPermutacionSol();
      numBests = 1;
      probSelec = 1;
    }
  }

  res->setPermutacionSol(bestSol, true);

  TabuSearch tbF;
  res->numItTS = 0;
  res->listaTabu.clear();
  res->permTS.clear();

  maxSinMejora = maxSinMejora <= 0 ? 12500 : maxSinMejora;
  tbF.tabuTenure = 3;
  tbF.tabuSearch(res, 7, 0, 0, maxSinMejora);

  res->numItTS = tbF.numIt;
  res->timeTS = tbF.time_search;

  if (PathRelinking::generarLog)
    cout << "#\tPR_res makespan: " << res->getMakespan() << " its " << tbF.numIt  << "\n";

  return res->getMakespan();
}

VPermutaciones PathRelinking::generarSolCandidata(
  VPermutaciones & p1, VPermutaciones & p2, 
  int dist, vector< pair<int,int> > & keysMap, 
  vector< map<int,pair<int,int>> > & ncss) {

//  cout << "#PR, generando Sol candidata...\n";

  VPermutaciones res = p1;
  if (p2.size() == 0 || p1.size() == 0) return res;

  if (keysMap.size() <= dist) {
    keysMap.clear();
    return p2;
  }

  int cambios = 0;
  pair<int,int> key;
  pair<int, int> swap;
  pair<int, int> tmpPair;
  int tmpOp = 0;

  map< int, pair<int,int> >::iterator it;

  while (cambios < dist && keysMap.size() > 0) {
    key = keysMap.back();
    keysMap.pop_back();

    it = ncss[key.first].find(key.second);

    if ( it == ncss[key.first].end() ) continue;
    cambios++;
    swap = it->second;

//    if (res[key.first][swap.first] == p2[key.first][swap.second]) continue;

    tmpOp = res[key.first][swap.second];
    res[key.first][swap.second] = res[key.first][swap.first];
    res[key.first][swap.first] = tmpOp;

    ncss[key.first].erase(it);

    it = ncss[key.first].find( tmpOp ); 
    if ( it != ncss[key.first].end() ) {
      tmpPair = it->second;
      tmpPair.first = swap.first;

      if (tmpPair.first == tmpPair.second) { 
        // se igualo otra op de la sol ini hacia la sol fin 
        cambios++;
        ncss[key.first].erase(it);  
      } else { 
        // actualizar referencia
        ncss[key.first][tmpOp] = tmpPair;
      }
    }
  }


  return res;
}

