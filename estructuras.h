#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H
#include <iostream>
using namespace std;

struct reserva {
  bool registro = true;
  char Agente[20];
  char nomFamilia[20];
  int cantFamiliares;
  int horaInicio;
  bool ultimo = false;
  int respuesta = 0;
  bool reAgendado = false;
  int horaReAgendada;
  long pid=0;
};



#endif 
