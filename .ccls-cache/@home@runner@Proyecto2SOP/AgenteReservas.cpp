
// $ ./controlador –i horaInicio –f horafinal –s segundoshora –t totalpersonas
// –p pipecrecibe
// $./agente –s nombre –a archivosolicitudes –p pipecrecibe
// ./controlador –i 2 -f 3 -s 5 -t 7 -p pipecrecibe
// $./AgenteReservas -s hola -a archivo -p pipecrecibe
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
#include "estructuras.h"

using namespace std;

/*struct Registro {
  string agente;
  string nombre;
  int hora;
  int personas;
};*/



// Duración en segundos de una "hora"
int segundosPorHora;
char pipeNuevo[30] = "pipe2";
int fd1;
char identificador_emisor2 = '2';
reserva r;
bool terminado = false;
agenteInfo infoA;

typedef void (*sighandler_t)(int);

void signalHandler (int signum) {
  int bytesEscritos;
  if(terminado == false)
  {
    // Debe hacer el código del manejador de señales
    printf("Desde el Manejador");
    bytesEscritos =  write(fd1, &identificador_emisor2, sizeof(char));

    if (bytesEscritos == -1) {
      perror("write");
      std::cerr << "Error al escribir en el pipe" << std::endl;
      // Aquí puedes manejar el error según tus necesidades
      exit(1);
    } else {
      std::cout << "Identificador: " << identificador_emisor2 << std::endl;
    }

    // Se manda la estructura
    bytesEscritos =  write(fd1, &r, sizeof(r));
    //bytesEscritos = write(fd[1], &reservaChar, sizeof(reservaChar));
    if (bytesEscritos == -1) {
      perror("write");
      std::cerr << "Error al escribir en el pipe" << std::endl;
      // Aquí puedes manejar el error según tus necesidades
      exit(1);
    } else {
      std::cout << "Mandando estructura " << std::endl;
    }
  }
  else {
    char identificador_emisor3 = '3';
    bytesEscritos =  write(fd1, &identificador_emisor3, sizeof(char));

    if (bytesEscritos == -1) {
      perror("write");
      std::cerr << "Error al escribir en el pipe" << std::endl;
      // Aquí puedes manejar el error según tus necesidades
      exit(1);
    } else {
      std::cout << "Identificador: " << identificador_emisor3 << std::endl;
    }

    // Se manda nombre del agente
    bytesEscritos = write(fd1, &infoA, sizeof(infoA));
    if (bytesEscritos == -1) {
      perror("write");
      std::cerr << "Error al escribir en el pipe" << std::endl;
      // Aquí puedes manejar el error según tus necesidades
      exit(1);
    } else {
      std::cout << "Nombre del agente: " << infoA.nombreAgente << std::endl;
    }
  }
}

void procesarSolicitudes(string nombreAgente, string archivoSolicitudes) {
  /*ifstream archivo(archivoSolicitudes);

  if (!archivo.is_open()) {
    cerr << "Error al abrir el archivo de solicitudes." << endl;
    return;
  }

  string linea;
  reserva reservas;

  while (getline(archivo, linea)) {
    istringstream ss(linea);
    string token;

    if (getline(ss, token, ',')) {
      reservas.nomFamilia = token;
    }

    if (getline(ss, token, ',')) {
      reservas.horaInicio = stoi(token);
    }

    if (getline(ss, token, ',')) {
      reservas.cantFamiliares = stoi(token);
    }

    // Aquí puedes enviar la solicitud al controlador y esperar la respuesta
    cout << "Agente: " << nombreAgente << ", Nombre: " << reservas.Agente
         << ", Hora: " << reservas.horaInicio << ", Personas: " << reservas.cantFamiliares
         << endl;
  }

  archivo.close();*/
}

void recibirhora(string nombreAgente)
{
  int fd, n, nbytes;

  mode_t fifo_mode = S_IRUSR | S_IWUSR;
  if (mkfifo(nombreAgente.c_str(), fifo_mode) == -1) {
    perror("mkfifo");
    exit(1);
  }

  // Apertura del pipe.
  if ((fd = open(nombreAgente.c_str(), O_RDONLY)) == -1) {
    perror("open:");
    // Puedes agregar un manejo de error aquí si es necesario
    exit(1);
  }

  nbytes = read(fd, &n, sizeof(int));

  if (nbytes == -1) {
    perror("proceso lector:");
    // Puedes agregar un manejo de error aquí si es necesario
  } else if (nbytes == 0) {
    cout << "nada leido" << endl;
  }
  else{
    cout << "Hora Actual" << n << endl;
  }
}

void primeraConexion(string nombreAgente) {
  // procesarSolicitudes(nombreAgente, archivoSolicitudes, pipeCrecibe);
  // Crear pipe de escritura
  int pid, n, creado = 0;
  string familia = "familia perez";
  char nombreAgenteChar[nombreAgente.length() + 1]; 
  strcpy(nombreAgenteChar, nombreAgente.c_str());

  strcpy(r.Agente, nombreAgente.c_str());
  strcpy(r.nomFamilia, familia.c_str());
  r.cantFamiliares = 5;
  r.horaInicio = 7;


  strcpy(infoA.nombreAgente, nombreAgente.c_str());
  infoA.pid = getpid();

  //n = getpid();
  // Este trozo de codigo contiene un sleep porque se está tratando de abrir un
  // pipe que crea otro proceso, así da tiempo de que nom2 lo cree si no lo ha
  // creado.

  string pipenom = "pipecrecibe";

  do {
    fd1 = open(pipenom.c_str(), O_WRONLY);
    if (fd1 == -1) {
      perror("pipe");
      printf(" Se volvera a intentar despues\n");
      sleep(5);
    } else
      creado = 1;
  } while (creado == 0);

  printf("Abrio el pipe, descriptor %d\n", fd1);
  // las llamadas al sistema write, deben validarse, también pueden devolver
  // error
  // El 1 es para incluir el caracter NULL (fin de string) porque strlen no lo
  // hace.

  // Se manda identificador de nombre del agente
  char identificador_emisor = '1';
  ssize_t bytesEscritos =  write(fd1, &identificador_emisor, sizeof(char));

  if (bytesEscritos == -1) {
    perror("write");
    std::cerr << "Error al escribir en el pipe" << std::endl;
    // Aquí puedes manejar el error según tus necesidades
    exit(1);
  } else {
    std::cout << "Identificador: " << identificador_emisor << std::endl;
  }

  // Se manda nombre del agente
  bytesEscritos = write(fd1, &infoA, sizeof(infoA));
  if (bytesEscritos == -1) {
    perror("write");
    std::cerr << "Error al escribir en el pipe" << std::endl;
    // Aquí puedes manejar el error según tus necesidades
    exit(1);
  } else {
    std::cout << "Nombre del agente: " << nombreAgenteChar << std::endl;
  }

  recibirhora(nombreAgente);

  sleep(3);

  // Se manda identificador de la estructura
  bytesEscritos =  write(fd1, &identificador_emisor2, sizeof(char));

  if (bytesEscritos == -1) {
    perror("write");
    std::cerr << "Error al escribir en el pipe" << std::endl;
    // Aquí puedes manejar el error según tus necesidades
    exit(1);
  } else {
    std::cout << "Identificador: " << identificador_emisor2 << std::endl;
  }

  // Se manda la estructura
  bytesEscritos =  write(fd1, &r, sizeof(r));
  //bytesEscritos = write(fd[1], &reservaChar, sizeof(reservaChar));
  if (bytesEscritos == -1) {
    perror("write");
    std::cerr << "Error al escribir en el pipe" << std::endl;
    // Aquí puedes manejar el error según tus necesidades
    exit(1);
  } else {
    std::cout << "Mandando estructura " << std::endl;
  }

  sleep(3);

  pause();

  sleep(3);

  pause();

  sleep(3);

  // El 1 es para incluir el caracter NULL (fin de string) porque strlen no lo
  // hace.
  // Llamada al sistema write, debe validarse y también puede devolver error
  terminado = true;

  pause();

  close(fd1);
  printf("Se cierra el pipe para escritura\n");
}

int main(int argc, char *argv[]) {
  if (argc != 7) {
    cerr << "Uso incorrecto. Debe proporcionar los argumentos correctamente."
         << endl;
    return 1;
  }

  string nombreAgente;
  string archivoSolicitudes;
  string pipeCrecibe;

  for (int i = 1; i < argc; i += 2) {
    if (string(argv[i]) == "-s") {
      nombreAgente = argv[i + 1];
    } else if (string(argv[i]) == "-a") {
      archivoSolicitudes = argv[i + 1];
    } else if (string(argv[i]) == "-p") {
      pipeCrecibe = argv[i + 1];
    } else {
      cerr << "Argumento desconocido: " << argv[i] << endl;
      return 1;
    }
  }

  signal(SIGUSR1, (sighandler_t) signalHandler);

  primeraConexion(nombreAgente);

  procesarSolicitudes(nombreAgente, archivoSolicitudes);

  return 0;
}
