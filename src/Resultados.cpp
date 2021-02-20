#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>

#include "UtilJS.hpp"

using namespace std;

int main(int argc, char **argv) {

  string nombreArchivo(argv[1]);

  ifstream fileInsts;
  fileInsts.open(nombreArchivo.c_str());

  string instancia;
  int valOpt;

  int n = 25;

  string ruta(argv[2]);

  cout << "instancia\tvalOpt"
  // TS_N7
    << "\tmejor\tpeor\tpromedio\tporcExs"
    << "\ttProm\ttMin\ttMax\titerProm\titerMin\titerMax"
    << "\titerSalProm\titerSalMin\titerSalMax"
    << "\tnumRepProm\tnumRepMin\tnumRepMax"
    << "\tmaxRepProm\tmaxRepMin\tmaxRepMax"
    << "\tpromRepProm\tpromRepMin\tpromRepMax"
    << "\tsolNFProm\tsolNFMin\tsolNFMax"
    << "\tsolFProm\tsolFMin\tsolFMax"

  // TS_N5
    << "\tmejor\tpeor\tpromedio\tporcExs"
    << "\ttProm\ttMin\ttMax\titerProm\titerMin\titerMax"
    << "\titerSalProm\titerSalMin\titerSalMax"
    << "\tnumRepProm\tnumRepMin\tnumRepMax"
    << "\tmaxRepProm\tmaxRepMin\tmaxRepMax"
    << "\tpromRepProm\tpromRepMin\tpromRepMax"
    << "\tsolNFProm\tsolNFMin\tsolNFMax"
    << "\tsolFProm\tsolFMin\tsolFMax"

//    << "\tsimIni\tsimTS\tsimHC"

    << "\n";

  int totalInst = 12;
  int numInst = 0;

  if( fileInsts.is_open() ) {
    
    while(!fileInsts.eof() && numInst < totalInst) {
      fileInsts >> instancia >> valOpt;

      string archivoSols( ruta + instancia + "_sols.txt" );
      string archivoRes( ruta + instancia + "_res.txt" );

      cout << instancia << "\t" << valOpt << "\t"; 

      UtilJS::procesarResul(archivoRes, archivoSols, n, valOpt);
      numInst++;
    } 
    fileInsts.close();    
  }
  
  return 0;
} 