/*Nombre del archivo: ControladorReservas.cpp
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Objetivo: ---
Fecha de finalizacion: 11/19/2023

Funciones que lo componen:
void manejadorSenales(int signum);
int EsNumero(const string &str);
bool validarComandos(int argc, string argumentos[], comando *comandoIngresado);
void generarInforme();
bool ContadorComando(int argc, string argumentos[], int comando);
bool ValoresCorrectos(int argc, string argumentos[], comando *comandoIngresado);
void enviarResultado(const char *nombreAgente, const reserva &r);
void inicializarHoras();
void verificarComando(int argc, string argumentos[], comando *comandos);
void *incrementarHora(void *indice);
void *verificarContador(void *indice);
void procesarSolicitudes(string nombreAgente, string archivoSolicitudes);
void recibirRespuesta(string nombreAgente);
void recibirhora(string nombreAgente);
void primeraConexion(string nombreAgente, string archivoSolicitudes);
*/

#include "estructuras.h"
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



/*

      VALIDACIÓN DE LOS DATOS DE ENTRADA

*/

//-------------------------------------------------------------------------------------------------------


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
std::map<int, int>
    aceptadaHora; // Para mantener un registro de las personas por hora

/*struct reserva {
  string Agente;
  string nomFamilia;
  int cantFamiliares;
  int horaInicio;
};*/

std::mutex mtx; // Mutex para proteger la variable compartida
int segundosHora;
int horaActual; // Hora actual del parque
int horaInicio;
int horaFinal;
int totalPersonas; // aforo maximo
int cantAgentes = 0;
int posAgenteActual = 0;
vector<reserva> reservas;
vector<string> listaAgentes;
string nombrePipe1;
vector<string> nombrePipesReserva;

int alarmFlag = 0;




/*
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Parámetros de entrada:
                      -  . 
Parámetros de salida:
                      -  . 
Función: ---.
*/
void manejadorSenales(int signum) {
  cout << "--------------------------------------" << endl;
  cout << "Hora Actual " << horaActual << endl;

  // Familias que entran en el parque
  cout << "Familias que entran en el parque:" << endl;
  for (const auto &reserva : reservasPorHora[horaActual]) {
    if (reserva.horaInicio == horaActual ||
        reserva.horaReAgendada ==
            horaActual) { // Solo mostrar al momento de entrar
      cout << "- Familia " << reserva.nomFamilia << " ("
           << reserva.cantFamiliares << " personas)" << endl;
    }
  }
  // Familias actualmente en el parque
  cout << "Familias actualmente en el parque:" << endl;
  for (int i = 7; i < horaActual; ++i) {
    for (const auto &reserva : reservasPorHora[i]) {
      if (i == reserva.horaInicio && i + 2 > horaActual ||
          i == reserva.horaReAgendada &&
              i + 2 > horaActual) { // Solo mostrar si están dentro de su
                                    // ventana de 2 horas
        cout << "- Familia " << reserva.nomFamilia << " ("
             << reserva.cantFamiliares << " personas)" << endl;
      }
    }
  }
  // Familias que salen del parque
  cout << "Familias que salen del parque:" << endl;
  int horaSalida = horaActual - 2;
  for (const auto &reserva : reservasPorHora[horaSalida]) {
    if (reserva.horaInicio == horaSalida ||
        reserva.horaReAgendada ==
            horaSalida) { // Solo mostrar al momento de salir
      cout << "- Familia " << reserva.nomFamilia << " ("
           << reserva.cantFamiliares << " personas)" << endl;
    }
  }

  cout << "--------------------------------------" << endl;
  alarmFlag = 1;
  horaActual++;
}


// FUNCIONES VERIFICACIÓN DE COMANDOS
// -------------------------------------------------------------------------------------------
int EsNumero(const string &str) {
  try {
    size_t pos = 0;
    stod(str, &pos); // Intenta convertir el string a un número de coma flotante
    // Verifica si se consumió todo el string, lo que indica que es un número
    // válido
    return pos == str.length() ? 1 : 0;
  } catch (const std::invalid_argument &) {
    // La conversión arroja una excepción si el string no es un número válido
    return 0;
  }
}

bool validarComandos(int argc, string argumentos[], comando *comandoIngresado) {
  // Itera sobre los argumentos (saltando de 2 en 2) para validar los comandos.
  for (int i = 1; i < argc; i = i + 2) {
    // Compara el comando actual con los comandos válidos almacenados en la
    // estructura.
    if (argumentos[i] == comandoIngresado->comando_i ||
        argumentos[i] == comandoIngresado->comando_f ||
        argumentos[i] == comandoIngresado->comando_s ||
        argumentos[i] == comandoIngresado->comando_t ||
        argumentos[i] == comandoIngresado->comando_p) {
      return true;
    } else {
      return false;
    }
  }

  return true;
}

void generarInforme() {
  // Calcular personas totales por hora desde las 7 hasta las 19
  std::map<int, int> personasTotalesPorHora;
  for (int hora = horaInicio; hora < horaFinal; ++hora) {
      personasTotalesPorHora[hora] = 0;  // Inicializar con 0
  }
  for (const auto &par : reservasPorHora) {
      if (par.first >= 7 && par.first < 19) {
          for (const auto &reserva : par.second) {
              personasTotalesPorHora[par.first] += reserva.cantFamiliares;
          }
      }
  }
  // Encontrar la hora con mayor y menor número de personas
  int maxPersonas = 0, minPersonas = std::numeric_limits<int>::max();
  std::vector<int> horasPico, horasMenosConcurridas;
  
  for (const auto &hora : personasTotalesPorHora) {
      if (hora.second > maxPersonas) {
          maxPersonas = hora.second;
          horasPico.clear();
          horasPico.push_back(hora.first);
      } else if (hora.second == maxPersonas) {
          horasPico.push_back(hora.first);
      }
  
      if (hora.second < minPersonas) {
          minPersonas = hora.second;
          horasMenosConcurridas.clear();
          horasMenosConcurridas.push_back(hora.first);
      } else if (hora.second == minPersonas) {
          horasMenosConcurridas.push_back(hora.first);
      }
  }
  
  // Mostrar el informe
  cout << "Informe del Parque:\n";
  cout << "Solicitudes Negadas: " << solicitudesNegadas << "\n";
  cout << "Solicitudes Aceptadas: " << solicitudesAceptadas << "\n";
  cout << "Solicitudes Reprogramadas: " << solicitudesReprogramadas << "\n";
  cout << "Horas Pico: ";
  for (int hora : horasPico) {
    cout << hora << " ";
  }
  cout << "\nHoras con Menor Número de Personas: ";
  for (int hora : horasMenosConcurridas) {
    cout << hora << " ";
  }

  cout << "\nSolicitudes Aceptadas por Hora:\n";
  for (const auto &hora : aceptadaHora) {
    cout << "Hora " << hora.first << ": " << hora.second
         << " solicitudes aceptadas\n";
  }
}

bool ContadorComando(int argc, string argumentos[], int comando) {
  int contador = 0; // contador para contar las ocurrencias del comando
  // Itera sobre los argumentos (saltando de 2 en 2, debido a que va el comando
  // seguido del valor) para buscar el comando.
  for (int i = 1; i < argc; i = i + 2) {
    if (argumentos[i] == argumentos[comando]) {
      contador++;
    }
  }
  // Verifica si el contador es exactamente 1 (el comando aparece una vez).
  if (contador != 1) {
    return false; // El comando no aparece exactamente una vez.
  } else {
    return true; // El comando aparece exactamente una vez.
  }
}

bool ValoresCorrectos(int argc, string argumentos[],
                      comando *comandoIngresado) {
  int num;
  bool inicial = false, final = false;
  // Itera sobre los argumentos (saltando de 2 en 2) para verificar los valores
  // asociados a los comandos.
  for (int i = 1; i < argc; i = i + 2) {
    if (argumentos[i] == comandoIngresado->comando_i) {
      num = EsNumero(argumentos[i + 1]);
      if (num != 1) { // Retorna false si el valor no es un número válido.
        return false;
      } else {
        if (stoi(argumentos[i + 1]) >= 7 && stoi(argumentos[i + 1]) < 19) {
          if (final == true) {
            if (stoi(argumentos[i + 1]) < comandoIngresado->valor_f) {
              comandoIngresado->valor_i = stoi(
                  argumentos[i + 1]); // Almacena el valor convertido a entero.
            } else {
              cout << "La hora inicial debe ser menor a la final" << endl;
              return false;
            }
          } else {
            inicial = true;
            comandoIngresado->valor_i = stoi(
                argumentos[i + 1]); // Almacena el valor convertido a entero.
          }
        } else {
          cout << "La hora incial no corresponde al horario del parque."
               << endl;
          return false;
        }
      }

      // Verifica si el valor asociado al comando_n es un número válido.
    } else if (argumentos[i] == comandoIngresado->comando_f) {
      num = EsNumero(argumentos[i + 1]);
      if (num != 1) {
        return false;
      } else {
        if (stoi(argumentos[i + 1]) > 7 && stoi(argumentos[i + 1]) <= 19) {
          if (inicial == true) {
            if (stoi(argumentos[i + 1]) > comandoIngresado->valor_i) {
              comandoIngresado->valor_f = stoi(
                  argumentos[i + 1]); // Almacena el valor convertido a entero.
            } else {
              cout << "La hora final debe ser mayor a la inicial" << endl;
              return false;
            }
          } else {
            final = true;
            comandoIngresado->valor_f = stoi(
                argumentos[i + 1]); // Almacena el valor convertido a entero.
          }
        } else {
          cout << "La hora final no corresponde al horario del parque." << endl;
          return false;
        }
      }
    } else if (argumentos[i] == comandoIngresado->comando_s) {
      num = EsNumero(argumentos[i + 1]);
      if (num != 1) {
        return false;
      } else {
        comandoIngresado->valor_s = stoi(argumentos[i + 1]);
      }
    } else if (argumentos[i] == comandoIngresado->comando_t) {
      num = EsNumero(argumentos[i + 1]);
      if (num != 1) {
        return false;
      } else {
        comandoIngresado->valor_t = stoi(argumentos[i + 1]);
      }
    } else if (argumentos[i] == comandoIngresado->comando_p) {
      comandoIngresado->valor_p = argumentos[i + 1];
    }
  }
  return true; // Todos los valores son válidos.
}

bool verificarComando(int argc, string argumentos[], comando *comandos) {
  bool cantidad_correcta, comandos_correctos, valores_correctos;

  // Verificación de la cantidad de argumentos
  if (argc != 11) {
    printf("Cantidad de argumentos ERRONEA \n");
    printf("Uso: $./controlador –i horaInicio –f horafinal –s segundoshora –t "
           "totalpersonas –p pipecrecibe \n");
    return false;
  }

  // Verificación de los comandos
  comandos_correctos = validarComandos(argc, argumentos, comandos);

  if (!comandos_correctos) {
    printf("Al menos uno de los comandos ingresados no es válido\n");
    return false;
  }

  // Verificar si cada comando está una vez
  for (int i = 1; i < argc; i = i + 2) {
    cantidad_correcta = ContadorComando(argc, argumentos, i);
    if (cantidad_correcta == false) {
      printf("Cantidad de comandos incorrecta: Se repite un comando\n");
      return false;
    }
  }
  // Verificar que los datos ingresados para cada comando sean válidos
  valores_correctos = ValoresCorrectos(argc, argumentos, comandos);

  if (valores_correctos == false) {
    printf("Los datos ingresados para al menos un comando son erroneos \n");
    return false;
  }

  // Imprimir la estructura
  /*cout << "Comando i: " << comandos->comando_i
       << ", Valor i: " << comandos->valor_i << endl;
  cout << "Comando f: " << comandos->comando_f
       << ", Valor f: " << comandos->valor_f << endl;
  cout << "Comando s: " << comandos->comando_s
       << ", Valor s: " << comandos->valor_s << endl;
  cout << "Comando t: " << comandos->comando_t
       << ", Valor t: " << comandos->valor_t << endl;
  cout << "Comando p: " << comandos->comando_p
       << ", Valor p: " << comandos->valor_p << endl;*/

  return true;
}

// -------------------------------------------------------------- HILOS

// Map global que tiene como clave la hora y como valor un vector de reservas.

void enviarResultado(const char *nombreAgente, const reserva &r) {
  int fd;
  while ((fd = open(nombreAgente, O_WRONLY)) == -1) {
    perror("pipe");
    cout << "Se volverá a intentar después" << endl;
    sleep(1);
  }
  // cout << "Se abrió el pipe para escribir, descriptor " << fd << endl;

  // Enviar la estructura reserva.
  int bytesEscritos = write(fd, &r, sizeof(reserva));
  if (bytesEscritos == -1) {
    perror("write");
    cerr << "Error al escribir en el pipe" << endl;
    exit(1);
  } else {
    cout << "Resultado de la reserva enviado" << endl << endl;
  }
  close(fd);
  // cout << "Se cierra el pipe para escritura" << endl;
}

// Función para inicializar las horas en el map.
void inicializarHoras() {
  for (int hora = horaInicio; hora <= horaFinal; ++hora) {
    reservasPorHora[hora] = vector<reserva>();
  }
}

void verificarReservas(reserva &r) {
  cout << endl;
  cout << "Verificando reserva..." << endl;
  cout << "Agente: " << r.Agente << endl;
  cout << "Mombre de Familia: " << r.nomFamilia << endl;
  cout << "Cantidad: " << r.cantFamiliares << endl;
  cout << "Hora: " << r.horaInicio << endl;

  auto personasEnHora = [&](int hora) {
    int totalPersonasHora = 0;
    for (const auto &reserva : reservasPorHora[hora]) {
      totalPersonasHora += reserva.cantFamiliares;
    }
    return totalPersonasHora;
  };

  if (r.horaInicio < horaActual) {
    r.respuesta = 3; // Intentar encontrar otro bloque de tiempo disponible.
    bool reservaAlternativa = false;
    for (int i = horaActual + 1; i <= horaFinal - 2; ++i) {
      if (i > 17)
        break; // Asegurarse de que no se exceda la hora límite de 17.
      bool bloqueDisponible = true;
      for (int j = i; j < i + 2; ++j) {
        if (j > horaFinal - 2 ||
            personasEnHora(j) + r.cantFamiliares > totalPersonas) {
          bloqueDisponible = false;
          break;
        }
      }
      if (bloqueDisponible) {
        r.horaReAgendada = i; // Actualizar la hora de inicio a la nueva hora.
        for (int j = i; j < i + 2; ++j) {
          reservasPorHora[j].push_back(r);
        }
        r.reAgendado = true;
        reservaAlternativa = true;
        solicitudesReprogramadas++;
        break;
      }
    }
    if (!reservaAlternativa) {
      solicitudesNegadas++;
    }
  } else if (r.horaInicio > horaFinal - 2 || r.horaInicio > 17) {
    r.respuesta = 4; // Reserva negada, debe volver otro día.
    solicitudesNegadas++;
  } else {
    bool espacioDisponible = true;
    for (int i = r.horaInicio; i < r.horaInicio + 2; ++i) {
      if (i > horaFinal - 2 ||
          personasEnHora(i) + r.cantFamiliares > totalPersonas) {
        espacioDisponible = false;
        break;
      }
    }

    if (espacioDisponible) {
      r.respuesta = 1;              // Reserva aprobada.
      aceptadaHora[r.horaInicio]++; // Actualizar el contador de solicitudes
                                    // aceptadas para esta hora
      solicitudesAceptadas++;
      for (int i = r.horaInicio; i < r.horaInicio + 2; ++i) {
        reservasPorHora[i].push_back(r);
      }
    } else {
      bool reservaAlternativa = false;
      for (int i = horaActual; i <= horaFinal - 2; ++i) {
        if (i > 17)
          break;
        bool bloqueDisponible = true;
        for (int j = i; j < i + 2; ++j) {
          if (j > horaFinal - 2 ||
              personasEnHora(j) + r.cantFamiliares > totalPersonas) {
            bloqueDisponible = false;
            break;
          }
        }
        if (bloqueDisponible) {
          r.respuesta = 2;      // Reserva garantizada para otra hora.
          r.horaReAgendada = i; // Actualizar la hora de inicio a la nueva hora.
          solicitudesReprogramadas++;
          for (int j = i; j < i + 2; ++j) {
            reservasPorHora[j].push_back(r);
          }
          reservaAlternativa = true;
          break;
        }
      }
      if (!reservaAlternativa) {
        r.respuesta = 4; // Reserva negada, debe volver otro día.
        solicitudesNegadas++;
      }
    }
  }
  enviarResultado(r.Agente, r);
}

//--------------------------------------- Verificacion
void enviarHora(char *nombrePipe) {
  int fd, bytesEscritos, creado = 0;

  do {
    fd = open(nombrePipe, O_WRONLY);
    if (fd == -1) {
      perror("pipe");
      printf(" Se volvera a intentar despues\n");
      sleep(3);
    } else
      creado = 1;
  } while (creado == 0);

  // printf("Abrio el pipe de escritura %d\n", fd);

  // Escribir hora en el pipe
  bytesEscritos = write(fd, &horaActual, sizeof(int));

  if (bytesEscritos == -1) {
    perror("write");
    cerr << "Error al escribir en el pipe" << endl << endl;
    exit(1);
  } else {
    cout << "Hora enviada: " << horaActual << std::endl;
  }
}
//--------------------------------------------

void *incrementarHora(void *indice) {
  signal(SIGALRM, manejadorSenales);
  while (true) {
    alarmFlag = 0;
    alarm(segundosHora);
    while (alarmFlag != 1) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (horaActual > 19 || horaActual > horaFinal) {
      cout << "Hilo 1: Contador alcanzó " << horaFinal
           << ". Terminando los hilos." << endl;
      generarInforme();
      break;
    }
  }
  exit(0);
}

void *verificarContador(void *indice) {
  int nbytes;
  char n = '0';
  char mensaje[30];
  int fd;
  char reserva[100];
  struct reserva r;

  // Creacion del pipe
  mode_t fifo_mode = S_IRUSR | S_IWUSR;
  if (mkfifo(nombrePipe1.c_str(), fifo_mode) == -1) {
    perror("mkfifo");
    exit(1);
  }

  // Apertura del pipe.
  if ((fd = open(nombrePipe1.c_str(), O_RDONLY)) == -1) {
    perror("open:");
    // Puedes agregar un manejo de error aquí si es necesario
    exit(1);
  }

  while (true) {
    // Sección crítica protegida por el mutex
    {
      std::lock_guard<std::mutex> lock(mtx);
      // std::cout << "Hilo 2: Contador = " << horaActual << std::endl;
    }

    nbytes = read(fd, &r, sizeof(r));

    if (nbytes == -1) {
      perror("pipe:");
      exit(0);
    } else if (nbytes == 0) {
      continue;
    }

    if (r.registro == true) {
      printf("Nombre agente: %s\n", r.Agente);
      enviarHora(r.Agente);
      listaAgentes.push_back(r.Agente);
      continue;
    } else if (r.registro == false) {
      verificarReservas(r);
      continue;
    }
  }

  // Cerrar el pipe después de salir del bucle
  close(fd);

  pthread_exit(NULL);
}

//--------------------------------------- MAIN

int main(int argc, char *argv[]) {
  comando comandos;
  bool comandosAceptados;
  
  inicializarHoras();
  // VERIFICACIÓN DE COMANDOS
  // Convierte los argumentos a string
  string arguments[argc];
  for (int i = 0; i < argc; i++) {
    arguments[i] = argv[i]; // Convierte el elemento argv[i] a std::string
  }

  /*for (int i = 0; i < argc; i++) {
    cout << "Argumento " << i << ": " << arguments[i] << std::endl;
  }*/

  comandosAceptados = verificarComando(argc, arguments, &comandos);

  if(comandosAceptados==true)
  {
    horaActual = comandos.valor_i;
    horaInicio=horaActual;
    segundosHora = comandos.valor_s;
    horaFinal = comandos.valor_f;
    totalPersonas = comandos.valor_t;

    // ---------------------- INICIANDO EL PROGRAMA
    // signal (SIGALRM, (sighandler_t)signalHandler);
    // signal (SIGALRM, (sighandler_t)signalHandler); // Establece signalHandler
    // como el manejador para SIGALRM
    unlink(comandos.valor_p.c_str());
    nombrePipe1 = comandos.valor_p;
    pthread_t threads[2];

    if (pthread_create(&threads[0], NULL, incrementarHora, NULL) != 0) {
      std::cerr << "Error al crear el hilo 1." << std::endl;
      return 1;
    }

    if (pthread_create(&threads[1], NULL, verificarContador, NULL) != 0) {
      std::cerr << "Error al crear el hilo 2." << std::endl;
      return 1;
    }

    // Esperar a que ambos hilos terminen
    void *returnValue;

    if (pthread_join(threads[0], &returnValue) != 0) {
      fprintf(stderr, "Error al unirse al hilo 1.\n");
      return 1;
    }

    if (pthread_join(threads[1], &returnValue) != 0) {
      fprintf(stderr, "Error al unirse al hilo 2.\n");
      return 1;
    }

    cout << "Programa terminado." << std::endl;

    return 0;
  }
}

// probando
