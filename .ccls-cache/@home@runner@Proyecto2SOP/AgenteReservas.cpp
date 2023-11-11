
// $ ./controlador –i horaInicio –f horafinal –s segundoshora –t totalpersonas
// –p pipecrecibe
// $./agente –s nombre –a archivosolicitudes –p pipecrecibe
// ./controlador –i 2 -f 3 -s 5 -t 7 -p pipecrecibe
// $./AgenteReservas -s hola -a archivo -p pipecrecibe

#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>

using namespace std;

struct Registro {
  string nombre;
  int hora;
  int personas;
};

// Duración en segundos de una "hora"
const int segundosPorHora = 3600;

void procesarSolicitudes(const string &nombreAgente,
                         const string &archivoSolicitudes,
                         const string &pipeCrecibe) {
  ifstream archivo(archivoSolicitudes);

  if (!archivo.is_open()) {
    cerr << "Error al abrir el archivo de solicitudes." << endl;
    return;
  }

  string linea;
  Registro registro;

  while (getline(archivo, linea)) {
    istringstream ss(linea);
    string token;

    if (getline(ss, token, ',')) {
      registro.nombre = token;
    }

    if (getline(ss, token, ',')) {
      registro.hora = stoi(token);
    }

    if (getline(ss, token, ',')) {
      registro.personas = stoi(token);
    }

    // Aquí puedes enviar la solicitud al controlador y esperar la respuesta
    cout << "Agente: " << nombreAgente << ", Nombre: " << registro.nombre
         << ", Hora: " << registro.hora << ", Personas: " << registro.personas
         << endl;
  }

  archivo.close();
}

void primeraConexion(string nombreAgente){
  // procesarSolicitudes(nombreAgente, archivoSolicitudes, pipeCrecibe);

  // Crear pipe de escritura
  int fd[2], pid, n, creado = 0;

  const char *nombreAgenteChar = nombreAgente.c_str(); // line 62 in AgenteReservas.cpp

  n = getpid();
  // Este trozo de codigo contiene un sleep porque se está tratando de abrir un
  // pipe que crea otro proceso, así da tiempo de que nom2 lo cree si no lo ha
  // creado.

  string pipenom = "pipecrecibe";

  do {
    fd[1] = open(pipenom.c_str(), O_WRONLY);
    if (fd[1] == -1) {
      perror("pipe");
      printf(" Se volvera a intentar despues\n");
      sleep(5);
    } else
      creado = 1;
  } while (creado == 0);

  printf("Abrio el pipe, descriptor %d\n", fd[1]);
  // las llamadas al sistema write, deben validarse, también pueden devolver
  // error
  // El 1 es para incluir el caracter NULL (fin de string) porque strlen no lo
  // hace.
  write(fd[1], nombreAgenteChar, strlen(nombreAgenteChar) + 1);
  printf("Nombre del agente: %s\n", nombreAgenteChar);

  sleep(3);

  char pipe2[30] = "pipe2";

  // El 1 es para incluir el caracter NULL (fin de string) porque strlen no lo
  // hace.
  write(fd[1], pipe2, strlen(pipe2) + 1);
  printf("Nombre del pipe: %s\n", pipe2);

  close(fd[1]);
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

  return 0;
}
