#ifndef AGENTE_H
#define AGENTE_H

#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>
#include <string>
#include <unistd.h>
#include <signal.h>
#include <vector>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <algorithm>  // Necesario para std::remove_if
#include <cctype>  
#include "estructuras.h"
using namespace std;


// Duración en segundos de una "hora"
int segundosPorHora;
char pipeNuevo[30] = "pipe2";
int fd1;
char identificador_emisor2 = '2';
reserva r;
bool terminado = false;
int horaGlobal;
string pipenom;
bool llegoS=false;
string nomAgenteGlobal;


void RecibirRespuesta(string nombreAgente);
void Sigusr1_handler(int signum);
void ProcesarSolicitudes(string nombreAgente, string archivoSolicitudes);
void Recibirhora(string nombreAgente);
void RecibirRespuesta(string nombreAgente);
void PrimeraConexion(string nombreAgente, string archivoSolicitudes);

#endif 