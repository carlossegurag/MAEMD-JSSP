#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>

#include "UtilJS.hpp"

using namespace std;

int main(int argc, char **argv) {

  string nombreArchivo(argv[1]);

  string prefijo(argv[3]);

  ifstream fileInsts;
  fileInsts.open(nombreArchivo.c_str());

  string instancia;
  int valOpt;

  // Numero de repeticiones por Instancia
  int n = 25;

  string ruta(argv[2]);

  cout << "instancia\tvalOpt\tnumRep"
    << "\tmejor\tpeor\tpromedio\tporcExs"
    << "\ttProm\ttMin\ttMax"
    << "\titerProm\titerMin\titerMax"


    << "\n";

  // Numero de Instnacias
  int totalInst = 16;
  int numInst = 0;

  if( fileInsts.is_open() ) {
    
    while(!fileInsts.eof() && numInst < totalInst) {
      fileInsts >> instancia >> valOpt >> n;

      string archivoSols( ruta + prefijo + instancia + "_sols.txt" );
      string archivoRes( ruta + prefijo + instancia + "_res.txt" );

      cout << instancia << "\t" << valOpt << "\t" << n << "\t"; 

      UtilJS::procesarResulAG(archivoRes, archivoSols, n, valOpt);
      numInst++;
    } 
    fileInsts.close();    
  }
  
  return 0;
} 