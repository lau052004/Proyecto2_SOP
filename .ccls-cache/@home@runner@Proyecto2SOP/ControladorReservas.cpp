#include <chrono>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
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

struct reserva {
  string nomFamilia;
  int cantFamiliares;
};

int horaActual = 7; // Hora actual del parque
vector<reserva> reservas;

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

void realizarAccionesPorHora(int horaActual) {
  // Realizar acciones para cada "hora" de simulación
  // ...
  cout << "Han transcurrido " << horaActual << " horas." << endl;
}

int main(int argc, char *argv[]) {
  comando comandos;
  int horaActual = 7;
  bool seguirSimulacion = true;
  int segundosTranscurridos = 0;
  int fd[2];
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

  // Abrir Pipe de comunicación donde lee
  int nbytes, n, pid;
  char mensaje[30];


  // Creacion del pipe
  mode_t fifo_mode = S_IRUSR | S_IWUSR;

  if (mkfifo(comandos.valor_p.c_str(), fifo_mode) == -1) {
     perror("mkfifo");
     exit(1);
  }

  // Apertura del pipe. 
  if ((fd[0] = open (comandos.valor_p.c_str(), O_RDONLY)) == -1) {
    perror("open:");
    exit(1);
  }  
  printf ("Abrio el pipe\n");

  nbytes = read(fd[0], &n, sizeof(int));
  if (nbytes == -1) {
     perror("proceso lector:");
     exit(1);
  }
  printf ("Lei el numero %d\n", n);
  nbytes = read (fd[0], mensaje, sizeof(mensaje));
  if (nbytes == -1) {
     perror("proceso lector:");
     exit(1);
  }
  
  printf ("Lei el mensaje %s\n", mensaje);
  close(fd[0]);
  
  if (unlink(comandos.valor_p.c_str()) == -1){
    perror("unlink");
    exit(1);
  }  

  // Abrir Pipe donde escribe y manda la hora

  // EMPIEZA LA SIMULACIÓN
  // Simular el tiempo transcurrido y cada vez que transcurra una hora saca
  // personas del parque porque se les acaba su tiempo y autoriza la entrada de
  // las personas que reservaron para la siguiente hora.

  while (seguirSimulacion) {
    // Realizar acciones para cada "hora" de simulación
    realizarAccionesPorHora(horaActual);

    // Esperar durante la duración de una "hora" (10 segundos)
    std::this_thread::sleep_for(std::chrono::seconds(comandos.valor_s));
    segundosTranscurridos += comandos.valor_s;

    if (segundosTranscurridos >= comandos.valor_s) {
      // Ha transcurrido una "hora", reinicia el contador de segundos
      segundosTranscurridos = 0;
      horaActual++;
    }

    if (horaActual > 19) {
      seguirSimulacion = false;
    }
  }

  // Recibir las solicitud de reserva de los agentes y las autoriza o rechaza
  // dependiendo de la cantidad de gente que ya ha reservado en las horas
  // solicitadas. En algunos casos, si es posible, coloca a las personas en un
  // espacio de tiempo distinto al solicitado.

  // Al finalizar el día, emite un reporte acerca de la ocupación del parque

  // Cerrar los pipes
  close(fd[0]);
  close(fd[1]);

  return 0;
}

// probando