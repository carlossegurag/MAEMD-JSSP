#ifndef Operacion_HPP
#define Operacion_HPP

#include <string>

using namespace std;

class Operacion {

  private:
    int job;
    int machine;
    int duration;
    int id;

    /* longitud de la ruta mas larga para
     llegar hasta esta operacion desde 0.
     Tambien se conoce como st o ri. */
    int tInicio;

    /* longitud de la ruta mas larga desde
     esta operacion hasta la op N */
    int rt;

    Operacion *jobAnt;
    Operacion *jobSig;

    Operacion *machAnt;
    Operacion *machSig;

    int indMach;

    /* Indica si esta en la Ruta Critica */
    bool isRC;
    int posRC;

  public:
    Operacion(int job, int machine, int duration, int id );
    ~Operacion();

    int getTInicio();
    void setTInicio(int t);

    int getRT();
    void setRT(int t);

    int getIndMach();
    void setIndMach(int ind);

    int getId();
    int getJob();
    int getMachine();
    int getDuration();

    bool getIsRC();
    void setRC(bool val);

    int getPosRC();
    void setPosRC(int pos);

    Operacion * getJobAnterior();
    void setJobAnterior(Operacion *op);

    Operacion * getJobSiguiente();
    void setJobSiguiente(Operacion *op);

    Operacion * getMachAnterior();
    void setMachAnterior(Operacion *op);

    Operacion * getMachSiguiente();
    void setMachSiguiente(Operacion *op); 

    Operacion * clone();

    string toString();
  
};

struct ComparaIdOperacion {
  bool operator()(Operacion *op1, Operacion *op2) {
    return op1->getId() < op2->getId();
  }
};

#endif