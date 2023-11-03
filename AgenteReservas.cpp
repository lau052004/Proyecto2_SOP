#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

struct Registro {
    string nombre;
    int hora;
    int personas;
};

void leerArchivo(const string& nombreArchivo) {
    ifstream archivo(nombreArchivo);

    if (!archivo.is_open()) {
        cerr << "Error al abrir el archivo." << endl;
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

        cout << "Nombre: " << registro.nombre << ", Hora: " << registro.hora << ", Personas: " << registro.personas << endl;
    }

    archivo.close();
}

int main() {
    const string nombreArchivo = "datos.txt"; 
    leerArchivo(nombreArchivo);
    return 0;
}
