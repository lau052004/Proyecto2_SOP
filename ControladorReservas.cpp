#include <chrono>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <sstream>
#include "estructuras.h"

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

/*struct reserva {
  string Agente;
  string nomFamilia;
  int cantFamiliares;
  int horaInicio;
};*/

std::mutex mtx; // Mutex para proteger la variable compartida
int horaActual; // Hora actual del parque
int segundosHora;
int horaFinal;
int totalPersonas;
int cantAgentes=0;
int posAgenteActual=0;
vector<reserva> reservas;
string nombrePipe1;
vector <string> nombrePipesReserva;
vector <agenteInfo> agentes;


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

void verificarComando(int argc, string argumentos[], comando *comandos) {
  bool cantidad_correcta, comandos_correctos, valores_correctos;

  // Verificación de la cantidad de argumentos
  if (argc != 11) {
    printf("Cantidad de argumentos ERRONEA \n");
    printf("Uso: $./controlador –i horaInicio –f horafinal –s segundoshora –t "
           "totalpersonas –p pipecrecibe \n");
    return;
  }

  // Verificación de los comandos
  comandos_correctos = validarComandos(argc, argumentos, comandos);

  if (!comandos_correctos) {
    printf("Al menos uno de los comandos ingresados no es válido\n");
    return;
  }

  // Verificar si cada comando está una vez
  for (int i = 1; i < argc; i = i + 2) {
    cantidad_correcta = ContadorComando(argc, argumentos, i);
    if (cantidad_correcta == false) {
      printf("Cantidad de comandos incorrecta: Se repite un comando\n");
      return;
    }
  }
  // Verificar que los datos ingresados para cada comando sean válidos
  valores_correctos = ValoresCorrectos(argc, argumentos, comandos);

  if (valores_correctos == false) {
    printf("Los datos ingresados para al menos un comando son erroneos \n");
    return;
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
}

// -------------------------------------------------------------- HILOS

void verificarReservas(reserva r) 
{ 
  cout << "Verificando reserva..." << endl; 
  cout << "Agente: " << r.Agente <<endl; 
  cout << "nombreFamilia: " << r.nomFamilia <<endl; 
  cout << "cant: " << r.cantFamiliares <<endl; 
  cout << "hora: " << r.horaInicio << endl; 
}

void enviarHora(char* nombrePipe) {
  int fd, bytesEscritos;

  fd = open(nombrePipe, O_WRONLY);
  if (fd == -1) {
    perror("proceso escritor:");
  }

  printf("Abrio el pipe de escritura %d\n", fd);

  // Escribir hora en el pipe
  bytesEscritos = write(fd, &horaActual, sizeof(int));

  if (bytesEscritos == -1) {
    perror("write");
    std::cerr << "Error al escribir en el pipe" << std::endl;
    // Aquí puedes manejar el error según tus necesidades
    exit(1);
  } else {
    std::cout << "Hora enviada: " << horaActual << std::endl;
  }
}

void *incrementarHora(void *indice) {
  while (true) {
    sleep(segundosHora); // usleep(3000000); en sistemas que no tienen sleep

    // Sección crítica protegida por el mutex
    std::lock_guard<std::mutex> lock(mtx);
    horaActual++;

    std::cout << "Hilo 1: Hora Actual = " << horaActual << std::endl;

    // Llamar a la función hacerAlgo cada vez que se incrementa el contador
    //verificarReservas();

    // Verificar si el contador llega a 10
    if (horaActual >= 19 || horaActual >= horaFinal) {
      break; // Salir del bucle si el contador llega a 10
    }
  }
  pthread_exit(NULL);
}

void *verificarContador(void *indice) {
  int nbytes;
  char n = '0';
  char mensaje[30];
  int fd;
  char reserva[100];
  struct reserva r;
  struct agenteInfo infoA;

  // Creacion del pipe
  mode_t fifo_mode = S_IRUSR | S_IWUSR;
  if (mkfifo(nombrePipe1.c_str(), fifo_mode) == -1) {
    perror("mkfifo");
    exit(1);
  }

  // Apertura del pipe.
  if ((fd = open(nombrePipe1.c_str(), O_RDONLY /*| O_NONBLOCK*/)) == -1) {
    perror("open:");
    // Puedes agregar un manejo de error aquí si es necesario
    exit(1);
  }

    while (true) {
        n = '0';

        // Sección crítica protegida por el mutex
        {
            std::lock_guard<std::mutex> lock(mtx);
            // std::cout << "Hilo 2: Contador = " << horaActual << std::endl;
        }

        if (horaActual >= 19 || horaActual >= horaFinal) {
            std::cout << "Hilo 2: Contador alcanzó 20. Terminando los hilos." << std::endl;
            break;
        }

        sleep(1);
        nbytes = read(fd, &n, sizeof(char));

        if (nbytes == -1) {
            perror("proceso lector id 1:");
            // Puedes agregar un manejo de error aquí si es necesario
        } else if (nbytes == 0) {
            // Verificar si el contador llega a 20
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (horaActual >= 19 || horaActual >= horaFinal) {
                    std::cout << "Hilo 2: Contador alcanzó 20. Terminando los hilos." << std::endl;
                    break;
                }
            }
            // ./controlador -i 2 -f 3 -s 5 -t 7 -p pipecrecibe
            sleep(1);
            continue;
        } else {
            // printf("Reabrio el pipe\n");
            cout << "identificador: " << n << endl;

            if (n == '1') {
                nbytes = read(fd, &infoA, sizeof(infoA));
                if (nbytes == -1) {
                    perror("proceso lector id 2:");
                    // Puedes agregar un manejo de error aquí si es necesario
                } else {
                    // se lee nombre del agente
                    printf("Nombre agente: %s\n", infoA.nombreAgente);
                    printf("Pid agente: %d\n", infoA.pid);
                    agentes.push_back(infoA);
                    cantAgentes++;
                    enviarHora(infoA.nombreAgente);
                    sleep(3);
                    // Le digo a ese agente que me puede mandar reservas
                    // Nota: No se envía señal aquí para evitar acumulación
                    continue;
                }
            } else if (n == '2') {

              sleep(3);
                // se lee estructura de la reserva
                nbytes = read(fd, &r, sizeof(r));

                if (nbytes == -1) {
                    perror("proceso lector id 3:");
                    // Puedes agregar un manejo de error aquí si es necesario
                } else {
                    verificarReservas(r);

                    // Se verifica la cantidad de procesos que hay en el vector y el último proceso al que se le permitió enviar información
                    // Se le da permiso al siguiente agente en el vector de agentes
                    cout << "posagente" << posAgenteActual << endl;
                    if (posAgenteActual == cantAgentes - 1) {
                        cout << "posagente condicion" << posAgenteActual << endl;
                        posAgenteActual = 0;
                    } else {
                        cout << "kill de n=2" << endl;
                        posAgenteActual++;
                        cout << "posagente kill" << posAgenteActual << endl;
                    }

                    // Envía la señal solo si hay agentes en el vector
                    if (!agentes.empty()) {
                        if (kill(agentes[posAgenteActual].pid, SIGUSR1) != 0) {
                            perror("kill");
                            exit(1);
                        }
                    }

                    cout << "salida posagente" << posAgenteActual << endl;
                }
                continue;
            } else if (n == '3') {
                // Quiere decir que el agente está avisando que terminó su ejecución por lo que debe ser eliminado de la lista de agentes
                nbytes = read(fd, &infoA, sizeof(infoA));

                if (nbytes == -1) {
                    perror("proceso lector id 4:");
                    // Puedes agregar un manejo de error aquí si es necesario
                } else {
                    for (int i = 0; i < agentes.size(); i++) {
                        if (agentes[i].pid == infoA.pid) {
                            cout << "eliminando: " << agentes[i].pid << endl;
                            agentes.erase(agentes.begin() + i);
                            cantAgentes--;
                            cout << "cant agentes" << cantAgentes << endl;
                            posAgenteActual--;

                            cout << "pos agentes" << posAgenteActual << endl;
                            // Ajustar posAgenteActual si es necesario
                            if (posAgenteActual >= cantAgentes) {
                                posAgenteActual = 0;
                            }
                            break; // Importante salir del bucle después de eliminar el agente
                        }
                    }
                }
                continue;
            } else {
                std::lock_guard<std::mutex> lock(mtx);
                if (horaActual >= 19 || horaActual >= horaFinal) {
                    std::cout << "Hilo 2: Contador alcanzó 20. Terminando los hilos." << std::endl;
                    break;
                }
                continue;
            }
        }
        sleep(1);
        continue;
    }

    // Cerrar el pipe después de salir del bucle
    close(fd);

    pthread_exit(NULL);
}


//--------------------------------------- MAIN



int main(int argc, char *argv[]) {
  comando comandos;

  // VERIFICACIÓN DE COMANDOS
  // Convierte los argumentos a string
  string arguments[argc];
  for (int i = 0; i < argc; i++) {
    arguments[i] = argv[i]; // Convierte el elemento argv[i] a std::string
  }

  for (int i = 0; i < argc; i++) {
    cout << "Argumento " << i << ": " << arguments[i] << std::endl;
  }

  verificarComando(argc, arguments, &comandos);

  horaActual = comandos.valor_i;
  segundosHora = comandos.valor_s;
  horaFinal = comandos.valor_f;
  totalPersonas = comandos.valor_t;

  // ---------------------- INICIANDO EL PROGRAMA
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

  std::cout << "Programa terminado." << std::endl;

  return 0;
}

// probando
