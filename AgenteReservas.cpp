/*Nombre del archivo: AgenteReservas.cpp
  Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
  Objetivo: Simular un agente que envia reservas a un controlador mediante pipes
  Fecha de finalizacion: 18/11/2023

  Funciones que lo componen:
    void RecibirRespuesta(string nombreAgente);
    void Sigusr1_handler(int signum);
    void ProcesarSolicitudes(string nombreAgente, string archivoSolicitudes);
    void Recibirhora(string nombreAgente);
    void RecibirRespuesta(string nombreAgente);
    void PrimeraConexion(string nombreAgente, string archivoSolicitudes);
*/
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
#include "Agente.h"


using namespace std;



//-------------------------------------------------------------------------------------------------------
/*
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Parámetros de entrada:
  - signum: Número de la señal recibida, en este caso SIGUSR1.
Parámetros de salida:
  - No hay parámetros de salida, ya que la función es de tipo void.
Función: Manejador de la señal SIGUSR1 que se activa cuando el controlador indica el fin de la simulación. Muestra un mensaje y prepara al agente para terminar su ejecución.
Variables globales utilizadas:
  - No se utilizan variables globales.
*/
void Sigusr1_handler(int signum) {
  cout << "Controlador termina la simulación. Agente terminando..." << endl;
  // Cerrando el file descriptor del pipecrecibe
  close(fd1);
  if (unlink(nomAgenteGlobal.c_str()) == -1) {
      cerr << "Error al eliminar el pipe: " << nomAgenteGlobal << endl;
  } else {
      cout << "Pipe eliminado exitosamente: " << nomAgenteGlobal << endl;
  }
  // Se termina la ejecución del proceso
  exit(0);
}


//-------------------------------------------------------------------------------------------------------
/*
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Parámetros de entrada:
  - nombreAgente: Nombre del agente que procesa las solicitudes.
  - archivoSolicitudes: Nombre del archivo que contiene las solicitudes a procesar.
Variables globales:
  - horaGlobal: Variable que representa la hora actual en la simulación.
  - fd1: Descriptor de archivo del pipe utilizado para comunicarse con el controlador.
Función: Procesa las solicitudes de reserva contenidas en un archivo, enviando cada solicitud al controlador y esperando una respuesta.
*/
void ProcesarSolicitudes(string nombreAgente, string archivoSolicitudes) {
  ifstream archivo(archivoSolicitudes);
  string linea;
  reserva reservas;
  
  // Verificar si el archivo de solicitudes se abrió correctamente
  if (!archivo.is_open()) {
    cerr << "Error al abrir el archivo de solicitudes." << endl;
    return;
  }

  //-- SEÑAL
  signal(SIGUSR1, Sigusr1_handler);
  //--


  // Leer y procesar cada línea del archivo de solicitudes
  while (getline(archivo, linea) && !llegoS) {
    istringstream ss(linea);
    string token;
    // Eliminar espacios en blanco de la línea
    linea.erase(remove_if(linea.begin(), linea.end(), ::isspace), linea.end());

    strcpy(reservas.Agente, nombreAgente.c_str());
    reservas.registro = false;

    if (getline(ss, token, ',')) {
      strncpy(reservas.nomFamilia, token.c_str(), sizeof(reservas.nomFamilia));
      reservas.nomFamilia[sizeof(reservas.nomFamilia) - 1] = '\0'; 
    }

    if (getline(ss, token, ',')) {
      reservas.horaInicio = stoi(token);
      // Descartar la solicitud si la hora de inicio es anterior a la hora global actual
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
      //exit(1);
    }
    // Recibir la respuesta del controlador
     RecibirRespuesta(nombreAgente);
     sleep(2);
  }
    
  
  // Informar que el agente ha terminado de procesar todas las solicitudes
  cout << "Agente " << nombreAgente << " termina." << endl;

  archivo.close();
}
//-------------------------------------------------------------------------------------------------------
/*
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Parámetros de entrada:
  - nombreAgente: Nombre que se le asignara al pipe.
Variables globales:
  - horaGlobal: Variable global que almacenará la hora actual recibida del controlador.
Función: Crea un pipe nominal y espera recibir la hora actual del controlador, almacenándola en la variable global 'horaGlobal'.
*/
void Recibirhora(string nombreAgente)
{
  int fd, nbytes;
  // Configuración de los permisos para el pipe
  mode_t fifo_mode = S_IRUSR | S_IWUSR;
  if (mkfifo(nombreAgente.c_str(), fifo_mode) == -1) {
    perror("mkfifo");
    exit(1);
  }

  // Crear el pipe nominal con el nombre del agente
  if ((fd = open(nombreAgente.c_str(), O_RDONLY)) == -1) {
    perror("open:");
    // Puedes agregar un manejo de error aquí si es necesario
    exit(1);
  }

  // Leer la hora actual del pipe
  nbytes = read(fd, &horaGlobal, sizeof(int));

  // Manejar los diferentes casos de la lectura
  if (nbytes == -1) {
    perror("proceso lector:");
    // Puedes agregar un manejo de error aquí si es necesario
  } else if (nbytes == 0) {
    cout << "nada leido" << endl;
  }
  else{
    cout << "Hora Actual " << horaGlobal << endl;
  }
  
  close(fd);
}


//-------------------------------------------------------------------------------------------------------
/*
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Parámetros de entrada:
  - nombreAgente: Nombre del pipe que espera recibir una respuesta del controlador.
Variables globales:
  - No utiliza variables globales directamente.
Función: Abre un pipe nominal y espera recibir la respuesta del controlador a una solicitud de reserva, mostrando los detalles de la respuesta.
*/
void RecibirRespuesta(string nombreAgente) {
    int fd, nbytes;
    reserva r;

    // Abrir el pipe de forma no bloqueante
    fd = open(nombreAgente.c_str(), O_RDONLY);
    if (fd == -1) {
        perror("open:");
        return;
    }

    // Leer los datos enviados por el controlador
    nbytes = read(fd, &r, sizeof(reserva));
    if (nbytes == -1) {
            perror("read:");
    } else if (nbytes == 0) {
        cout << "No se recibió ninguna respuesta." << endl;
    } else {
        // Mostrar los detalles de la respuesta recibida
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
                if(r.reAgendado) {
                    cout <<" - Reserva negada por tarde para la familia " << r.nomFamilia << ". Pero reajustada a las " << r.horaReAgendada << " horas." << endl;
                } else {
                    cout <<" - Reserva negada por tarde para la familia " << r.nomFamilia << ". No se encontró otro bloque de tiempo disponible." << endl;
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
  close(fd);
}

//-------------------------------------------------------------------------------------------------------
/*
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Parámetros de entrada:
  - nombreAgente: Nombre del agente que establece la primera conexión.
  - archivoSolicitudes: Nombre del archivo que contiene las solicitudes de reserva.
Variables globales:
  - pipenom: Nombre del pipe nominal utilizado para comunicarse con el controlador.
  - fd1: Descriptor de archivo para el pipe utilizado en la comunicación.
  - r: Estructura de tipo reserva utilizada para enviar datos al controlador.
Función: Establece la primera conexión con el controlador enviando una estructura de registro, recibe la hora actual y procesa las solicitudes de reserva.
*/
void PrimeraConexion(string nombreAgente, string archivoSolicitudes) {
  // Crear pipe de escritura
  int creado = 0;
  int bytesEscritos;
  strcpy(r.Agente, nombreAgente.c_str());
  r.registro = true; // Indicar que es un registro de agente
  r.pid = getpid();

  // Intentar abrir el pipe hasta que se logre la conexión
  do {
    fd1 = open(pipenom.c_str(), O_WRONLY);
    if (fd1 == -1) {
      perror("pipe");
      printf(" Se volvera a intentar despues\n");
      sleep(5); // Esperar antes de reintentar
    } else
      creado = 1; // Indicar que el pipe se abrió correctamente
  } while (creado == 0);

  printf("Abrio el pipe, descriptor %d\n", fd1);

  // Se manda la estructura de registro
  bytesEscritos =  write(fd1, &r, sizeof(r));
  //bytesEscritos = write(fd[1], &reservaChar, sizeof(reservaChar));
  if (bytesEscritos == -1) {
    perror("write");
    cerr << "Error al escribir en el pipe" << endl;
    // Aquí puedes manejar el error según tus necesidades
    exit(1);
  } else {
    cout << "Inicia el envío de solicitudes de reserva ... " << endl;
  }

  // Se recibe la hora actual
  Recibirhora(nombreAgente);

  // Llama a procesarSolicitudes para leer y enviar las reservas
  ProcesarSolicitudes(nombreAgente, archivoSolicitudes);

  close(fd1);
  
  if (unlink(nombreAgente.c_str()) == -1) {
      cerr << "Error al eliminar el pipe: " << nombreAgente << endl;
  } else {
      cout << "Pipe eliminado exitosamente: " << nombreAgente << endl;
  }
}


//-------------------------------------------------------------------------------------------------------
/*
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Parámetros de entrada:
  - Argumentos de línea de comandos que incluyen:
    -s: Nombre del agente.
    -a: Archivo de solicitudes.
    -p: Nombre del pipe nominal.
Variables globales:
  - pipenom: Nombre del pipe nominal para la comunicación con el controlador.
Función: Configura el ambiente para el agente, procesa los argumentos de la línea de comandos, establece la señal para manejar y luego realiza la primera conexión.
*/
int main(int argc, char *argv[]) {
  string nombreAgente;
  string archivoSolicitudes;
  // Verificar que se hayan proporcionado la cantidad correcta de argumentos
  if (argc != 7) {
    cerr << "Uso incorrecto. Debe proporcionar los argumentos correctamente."
         << endl;
    return 1;
  }

  // Procesar los argumentos de la línea de comandos
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

  if (unlink(nomAgenteGlobal.c_str()) == -1) {
      cerr << "Error al eliminar el pipe: " << nomAgenteGlobal << endl;
  } else {
      cout << "Pipe eliminado exitosamente: " << nomAgenteGlobal << endl;
  }

  nomAgenteGlobal = nombreAgente;
  
  // Realizar la primera conexión y procesar las solicitudes
  PrimeraConexion(nombreAgente, archivoSolicitudes);

  return 0;
}
