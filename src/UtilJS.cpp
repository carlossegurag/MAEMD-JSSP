#include <fstream>
#include <cmath>
#include <iostream>
#include <algorithm>    // std::min_element, std::max_element
#include <climits>  // INT_MAX

#include "UtilJS.hpp"

unsigned int UtilJS::permutationHash(const vector<int> & s1) {
  unsigned int res = 1;

  unsigned int R = 1779033703;

  for (int i=0; i < s1.size(); i++) {
    res *= (R+2*s1[i]);
  }

  return res/2;
}

unsigned int UtilJS::hashPerms(const vector<vector<int>> & s1) {
  unsigned int res = 1;

  unsigned int R = 1779033703;

  for (int i=0; i < s1.size(); i++) {
    res *= (R+2*permutationHash(s1[i]));
  }

  return res/2;
}

double UtilJS::similitudPromPob(vector<vector<vector<int>>> & pob) {
  double res = 0;

  for(int i =0; i < pob.size(); i++) {
    res += similitudPobPerm(pob, pob[i], i);
  }

  res = res/pob.size();

  return res;
}

int UtilJS::similitudPobPerm(vector<vector<vector<int>>> & pob, vector<vector<int>> & ind, int idx) {
  int res = 0;

  int tmp = 0;

  for(int i=0; i < pob.size(); i++) {
    if (i== idx) continue;
    tmp = similitudPerms(pob[i], ind);
    res = tmp > res ? tmp : res;
  }

  return res;
}

int UtilJS::similitudPerms(vector<vector<int>> & s1, vector<vector<int>> & s2) {

  int sumaLCS = 0; 
  vector< pair<int, int> > tmp;

  for(int i=0; i < s1.size(); i++) {
    tmp = LCS_int(s1[i], s2[i]);
    sumaLCS += tmp.size();
  }

  return sumaLCS;
}

/* distancia de la diferencia entre las posiciones */
int UtilJS::difPosPerms(vector<vector<int>> & s1, vector<vector<int>> & s2) {
  int sumaAcc = 0;
  int totalOps = s1.size()*s1[0].size(); 
  vector<int> posS1(totalOps);

  for(int i=0; i < s1.size(); i++) {
    for(int j=0; j < s1[i].size(); j++) {
      posS1[ s1[i][j] - 1 ] = j;
    }
  }

  for(int i=0; i < s1.size(); i++) {
    for(int j=0; j < s1[i].size(); j++) {
      sumaAcc += abs( j -  posS1[ s2[i][j] - 1]);
    }
  }

  return sumaAcc;
}

/* Numero de operaciones que diferen en posicion  */
int UtilJS::numPosDefPerms(vector<vector<int>> & s1, vector<vector<int>> & s2) {
  int sumaAcc = 0;
  int totalOps = s1.size()*s1[0].size(); 
  vector<int> posS1(totalOps);

  for(int i=0; i < s1.size(); i++) {
    for(int j=0; j < s1[i].size(); j++) {
      posS1[ s1[i][j] - 1 ] = j;
    }
  }

  for(int i=0; i < s1.size(); i++) {
    for(int j=0; j < s1[i].size(); j++) {
      if ( j != posS1[ s2[i][j] - 1 ] ) sumaAcc++;
    }
  }

  return sumaAcc;
}

vector< map< int, pair<int, int> > > UtilJS::NCS_vint(
  vector<vector<int>> & s1, vector<vector<int>> & s2) {
  vector< map< int, pair<int, int> > > res;

  for(int i=0; i < s1.size(); i++) {
    res.push_back(NCS_int(s1[i], s2[i]));
  }

  return res;
}

// map[val] = (pos_x, pos_y)
// El mapa almacena los valores (con sus posiciones) que corresponden
// a elementos que no coinciden (en posicion)
map< int, pair<int, int> > UtilJS::NCS_int(vector<int> & x, vector<int> & y) {
  map< int, pair<int,int> > res;

  map< int, pair<int,int> >::iterator it;
  pair<int,int> pTmp;

  // Recorremos el vector X
  for(int i=0; i < x.size(); i++) {
    // Si los elementos son iguales (mismo valor en la misma posicion),
    // se ignoran. En caso contrario, almacenosmo el valor y la posicion
    // en la que aparece en el vector x y en el vector y.
    if (x[i] != y[i]) {
      it = res.find(x[i]);
      if (it != res.end()) { // el elemento ya aparecio en y
        pTmp = it->second;
        pTmp.first = i;
        res[x[i]] = pTmp;
      } else {
        pTmp = make_pair(i, -1);
        res.insert( make_pair(x[i], pTmp) );
      }

      it = res.find(y[i]);
      if (it != res.end()) { // // el elemento ya aparecio en x
        pTmp = it->second;
        pTmp.second = i;
        res[y[i]] = pTmp;
      } else {
        pTmp = make_pair(-1, i);
        res.insert( make_pair(y[i], pTmp) );
      }
    }
  }

  return res;
}

vector< vector< pair<int, int> > > UtilJS::LCS_vint(
  vector<vector<int>> & s1, vector<vector<int>> & s2) {
  
  vector< vector< pair<int, int> > > res;

  for(int i=0; i < s1.size(); i++) {
    res.push_back(LCS_int(s1[i], s2[i]));
  }

  return res;
}

vector< pair<int, int> > UtilJS::LCS_int(vector<int> & x, vector<int> & y) {

  int n = x.size();
  int m = y.size();

  int C[n+1][m+1] = {0};

  // Generamos la matriz 
  for (int i=0; i <= n; i++) {
    for (int j=0; j <= m; j++) {
      if (i== 0 || j==0) {
        C[i][j] = 0;
      } else if (x[i-1] == y[j-1]) {
        C[i][j] = 1 + C[i-1][j-1];
      } else {
        C[i][j] = C[i-1][j] > C[i][j-1] ? C[i-1][j] : C[i][j-1];

      }
    }
  }

  int lcs = C[n][m];

  // recuperamos la cadena
  vector< pair<int, int> > pos(lcs);
  int num = 0;
  int i = n;
  int j = m;

  while ( num < C[n][m] && i > 0 && j > 0 ) {
    if (x[i-1] == y[j-1] ) {
      pos[C[n][m] - 1 - num++] = make_pair(i-1, j-1);
      i--;
      j--;
      lcs--;
    } else if (C[i-1][j] == lcs) {
      i--;
    } else {
      j--;
    }
  }

//  return C[n][m];
  return pos;
}

void UtilJS::guardarPerms(vector<vector<int>> & s1, string nombreArchivo) {
  ofstream filePerms;
  filePerms.open(nombreArchivo.c_str(), std::ofstream::out | std::ofstream::app) ;

  int m = s1.size();
  int n = s1[0].size();

  if (filePerms.is_open()) {
    filePerms << m << " " << n << "\n";
    for(int i=0; i < m; i++) {
      for(int j=0; j < n; j++) {
        filePerms << s1[i][j] << " ";
      }
      filePerms << "\n";
    }
    filePerms.flush();
    filePerms.close();
  }
}

void UtilJS::escribirPermutacion( vector<vector<int>> & s1 , ostream & out) {
  out << s1.size() << " " << s1[0].size() << " ";
  for(int i=0; i < s1.size(); i++) {
    for(int j=0; j < s1[i].size(); j++) {
      out << s1[i][j] << " ";
    }
  }
  out << std::flush;
}

vector<vector<int>> UtilJS::leerPermutacion( istream & in) {
  int m;
  int n;
  in >> m >> n;

  vector<vector<int>> res(m);
  for(int i=0; i < m; i++) {
    res[i].resize(n);
    for(int j=0; j < n; j++) {
      in >> res[i][j];
    }
  }  
  return res;
}

/* idsOps[i][j], es el id de la operacion del trabajo j, en la maquina i */
vector< vector<int> > UtilJS::leerPerms(ifstream & filePerms, int n, int m,
  vector<vector<int>> & idsOps ) {

  vector<vector<int>> perms(m);
  int idJob=0;

  /* cout << "Ids Recibidos: \n";
  for(int i=0; i < idsOps.size(); i++) {
    for (int j=0; j < idsOps[i].size(); j++) {
      cout << idsOps[i][j] << " ";
    }
    cout << "\n";
  }
  cout << "\n"; */

  if( filePerms.is_open() ) {
    cout << "# Leyendo permutacion de tamanio: m=" << m << " , n=" << n << " ...\n";
 
      for(int i=0; i < m; i++) {
        perms[i].resize(n);
        for(int j=0; j < n; j++) {
          filePerms >> idJob; // leemos id del job
          //cout << "M" << i << "," "IdJob: " << idJob << "\n";

          // perms[i][j], es el id de la operacion j-esima operacion en la maquina i
          perms[i][j] = idsOps[ i ] [idJob];

        }
      }
  //  } 
  //  filePerms.close();    
  } else {
    cout << "ERROR al abrir el archivo...\n";
  }

  return perms;

}

vector<vector<int>> UtilJS::leerPerms(ifstream & filePerms){

//  ifstream filePerms;
//  filePerms.open(nombreArchivo.c_str());

  int n=0;
  int m=0;
  vector<vector<int>> perms;

  if( filePerms.is_open() ) {
    
      filePerms >> m >> n;

      perms.resize(m);
   
      for(int i=0; i < m; i++) {
        perms[i].resize(n);
        for(int j=0; j < n; j++) {
          filePerms >> perms[i][j]; // imprimimos id de la operacion
        }
      }
  }

  return perms;
}

void UtilJS::procesarResulAG(string archivoRes, string archivoSols, int n, int BKS) {
  ifstream filePerms;
  filePerms.open(archivoSols.c_str());

  if (!filePerms.is_open()) {
    cout << "Error al abrir el archivo: " << archivoSols << "\n";
    return;
  }

  ifstream fileRes;
  fileRes.open(archivoRes.c_str());

  if (!fileRes.is_open()) {
    filePerms.close();
    cout << "Error al abrir el archivo: " << archivoRes << "\n";
    return;
  }

  vector<int> bestAG(n);
  vector<double> timeAG(n);
  
  vector<int> iterAG(n);

  double numExitos = 0;

  for(int i=0; i < n; i++) {
    fileRes >> bestAG[i];
    fileRes >> iterAG[i];
    fileRes >> timeAG[i];

    if (bestAG[i] <= BKS) numExitos++;
  }

  cout << *min_element( begin(bestAG), end(bestAG)) << "\t ";
  cout << *max_element( begin(bestAG), end(bestAG)) << "\t ";
  cout << UtilJS::average<int> (bestAG) << "\t ";
  cout << ((numExitos/n)*100) << "\t ";

  cout << UtilJS::average<double> (timeAG) << "\t ";
  cout << *min_element( begin(timeAG), end(timeAG)) << "\t ";
  cout << *max_element( begin(timeAG), end(timeAG)) << "\t ";

  cout << UtilJS::average<int> (iterAG) << "\t ";
  cout << *min_element( begin(iterAG), end(iterAG)) << "\t ";
  cout << *max_element( begin(iterAG), end(iterAG)) << "\t ";

  cout << "\n";

  filePerms.close();
  fileRes.close();
}

void UtilJS::procesarResul(string archivoRes, string archivoSols, int n,
  int BKS) {

  ifstream filePerms;
  filePerms.open(archivoSols.c_str());

  if (!filePerms.is_open()) {
    cout << "Error al abrir el archivo: " << archivoSols << "\n";
    return;
  }

  ifstream fileRes;
  fileRes.open(archivoRes.c_str());

  if (!fileRes.is_open()) {
    filePerms.close();
    cout << "Error al abrir el archivo: " << archivoRes << "\n";
    return;
  }

//  vector<vector<vector<int>>> solsIni(n);
//  vector<vector<vector<int>>> solsTS(n);
//  vector<vector<vector<int>>> solsHC(n);

  vector<int> makespanIni(n);

  vector<int> bestTS(n);
  vector<double> timeTS(n);
  vector<int> simIni_TS(n);
  vector<int> iterTS(n);
  vector<int> iterBTS(n);
  vector<int> nsolRepTS(n);
  vector<int> maxsolRepTS(n);
  vector<double> promsolRepTS(n);
  vector<int> numSF_TS(n);
  vector<int> numSNF_TS(n);


  vector<int> bestTS5(n);
  vector<double> timeTS5(n);
  vector<int> simIni_TS5(n);
  vector<int> iterTS5(n);
  vector<int> iterBTS5(n);
  vector<int> nsolRepTS5(n);
  vector<int> maxsolRepTS5(n);
  vector<double> promsolRepTS5(n);
  vector<int> numSF_TS5(n);
  vector<int> numSNF_TS5(n);
/*  vector<int> bestHC(n);
  vector<double> timeHC(n);
  vector<int> simIni_HC(n);
  vector<int> iterHC(n);

  vector<int> simTS_HC(n); */

  double numExitos = 0;
  double numExitos5 = 0;

  for(int i=0; i < n; i++) {
//    solsIni[i] = UtilJS::leerPerms(filePerms);
//    solsTS[i] = UtilJS::leerPerms(filePerms);
//    solsHC[i] = UtilJS::leerPerms(filePerms);
    fileRes >> makespanIni[i];

    fileRes >> bestTS[i];
    fileRes >> timeTS[i];
    fileRes >> simIni_TS[i];
    fileRes >> iterTS[i];
    fileRes >> iterBTS[i];
    fileRes >> nsolRepTS[i];
    fileRes >> maxsolRepTS[i];
    fileRes >> promsolRepTS[i];
    fileRes >> numSF_TS[i];
    fileRes >> numSNF_TS[i];

    fileRes >> bestTS5[i];
    fileRes >> timeTS5[i];
    fileRes >> simIni_TS5[i];
    fileRes >> iterTS5[i];
    fileRes >> iterBTS5[i];
    fileRes >> nsolRepTS5[i];
    fileRes >> maxsolRepTS5[i];
    fileRes >> promsolRepTS5[i];
    fileRes >> numSF_TS5[i];
    fileRes >> numSNF_TS5[i];

/*    fileRes >> bestHC[i];
    fileRes >> timeHC[i];
    fileRes >> simIni_HC[i];
    fileRes >> iterHC[i];
    fileRes >> simTS_HC[i];*/

    if (bestTS[i] <= BKS) numExitos++;
    if (bestTS5[i] <= BKS) numExitos5++;
  }
  filePerms.close();
  fileRes.close();

/*  int numMaquinas = solsIni[0].size();
  int numTrabajos = solsIni[0][0].size();
  int tamInst = numTrabajos*numMaquinas;

  double simIni = UtilJS::similitudPromPob(solsIni)/tamInst;
  double simiTS = UtilJS::similitudPromPob(solsTS)/tamInst;
  double simiHC = UtilJS::similitudPromPob(solsHC)/tamInst; */

  /* datos TS N7 */
  cout << *min_element( begin(bestTS), end(bestTS)) << "\t ";
  cout << *max_element( begin(bestTS), end(bestTS)) << "\t ";
  cout << UtilJS::average<int> (bestTS) << "\t ";
  cout << ((numExitos/n)*100) << "\t ";

  cout << UtilJS::average<double> (timeTS) << "\t ";
  cout << UtilJS::average<int> (iterTS) << "\t ";
  cout << UtilJS::average<int> (iterBTS) << "\t ";
  cout << UtilJS::average<int> (nsolRepTS) << "\t ";
  cout << UtilJS::average<int> (maxsolRepTS) << "\t ";
  cout << UtilJS::average<double> (promsolRepTS) << "\t ";

  cout << UtilJS::average<int> (numSF_TS) << "\t ";
  cout << UtilJS::average<int> (numSNF_TS) << "\t ";


  /* datos TS_N5 */
  cout << *min_element( begin(bestTS5), end(bestTS5)) << "\t ";
  cout << *max_element( begin(bestTS5), end(bestTS5)) << "\t ";
  cout << UtilJS::average<int> (bestTS5) << "\t ";
  cout << ((numExitos5/n)*100) << "\t ";

  cout << UtilJS::average<double> (timeTS5) << "\t ";
  cout << UtilJS::average<int> (iterTS5) << "\t ";
  cout << UtilJS::average<int> (iterBTS5) << "\t ";
  cout << UtilJS::average<int> (nsolRepTS5) << "\t ";
  cout << UtilJS::average<int> (maxsolRepTS5) << "\t ";
  cout << UtilJS::average<double> (promsolRepTS5) << "\t ";

  cout << UtilJS::average<int> (numSF_TS5) << "\t ";
  cout << UtilJS::average<int> (numSNF_TS5) << "\t ";



  //cout << simIni << "\t " << simiTS << "\t " << simiHC 
  cout << "\n";
}

template< typename T>
double UtilJS::variance(vector<T> & v) {
  double avg = UtilJS::average<T>(v);
  double sum = 0.0;
  double temp =0.0;
  double var =0.0;
       
  for ( int j =0; j < v.size(); j++) {
    temp = (v[j] - avg)*(v[j] - avg);
    sum += temp;
  }
       
  return sum/(v.size()-2);
}

template< typename T>
double UtilJS::average(vector<T> & v) {
  double res = 0.0;
  int n = v.size();
       
  for ( int i=0; i < n; i++) {
    res += v[i];
  }
       
  return ( res / n);
}

