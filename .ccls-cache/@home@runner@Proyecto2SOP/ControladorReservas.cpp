#include <chrono>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <vector>
#include <mutex>

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

struct reserva {
  string nomFamilia;
  int cantFamiliares;
};


std::mutex mtx;  // Mutex para proteger la variable compartida
int horaActual = 7; // Hora actual del parque
vector<reserva> reservas;
string nombrePipe1;

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
  // Itera sobre los argumentos (saltando de 2 en 2) para verificar los valores
  // asociados a los comandos.
  for (int i = 1; i < argc; i = i + 2) {
    if (argumentos[i] == comandoIngresado->comando_i) {
      num = EsNumero(argumentos[i + 1]);
      if (num != 1) { // Retorna false si el valor no es un número válido.
        return false;
      } else {
        comandoIngresado->valor_i =
            stoi(argumentos[i + 1]); // Almacena el valor convertido a entero.
      }
      // Verifica si el valor asociado al comando_n es un número válido.
    } else if (argumentos[i] == comandoIngresado->comando_f) {
      num = EsNumero(argumentos[i + 1]);
      if (num != 1) {
        return false;
      } else {
        comandoIngresado->valor_f = stoi(argumentos[i + 1]);
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
  cout << "Comando i: " << comandos->comando_i
       << ", Valor i: " << comandos->valor_i << endl;
  cout << "Comando f: " << comandos->comando_f
       << ", Valor f: " << comandos->valor_f << endl;
  cout << "Comando s: " << comandos->comando_s
       << ", Valor s: " << comandos->valor_s << endl;
  cout << "Comando t: " << comandos->comando_t
       << ", Valor t: " << comandos->valor_t << endl;
  cout << "Comando p: " << comandos->comando_p
       << ", Valor p: " << comandos->valor_p << endl;
}


// -------------------------------------------------------------- hilos

void verificarReservas()
{
  cout << "Verificando reserva..." << endl;
}

void *incrementarHora(void *segundosHora) {
  while(true)
  {
    sleep(3);  // usleep(3000000); en sistemas que no tienen sleep

    // Sección crítica protegida por el mutex
    std::lock_guard<std::mutex> lock(mtx);
    horaActual++;

    std::cout << "Hilo 1: Contador = " << horaActual << std::endl;

    // Llamar a la función hacerAlgo cada vez que se incrementa el contador
    verificarReservas();

    // Verificar si el contador llega a 10
    if (horaActual >= 20) {
        break;  // Salir del bucle si el contador llega a 10
    }
  }
  pthread_exit(NULL);
}

void *verificarContador(void *indice) {
  int nbytes, n;
  char mensaje[30];
  int fd[2];
  bool lee = false;

  // Creacion del pipe
  mode_t fifo_mode = S_IRUSR | S_IWUSR;
  if (mkfifo(nombrePipe1.c_str(), fifo_mode) == -1) {
    perror("mkfifo");
    exit(1);
  }

  // Apertura del pipe.
  if ((fd[0] = open(nombrePipe1.c_str(), O_RDONLY)) == -1) {
    perror("open:");
    // Puedes agregar un manejo de error aquí si es necesario
    exit(1);
  }

  while (true) {
    //std::cout << "Hilo 2: Contador = " << horaActual << std::endl;

    nbytes = read(fd[0], &n, sizeof(int));

    if (nbytes == -1) {
      perror("proceso lector:");
      // Puedes agregar un manejo de error aquí si es necesario
    }
    else if (nbytes == 0) {
      // Verificar si el contador llega a 20
          if (horaActual >= 20) {
            std::cout << "Hilo 2: Contador alcanzó 20. Terminando los hilos." << std::endl;
            break;
          }
      // ./controlador -i 2 -f 3 -s 5 -t 7 -p pipecrecibe
          sleep(1);
    }
    else{
      printf("Reabrio el pipe\n");

        printf("Lei el numero %d\n", n);

        nbytes = read(fd[0], mensaje, sizeof(mensaje));
        if (nbytes == -1) {
          perror("proceso lector:");
          // Puedes agregar un manejo de error aquí si es necesario
          break;
        }

        printf("Lei el mensaje %s\n", mensaje);

        if (horaActual >= 20) {
          std::cout << "Hilo 2: Contador alcanzó 20. Terminando los hilos." << std::endl;
          break;
        }
        lee = false;
        continue;
      }
    }

  // Cerrar el pipe después de salir del bucle
  close(fd[0]);

  pthread_exit(NULL);
}




//--------------------------------------- MAIN


void realizarAccionesPorHora(int horaActual) {
  // Realizar acciones para cada "hora" de simulación
  // ...
  cout << "Han transcurrido " << horaActual << " horas." << endl;
}

int main(int argc, char *argv[]) {
  comando comandos;
  bool seguirSimulacion = true;
  int segundosTranscurridos = 0;
  char buffer[256];
  

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

  // ---------------------- INICIANDO EL PROGRAMA
  nombrePipe1 = comandos.valor_p;
  pthread_t threads[2];


  if (pthread_create(&threads[0], NULL, incrementarHora, reinterpret_cast<void*>(comandos.valor_s)) != 0) {
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

// probando