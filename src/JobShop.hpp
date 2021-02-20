#ifndef JOB_SHOP_HPP
#define JOB_SHOP_HPP

#include <string>
#include <vector>
#include <utility> // std:pair  

using namespace std;

/* 
 * Esta clase se utiliza solamente para leer 
 * los archivos de los benchmark
*/
class JobShop {

  private:
    int numTrabajos;
    int numMaquinas;

    // indica el orden de la secuncia de operaciones
    // cada renglon representa un trabajo
    int **operaciones;

    // indica los costos asociados a cada operacion
    int **tiempos;

  public:
	 JobShop(string nombreArchivo);
	 ~JobShop();

   int getNumMaquinas();
   int getNumTrabajos();

   int ** getOperaciones();
   int ** getTiempos();

   string toString();
};

#endif