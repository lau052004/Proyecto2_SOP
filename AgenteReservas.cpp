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
#include <cctype>     // Necesario para std::isspace
#include "estructuras.h"

using namespace std;
void recibirRespuesta(string nombreAgente);

// Duración en segundos de una "hora"
int segundosPorHora;
char pipeNuevo[30] = "pipe2";
int fd1;
char identificador_emisor2 = '2';
reserva r;
bool terminado = false;
int horaGlobal;
string pipenom;

void procesarSolicitudes(string nombreAgente, string archivoSolicitudes) {
  ifstream archivo(archivoSolicitudes);

  if (!archivo.is_open()) {
    cerr << "Error al abrir el archivo de solicitudes." << endl;
    return;
  }

  string linea;
  reserva reservas;

  while (getline(archivo, linea)) {
    // Eliminar espacios en blanco de la línea
    linea.erase(remove_if(linea.begin(), linea.end(), ::isspace), linea.end());

    strcpy(reservas.Agente, nombreAgente.c_str());
    reservas.registro = false;
    
    istringstream ss(linea);
    string token;

    if (getline(ss, token, ',')) {
      strncpy(reservas.nomFamilia, token.c_str(), sizeof(reservas.nomFamilia));
      reservas.nomFamilia[sizeof(reservas.nomFamilia) - 1] = '\0'; 
    }

    if (getline(ss, token, ',')) {
      reservas.horaInicio = stoi(token);
      if (reservas.horaInicio < horaGlobal) {
        continue;
      }
    }

    if (getline(ss, token, ',')) {
      reservas.cantFamiliares = stoi(token);
    }
    // Envía la solicitud al controlador
    int bytesEscritos = write(fd1, &reservas, sizeof(reservas));
    if (bytesEscritos == -1) {
      perror("write");
      cerr << "Error al escribir en el pipe" << endl;
      exit(1);
    }
     recibirRespuesta(nombreAgente);
     sleep(2);
  }

  cout << "Agente " << nombreAgente << " termina." << endl;

  archivo.close();
}

void recibirhora(string nombreAgente)
{
  int fd, nbytes;

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

  nbytes = read(fd, &horaGlobal, sizeof(int));

  if (nbytes == -1) {
    perror("proceso lector:");
    // Puedes agregar un manejo de error aquí si es necesario
  } else if (nbytes == 0) {
    cout << "nada leido" << endl;
  }
  else{
    cout << "Hora Actual " << horaGlobal << endl;
  }
}



void recibirRespuesta(string nombreAgente) {
int fd, nbytes;
reserva r;

// Apertura del pipe de forma no bloqueante.
fd = open(nombreAgente.c_str(), O_RDONLY);
if (fd == -1) {
    perror("open:");
    return;
}

nbytes = read(fd, &r, sizeof(reserva));
if (nbytes == -1) {
    if (errno == EAGAIN) {
        // No hay datos disponibles para leer en este momento.
        cout << "No hay datos disponibles para leer." << endl;
    } else {
        perror("read:");
    }
  } else if (nbytes == 0) {
    cout << "Pipe cerrado." << endl;
} else {
    
      cout<<"-------------------------------------------"<<endl;
      cout << "Nombre de la familia: " << r.nomFamilia <<endl; 
      cout << "Cantidad de personas: " << r.cantFamiliares <<endl; 
      cout << "Hora: " << r.horaInicio << endl;
      cout<< "Respuesta: ";
      switch (r.respuesta) {
          case 1:
              cout << " - Reserva aprobada para la familia " << r.nomFamilia << " a las " << r.horaInicio << " horas." << endl;
              break;
          case 2:
              cout << " - Reserva garantizada para otra hora (reajustada a las " << r.horaReAgendada << " horas) para la familia " << r.nomFamilia << "." << endl;
              break;
          case 3:
              if(r.reAgendado)
              {
                cout <<" - Reserva negada por tarde para la familia " << r.nomFamilia << ". Pero se reajustada a las " << r.horaReAgendada<<endl;
              }
              else
              {
                cout <<" - Reserva negada por tarde para la familia " << r.nomFamilia << ". No se encontró otro bloque de tiempo disponible."<<endl;
              }
              break;
          case 4:
              cout << " - Reserva negada para la familia " << r.nomFamilia << ". Debe volver otro día." << endl;
              break;
          default:
              cout << " - Estado de reserva desconocido para la familia " << r.nomFamilia << "." << endl;
              break;
      }
      cout<<"-------------------------------------------"<<endl;
  }
}

void primeraConexion(string nombreAgente, string archivoSolicitudes) {
  // Crear pipe de escritura
  int creado = 0;
  strcpy(r.Agente, nombreAgente.c_str());
  r.registro = true;
  

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
  int bytesEscritos =  write(fd1, &r, sizeof(r));
  //bytesEscritos = write(fd[1], &reservaChar, sizeof(reservaChar));
  if (bytesEscritos == -1) {
    perror("write");
    std::cerr << "Error al escribir en el pipe" << std::endl;
    // Aquí puedes manejar el error según tus necesidades
    exit(1);
  } else {
    std::cout << "Inicia el envío de solicitudes de reserva ... " << std::endl;
  }

  // Se recibe la hora actual
  recibirhora(nombreAgente);

  // Llama a procesarSolicitudes para leer y enviar las reservas
  procesarSolicitudes(nombreAgente, archivoSolicitudes);

  close(fd1);
  
  if (unlink(nombreAgente.c_str()) == -1) {
      cerr << "Error al eliminar el pipe: " << nombreAgente << endl;
  } else {
      cout << "Pipe eliminado exitosamente: " << nombreAgente << endl;
  }
}

int main(int argc, char *argv[]) {
  if (argc != 7) {
    cerr << "Uso incorrecto. Debe proporcionar los argumentos correctamente."
         << endl;
    return 1;
  }

  string nombreAgente;
  string archivoSolicitudes;
  

  for (int i = 1; i < argc; i += 2) {
    if (string(argv[i]) == "-s") {
      nombreAgente = argv[i + 1];
    } else if (string(argv[i]) == "-a") {
      archivoSolicitudes = argv[i + 1];
    } else if (string(argv[i]) == "-p") {
      pipenom = argv[i + 1];
    } else {
      cerr << "Argumento desconocido: " << argv[i] << endl;
      return 1;
    }
  }

  primeraConexion(nombreAgente, archivoSolicitudes);

  return 0;
}
