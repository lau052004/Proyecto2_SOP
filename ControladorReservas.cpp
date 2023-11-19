/*Nombre del archivo: ControladorReservas.cpp
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Objetivo: ---
Fecha de finalizacion: 11/19/2023

Funciones que lo componen:
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
*/

#include "estructuras.h"
#include "Controlador.h"
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
/*
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Parámetros de entrada:
                      - No hay parámetros de entrada.
Parámetros de salida:
                      - No hay parámetros de salida, ya que la función es de tipo void.
Función: Imprime el estado actual del parque, mostrando las familias que entran, están actualmente en el parque y salen del parque.
Variables globales utilizadas:
  - horaActual: Hora actual en la simulación del parque.
  - reservasPorHora: Mapa que almacena las reservas organizadas por hora.
*/
void MostrarEstadoDelParque() {

  cout << endl << "REPORTE : " << horaActual-1 << endl;
    int horaSalida = horaActual - 3;
   
    //cout << "Hora Actual " << horaActual << endl;

    // Familias que entran en el parque
    cout << "Familias que entran en el parque:" << endl;
    for (const auto &reserva : reservasPorHora[horaActual-1]) {
        if (reserva.horaInicio == horaActual-1 || reserva.horaReAgendada == horaActual-1) {
            cout << "- Familia " << reserva.nomFamilia << " (" << reserva.cantFamiliares << " personas)" << endl;
        }
    }

    // Familias actualmente en el parque
    cout << "Familias actualmente en el parque:" << endl;
    for (int i = 7; i < horaActual-1; ++i) {
        for (const auto &reserva : reservasPorHora[i]) {
            if ((i == reserva.horaInicio || i == reserva.horaReAgendada) && i + 2 > horaActual) {
                cout << "- Familia " << reserva.nomFamilia << " (" << reserva.cantFamiliares << " personas)" << endl;
            }
        }
    }

    // Familias que salen del parque
    cout << "Familias que salen del parque:" << endl;
    for (const auto &reserva : reservasPorHora[horaSalida]) {
        if (reserva.horaInicio == horaSalida || reserva.horaReAgendada == horaSalida) {
            cout << "- Familia " << reserva.nomFamilia << " (" << reserva.cantFamiliares << " personas)" << endl;
        }
    }
}

//-------------------------------------------------------------------------------------------------------
/*
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Parámetros de entrada:
                      - signum: Número de señal recibida.
Parámetros de salida:
                      - No hay parámetros de salida, ya que la función es de tipo void.
Función: Actualiza la hora actual del parque, cambia el indicador de alarma y muestra el estado actual del parque.
Variables globales utilizadas:
  - horaActual: Hora actual en la simulación del parque.
  - alarmFlag: Indicador de alarma para controlar el flujo de ejecución en el programa.
*/
void ManejadorSenales(int signum) {
  // Incrementar la hora actual y cambiar el indicador de alarma
  // Sección crítica protegida por el mutex
  {
    lock_guard<mutex> lock(mtx);
    horaActual++;
  }
  
  cout << "--------------------------------------" << endl;
  cout << "-----Hora actual: " << horaActual <<" -----"<< endl;
  // Mostrar el estado actual del parque
  MostrarEstadoDelParque();
  cout << "--------------------------------------" << endl<<endl;
  alarmFlag = 1;
}

// -------------------------------------------------------------------------------------------
/*
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Parámetros de entrada:
                      - &str:  intenta determinar si un string es convertible en un número válido. 
Parámetros de salida:
                      -Si la conversión es exitosa y todos los caracteres del string fueron usados en el intento de convertirlo a número, la función devuelve 1 indicando que el string es un número. De lo contrario, la función devuelve 0 indicando que el string no es un número válido.

Función: Verificar si &str representa un número válido.
Variables globales utilizadas:
  - Ninguna.
*/
int EsNumero(const string &str) {
  try {
    size_t pos = 0;
    stod(str, &pos); // Intenta convertir el string a un número de coma flotante
    // Verifica si se consumió todo el string, lo que indica que es un número válido
    return pos == str.length() ? 1 : 0;
  } catch (const std::invalid_argument &) {
    // La conversión arroja una excepción si el string no es un número válido
    return 0;
  }
}

//-------------------------------------------------------------------------------------------------------
/*
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Parámetros de entrada:
                      - argc: Número de argumentos en la línea de comandos.
                      - argumentos[]: Array de cadenas de caracteres que contiene los argumentos proporcionados en la línea de comandos al ejecutar el programa.
                      - *comandoIngresado:  Es un puntero a una estructura que contiene información sobre los comandos válidos que se pueden ingresar al programa. Este puntero está se utiliza para acceder a los campos de la estructura que representan los comandos válidos.

Parámetros de salida:
                      -Retorna true si al menos uno de los comandos proporcionados coincide con alguno de los comandos válidos almacenados en la estructura comandoIngresado. En caso contrario, devuelve false.

Función: Validar los comandos en la línea de comandos comparándolos con los comandos válidos almacenados en la estructura.
Variables globales utilizadas:
  - Ninguna.
*/
bool ValidarComandos(int argc, string argumentos[], comando *comandoIngresado) {
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

//-------------------------------------------------------------------------------------------------------
/*
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Parámetros de entrada:
                      - No hay parámetros de entrada. 
Parámetros de salida:
                      - No hay parámetros de salida, ya que la función es de tipo void.

Función:  realiza un análisis de las reservas registradas por hora y genera un reporte donde muestra las s olicitudes negadas, las solicitudes aceptadas, las solicitudes reprogramadas y las horas pico.
Variables globales utilizadas:
  - horaInicio: Hora de inicio de la simulación del parque.
  - horaFinal: Hora de finalización de la simulación del parque.
  - reservasPorHora: Mapa global que asocia cada hora con un vector de reservas.
  - solicitudesNegadas: Contador de solicitudes negadas.
  - solicitudesAceptadas: Contador de solicitudes aceptadas.
  - solicitudesReprogramadas: Contador de solicitudes reprogramadas.
  - aceptadaHora: Mapa que almacena el número de solicitudes aceptadas por hora. 
*/
void GenerarInforme() {
  // Calcular personas totales por hora desde las 7 hasta las 19
  map<int, int> personasTotalesPorHora;
  int maxPersonas = 0, minPersonas = numeric_limits<int>::max();
  vector<int> horasPico, horasMenosConcurridas;
  // Inicializar el mapa personasTotalesPorHora para almacenar el recuento de personas por hora
  for (int hora = horaInicio; hora < horaFinal; ++hora) {
    personasTotalesPorHora[hora] = 0; // Inicializar con 0
  }
  // Iterar sobre la estructura reservasPorHora para contar personas por hora
  for (const auto &par : reservasPorHora) {
    // Verificar si la hora está dentro del rango de 7 a 18 horas
    if (par.first >= 7 && par.first < 19) {
      // Sumar la cantidad de personas de cada reserva para cada hora
      for (const auto &reserva : par.second) {
        personasTotalesPorHora[par.first] += reserva.cantFamiliares;
      }
    }
  }
  // Encontrar la hora con mayor y menor número de personas
  // Iterar sobre el mapa personasTotalesPorHora para identificar las horas pico y menos concurridas
  for (const auto &hora : personasTotalesPorHora) {
    // Identificar la hora con la mayor cantidad de personas (hora pico)
    if (hora.second > maxPersonas) {
      maxPersonas = hora.second;
      horasPico.clear();
      horasPico.push_back(hora.first);
    } else if (hora.second == maxPersonas) {
      horasPico.push_back(hora.first);
    }

    // Identificar la hora con la menor cantidad de personas (hora menos concurrida)
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

//-------------------------------------------------------------------------------------------------------
/*
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Parámetros de entrada:
                      - argc: Número de argumentos en la línea de comandos.
                      - argumentos[]: Array de cadenas de caracteres que contiene los argumentos proporcionados en la línea de comandos al ejecutar el programa.
                      - comando: Índice que representa la posición del comando que se quiere contar en los argumentos.
Parámetros de salida:
                      - devuelve un valor booleano que indica si el comando especificado aparece exactamente una vez en la lista de argumentos pasados por la línea de comandos.

Función: Recorre los argumentos de la línea de comandos y cuenta cuántas veces aparece un comando específico en esa lista. Devuelve true si el comando aparece exactamente una vez y false en caso contrario.
Variables globales utilizadas:
  - Ninguna.
*/
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

//-------------------------------------------------------------------------------------------------------
/*
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Parámetros de entrada:
                      - argc: Número de argumentos en la línea de comandos.
                      - argumentos[]: Array de cadenas de caracteres que contiene los argumentos proporcionados en la línea de comandos al ejecutar el programa.
                      - *comandoIngresado:  Es un puntero a una estructura que contiene información sobre los comandos válidos que se pueden ingresar al programa. Este puntero está se utiliza para acceder a los campos de la estructura que representan los comandos válidos.
Parámetros de salida:
                      - Devuelve un valor booleano que indica si los valores asociados a los comandos pasados por la línea de comandos cumplen con ciertas condiciones preestablecidas. No tiene parámetros de salida adicionales.

Función: Se encarga de validar los valores asociados a ciertos comandos de la línea de comandos y devuelve un booleano indicando si estos valores son correctos o no.
Variables globales utilizadas:
  - Ninguna.
*/
bool ValoresCorrectos(int argc, string argumentos[], comando *comandoIngresado) {
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
          cout << "La hora incial no corresponde al horario del parque."<< endl;
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

//-------------------------------------------------------------------------------------------------------
/*
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Parámetros de entrada:
                      - argc: Número de argumentos en la línea de comandos.
                      - argumentos[]: Array de cadenas de caracteres que contiene los argumentos proporcionados en la línea de comandos al ejecutar el programa.
                      - *comandos: Es un puntero a una estructura comando que almacena información sobre los comandos válidos y sus valores asociados.
Parámetros de salida:
                      - Devuelve un valor booleano que indica si los comandos y sus valores asociados pasados por la línea de comandos son correctos y cumplen con ciertas condiciones preestablecidas. No tiene parámetros de salida adicionales.

Función: Es un conjunto de validaciones que se aplican a los argumentos ingresados en la línea de comandos para asegurarse de que se cumplan ciertas condiciones. En caso de que alguna de las condiciones de verificación falle la función muestra un mensaje específico de error y retorna false, lo que indica que los comandos ingresados no son válidos. Si todas las comprobaciones se superan con éxito, la función devuelve true.
Variables globales utilizadas:
  - Ninguna.
*/
bool VerificarComando(int argc, string argumentos[], comando *comandos) {
  bool cantidad_correcta, comandos_correctos, valores_correctos;

  // Verificación de la cantidad de argumentos
  if (argc != 11) {
    printf("Cantidad de argumentos ERRONEA \n");
    printf("Uso: $./controlador –i horaInicio –f horafinal –s segundoshora –t "
           "totalpersonas –p pipecrecibe \n");
    return false;
  }

  // Verificación de los comandos
  comandos_correctos = ValidarComandos(argc, argumentos, comandos);

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
  return true;
}

//-------------------------------------------------------------------------------------------------------
/*
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Parámetros de entrada:
  - nombreAgente: Nombre del agente al cual se enviará el resultado de la reserva.
  - r: Estructura de reserva que contiene los detalles del resultado de la reserva.
Parámetros de salida:
  - No hay parámetros de salida, ya que la función es de tipo void.
Función: Envía el resultado de una reserva a través de un pipe al agente especificado.
Variables globales utilizadas:
  - No se usan.
*/
void EnviarResultado(const char *nombreAgente, const reserva &r) {
  int fd;

  // Intentar abrir el pipe para escritura
  while ((fd = open(nombreAgente, O_WRONLY)) == -1) {
    perror("pipe");
    cout << "Se volverá a intentar después" << endl;
    sleep(1); // Esperar antes de reintentar
  }

  // Enviar la estructura de reserva a través del pipe
  int bytesEscritos = write(fd, &r, sizeof(reserva));
  if (bytesEscritos == -1) {
    perror("write");
    cerr << "Error al escribir en el pipe" << endl;
  } else {
    cout << "Resultado de la reserva enviado" << endl << endl;
  }

  // Cerrar el pipe después de enviar los datos
  close(fd);
}


//-------------------------------------------------------------------------------------------------------
/*
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Parámetros de entrada:
                      - No hay parámetros de entrada directos, pero la función utiliza variables globales.
Parámetros de salida:
                      - No hay parámetros de salida, ya que la función es de tipo void.
Función: Inicializa el mapa global 'reservasPorHora' asignando un vector vacío de reservas para cada hora entre 'horaInicio' y 'horaFinal'.
Variables globales utilizadas:
  - horaInicio: Hora de inicio de la simulación del parque.
  - horaFinal: Hora de finalización de la simulación del parque.
  - reservasPorHora: Mapa global que asocia cada hora con un vector de reservas.
*/
void InicializarHoras() {
  // Inicializar el mapa 'reservasPorHora' para cada hora en el rango
  for (int hora = horaInicio; hora <= horaFinal; ++hora) {
    reservasPorHora[hora] = vector<reserva>();
  }
}


//-------------------------------------------------------------------------------------------------------
/*
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Parámetros de entrada:
                      - r: Referencia a una estructura de reserva que contiene los detalles de la reserva a verificar.
Parámetros de salida:
                      - No hay parámetros de salida, ya que se modifica la estructura de reserva directamente.
Función: Verifica la viabilidad de una reserva dada, ajustando la reserva si es necesario.
Variables globales utilizadas:
  - horaActual: Hora actual en la simulación del parque.
  - horaFinal: Hora de cierre del parque.
  - totalPersonas: Capacidad máxima de personas en el parque.
  - reservasPorHora: Mapa que almacena las reservas organizadas por hora.
  - solicitudesNegadas: Contador de solicitudes negadas.
  - solicitudesAceptadas: Contador de solicitudes aceptadas.
  - solicitudesReprogramadas: Contador de solicitudes reprogramadas.
  - aceptadaHora: Mapa que almacena el número de solicitudes aceptadas por hora. 
*/
void VerificarReservas(reserva &r) {
  cout << endl;
  cout << "Verificando reserva..." << endl;
  cout << "Agente: " << r.Agente << endl;
  cout << "Mombre de Familia: " << r.nomFamilia << endl;
  cout << "Cantidad: " << r.cantFamiliares << endl;
  cout << "Hora: " << r.horaInicio << endl;

  // Función lambda para calcular el total de personas en una hora dada
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
          if (j > horaFinal - 1 || personasEnHora(j) + r.cantFamiliares > totalPersonas)           {
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
  EnviarResultado(r.Agente, r);
}

//-------------------------------------------------------------------------------------------------------
/*
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Parámetros de entrada:
                      - nombrePipe: Nombre del pipe utilizado para enviar la hora actual.
Parámetros de salida:
                      - No hay parámetros de salida, ya que la función es de tipo void.
Función: Esta función intenta abrir un pipe y envía la hora actual a través de él.
Variables globales utilizadas:
  - horaActual: Hora actual en la simulación del parque que se enviará a través del pipe.
*/
void EnviarHora(char *nombrePipe) {
  int fd, bytesEscritos, creado = 0;

  // Intentar abrir el pipe para escritura
  do {
    fd = open(nombrePipe, O_WRONLY);
    if (fd == -1) {
      perror("pipe");
      printf(" Se volvera a intentar despues\n");
      sleep(3); // Esperar antes de reintentar
    } else
      creado = 1; // Indicar que el pipe se abrió correctamente
  } while (creado == 0);

  // Escribir la hora actual en el pipe
  bytesEscritos = write(fd, &horaActual, sizeof(int));

  // Manejar los errores de escritura
  if (bytesEscritos == -1) {
    perror("write");
    cerr << "Error al escribir en el pipe" << endl;
  } else {
    cout << "Hora enviada: " << horaActual << endl; 
  }

  close(fd);
}

//-------------------------------------------------------------------------------------------------------

void TerminarAgentes()
{
  cout << "--------------------------------------"<<endl;
  cout << "Terminando conexión con agentes ..." << endl;
  for(long unsigned int i=0; i<listaAgentes.size();i++)
    {
      if(access(listaAgentes[i].Agente, F_OK) == 0)
      {
        cout << "El pipe nominal " << listaAgentes[i].Agente << " existe." << endl;
        
        if(kill(listaAgentes[i].pid, SIGUSR1)==-1) ///// MAJEO DE LA EXCEPCION
        {
          perror("kill:");
        }
        else
        {
          cout << "Señal enviada correctamente"<<endl;
        }
      }
      else
      {
        cout << "El pipe nominal " << listaAgentes[i].Agente << " NO existe. El agente ya había terminado" << endl;
      }
    }

  cout << "--------------------------------------"<<endl;
}

//-------------------------------------------------------------------------------------------------------
/*
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Parámetros de entrada:
                      - indice: Puntero a void utilizado para pasar argumentos al hilo.
Parámetros de salida:
                      - No hay parámetros de salida, la función es de tipo void* y retorna NULL al terminar.
Función: Este hilo se encarga de incrementar la hora actual del parque y generar un informe cuando se alcanza la hora de cierre.
Variables globales utilizadas:
  - horaActual: Hora actual en la simulación del parque.
  - horaFinal: Hora de cierre del parque.
  - segundosHora: Intervalo de tiempo en segundos para incrementar la hora.
  - alarmFlag: Bandera utilizada para controlar el flujo de ejecución en función de las alarmas.
*/
void *IncrementarHora(void *indice) {
  // Configurar el manejador de señales para SIGALRM
  signal(SIGALRM, ManejadorSenales);

  while (true) {
    // Reiniciar la bandera de alarma y establecer una alarma
    alarmFlag = 0;
    alarm(segundosHora);

    // Esperar a que la bandera de alarma cambie a 1
    while (alarmFlag != 1) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Verificar si se ha alcanzado la hora final o la hora de cierre
    if (horaActual >= 19 || horaActual >= horaFinal) {
      cout << "Hilo 1: Contador alcanzó " << horaFinal << ". Terminando los hilos." << endl;
      // Generar informe al final del día
      GenerarInforme();
      // Terminar ejecucion de los agentes
      TerminarAgentes();
      break;
    }
  }

  exit(0);
}


//-------------------------------------------------------------------------------------------------------
/*
Autores: Jose Manuel Rodriguez, Laura Valentina Ovalle, Juan Miguel Zuluaga
Parámetros de entrada:
                      - indice: Puntero a void utilizado para pasar argumentos al hilo.
Parámetros de salida:
                      - No hay parámetros de salida, la función es de tipo void* y retorna NULL.
Función: Este hilo se encarga de leer y procesar las solicitudes de reserva o registros de agentes recibidos a través de un pipe.
Variables globales utilizadas:
  - nombrePipe1: Nombre del pipe nominal para recibir datos.
  - horaActual: Hora actual en la simulación del parque.
  - listaAgentes: Lista de nombres de agentes que han hecho solicitudes.
*/
void *VerificarContador(void *indice) {
  int nbytes;
  int fd;
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
    
    // Leer datos del pipe
    nbytes = read(fd, &r, sizeof(r));

    if (nbytes == -1) {
      perror("pipe:");
      exit(0);
    } else if (nbytes == 0) {
      continue;
    }

    // Procesar si es un registro de agente o una solicitud de reserva
    if (r.registro == true) {
      printf("Nombre agente: %s\n", r.Agente);
      EnviarHora(r.Agente);
      listaAgentes.push_back(r);
      continue;
    } else if (r.registro == false) {
      VerificarReservas(r);
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
  pthread_t threads[2];
  void *returnValue;

  InicializarHoras();
  // VERIFICACIÓN DE COMANDOS
  // Convierte los argumentos a string
  string arguments[argc];
  for (int i = 0; i < argc; i++) {
    arguments[i] = argv[i]; // Convierte el elemento argv[i] a std::string
  }

  /*for (int i = 0; i < argc; i++) {
    cout << "Argumento " << i << ": " << arguments[i] << std::endl;
  }*/

  comandosAceptados = VerificarComando(argc, arguments, &comandos);

  if (comandosAceptados == true) {
    horaActual = comandos.valor_i;
    horaInicio = horaActual;
    segundosHora = comandos.valor_s;
    horaFinal = comandos.valor_f;
    totalPersonas = comandos.valor_t;

    // ---------------------- INICIANDO EL PROGRAMA
    // signal (SIGALRM, (sighandler_t)signalHandler);
    // signal (SIGALRM, (sighandler_t)signalHandler); // Establece signalHandler
    // como el manejador para SIGALRM
    unlink(comandos.valor_p.c_str());
    nombrePipe1 = comandos.valor_p;

    if (pthread_create(&threads[0], NULL, IncrementarHora, NULL) != 0) {
      cerr << "Error al crear el hilo 1." << endl;
      return 1;
    }

    if (pthread_create(&threads[1], NULL, VerificarContador, NULL) != 0) {
      cerr << "Error al crear el hilo 2." << endl;
      return 1;
    }

    // Esperar a que ambos hilos terminen

    if (pthread_join(threads[0], &returnValue) != 0) {
      fprintf(stderr, "Error al unirse al hilo 1.\n");
      return 1;
    }

    if (pthread_join(threads[1], &returnValue) != 0) {
      fprintf(stderr, "Error al unirse al hilo 2.\n");
      return 1;
    }

    cout << "Programa terminado." << endl;

    return 0;
  }
}