#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H
#include <iostream>
using namespace std;


struct reserva {
  char Agente[20];
  char nomFamilia[20];
  int cantFamiliares;
  int horaInicio;
};

struct agenteInfo {
  char nombreAgente[20];
  int pid;
  bool primera = true;
};

#endif /* MI_HEADER_H */
