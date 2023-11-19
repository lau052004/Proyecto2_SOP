#ifndef CONTROLADOR_H
#define CONTROLADOR_H

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <mutex>
#include <pthread.h>
#include <signal.h>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>
using namespace std;

struct comando {
  string comando_i = "-i";
  string comando_f = "-f";
  string comando_s = "-s";
  string comando_t = "-t";
  string comando_p = "-p";
  int valor_i;
  int valor_f;
  int valor_s;
  int valor_t;
  string valor_p;
};


std::map<int, std::vector<reserva>> reservasPorHora;
int solicitudesNegadas = 0;
int solicitudesAceptadas = 0;
int solicitudesReprogramadas = 0;
std::map<int, int> aceptadaHora; // Para mantener un registro de las personas por hora
std::mutex mtx; // Mutex para proteger la variable compartida
int segundosHora;
int horaActual; // Hora actual del parque
int horaInicio;
int horaFinal;
int totalPersonas; // aforo maximo
int cantAgentes = 0;
int posAgenteActual = 0;
vector<reserva> reservas;
vector<reserva> listaAgentes;
string nombrePipe1;
vector<string> nombrePipesReserva;
int alarmFlag = 0;


void MostrarEstadoDelParque();
void ManejadorSenales(int signum);
int EsNumero(const string &str);
bool ValidarComandos(int argc, string argumentos[], comando *comandoIngresado);
void GenerarInforme();
bool ContadorComando(int argc, string argumentos[], int comando);
bool ValoresCorrectos(int argc, string argumentos[], comando *comandoIngresado);
bool VerificarComando(int argc, string argumentos[], comando *comandos);
void EnviarResultado(const char *nombreAgente, const reserva &r);
void InicializarHoras();
void VerificarReservas(reserva &r);
void EnviarHora(char *nombrePipe);
void TerminarAgentes();
void *IncrementarHora(void *indice);
void *VerificarContador(void *indice);

#endif 

