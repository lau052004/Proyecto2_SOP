
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


// Duración en segundos de una "hora"
int segundosPorHora;
char pipeNuevo[30] = "pipe2";
int fd1;
char identificador_emisor2 = '2';
reserva r;
bool terminado = false;

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
  int bytesEscritos;
  int pid, n, creado = 0;
  string familia = "familia perez";
  char nombreAgenteChar[nombreAgente.length() + 1]; 
  strcpy(nombreAgenteChar, nombreAgente.c_str());
  r.registro = true;
  strcpy(r.Agente, nombreAgente.c_str());
  

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

  
  // Se manda la estructura de registro
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

  // Se recibe la hora actual
  recibirhora(nombreAgente);
   r.registro = false;
  strcpy(r.nomFamilia, familia.c_str());
  r.cantFamiliares = 5;
  r.horaInicio = 7;

  // Se inicia con el envío de reservas
  for(int i=0;i<3;i++)
  {
    sleep(3);

    if(i==2)
    {
      r.ultimo = true;
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

  primeraConexion(nombreAgente);

  procesarSolicitudes(nombreAgente, archivoSolicitudes);

  return 0;
}
