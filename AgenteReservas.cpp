
// $ ./controlador –i horaInicio –f horafinal –s segundoshora –t totalpersonas –p pipecrecibe

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Registro
{
    string nombre;
    int hora;
    int personas;
};

// Duración en segundos de una "hora"
const int segundosPorHora = 3600;

void procesarSolicitudes(const string &nombreAgente, const string &archivoSolicitudes, const string &pipeCrecibe)
{
    ifstream archivo(archivoSolicitudes);

    if (!archivo.is_open())
    {
        cerr << "Error al abrir el archivo de solicitudes." << endl;
        return;
    }

    string linea;
    Registro registro;

    while (getline(archivo, linea))
    {
        istringstream ss(linea);
        string token;

        if (getline(ss, token, ','))
        {
            registro.nombre = token;
        }

        if (getline(ss, token, ','))
        {
            registro.hora = stoi(token);
        }

        if (getline(ss, token, ','))
        {
            registro.personas = stoi(token);
        }

        // Aquí puedes enviar la solicitud al controlador y esperar la respuesta
        cout << "Agente: " << nombreAgente << ", Nombre: " << registro.nombre << ", Hora: " << registro.hora << ", Personas: " << registro.personas << endl;
    }

    archivo.close();
}

int main(int argc, char *argv[])
{
    if (argc != 7)
    {
        cerr << "Uso incorrecto. Debe proporcionar los argumentos correctamente." << endl;
        return 1;
    }

    string nombreAgente;
    string archivoSolicitudes;
    string pipeCrecibe;

    for (int i = 1; i < argc; i += 2)
    {
        if (string(argv[i]) == "-s")
        {
            nombreAgente = argv[i + 1];
        }
        else if (string(argv[i]) == "-a")
        {
            archivoSolicitudes = argv[i + 1];
        }
        else if (string(argv[i]) == "-p")
        {
            pipeCrecibe = argv[i + 1];
        }
        else
        {
            cerr << "Argumento desconocido: " << argv[i] << endl;
            return 1;
        }
    }

    procesarSolicitudes(nombreAgente, archivoSolicitudes, pipeCrecibe);

    return 0;
}
