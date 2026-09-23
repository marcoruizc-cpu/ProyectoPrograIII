#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm> // std::min
#ifdef _WIN32
// NOMINMAX evita que <windows.h> defina las macros min/max, que chocan con
// std::min/std::max (usado en la paginacion de resultados).
#define NOMINMAX
#include <windows.h>
#endif
#include "modelo/Pelicula.h"
#include "app/Plataforma.h"
using namespace std;

const size_t TAMANO_PAGINA = 5;

// Lee una linea completa y la devuelve sin espacios a los costados.
// Devuelve false si se acabo la entrada (EOF), para no quedar en un bucle.
bool leerLinea(string& linea) {
    if (!getline(cin, linea)) return false;
    size_t inicio = linea.find_first_not_of(" \t\r");
    size_t fin = linea.find_last_not_of(" \t\r");
    linea = (inicio == string::npos) ? "" : linea.substr(inicio, fin - inicio + 1);
    return true;
}

// Imprime "Titulo (anio)" de cada pelicula de una coleccion de IDs.
template <typename Coleccion>
void imprimirTitulos(const Plataforma& plataforma, const Coleccion& ids) {
    if (ids.empty()) {
        cout << "  (vacia)\n";
        return;
    }
    for (int id : ids) {
        const Pelicula& p = plataforma.obtenerPelicula(id);
        cout << "  - " << p.titulo << " (" << p.anioEstreno << ")\n";
    }
}

string unirConComas(const vector<string>& elementos) {
    string resultado;
    for (size_t i = 0; i < elementos.size(); i++) {
        if (i > 0) resultado += ", ";
        resultado += elementos[i];
    }
    return resultado;
}

// F11: detalle de una pelicula con las opciones Like y Ver mas tarde (F12).
void mostrarDetalle(Plataforma& plataforma, int id) {
    const Pelicula& p = plataforma.obtenerPelicula(id);
    cout << "\n=========================================\n";
    cout << " " << p.titulo << " (" << p.anioEstreno << ")\n";
    cout << "=========================================\n";
    cout << " Origen:   " << p.origen << "\n";
    cout << " Director: " << p.director << "\n";
    cout << " Reparto:  " << (p.reparto.empty() ? "-" : unirConComas(p.reparto)) << "\n";
    cout << " Genero:   " << p.genero << "\n";
    cout << "-----------------------------------------\n";
    cout << " SINOPSIS\n " << p.sinopsis << "\n";
    cout << "-----------------------------------------\n";
    cout << " Wikipedia: " << p.wikiPagina << "\n";

    string entrada;
    while (true) {
        cout << "\n[1] Like" << (plataforma.tieneLike(id) ? " (ya te gusta)" : "")
             << "   [2] Ver mas tarde" << (plataforma.estaEnVerMasTarde(id) ? " (ya esta en tu lista)" : "")
             << "   [0] Volver\nOpcion: ";
        if (!leerLinea(entrada) || entrada == "0") return;

        if (entrada == "1") {
            cout << (plataforma.darLike(id) ? "Le diste Like a " : "Ya le habias dado Like a ")
                 << p.titulo << ".\n";
        } else if (entrada == "2") {
            cout << (plataforma.agregarAVerMasTarde(id) ? "Agregada a Ver mas tarde: " : "Ya estaba en Ver mas tarde: ")
                 << p.titulo << ".\n";
        } else {
            cout << "Opcion invalida.\n";
        }
    }
}

// F9: muestra los resultados (ya ordenados por importancia) de 5 en 5, con
// la opcion de ver los 5 siguientes o elegir una pelicula por su numero.
void navegarResultados(Plataforma& plataforma, const vector<int>& ids) {
    if (ids.empty()) {
        cout << "  (sin coincidencias)\n";
        return;
    }

    size_t inicio = 0;
    string entrada;
    while (true) {
        size_t fin = min(inicio + TAMANO_PAGINA, ids.size());
        cout << "\nResultados " << inicio + 1 << "-" << fin << " de " << ids.size() << ":\n";
        for (size_t i = inicio; i < fin; i++) {
            const Pelicula& p = plataforma.obtenerPelicula(ids[i]);
            cout << "  [" << i + 1 << "] " << p.titulo << " (" << p.anioEstreno << ")\n";
        }

        cout << "Numero = ver detalle";
        if (fin < ids.size()) cout << " | V = ver siguientes 5";
        cout << " | X = volver\nOpcion: ";
        if (!leerLinea(entrada)) return;

        if (entrada == "x" || entrada == "X") return;

        if (entrada == "v" || entrada == "V") {
            if (fin < ids.size()) {
                inicio = fin;
            } else {
                cout << "No hay mas resultados.\n";
            }
            continue;
        }

        // Numero de pelicula: se convierte con istringstream revisando el
        // estado de fallo (igual que el anio en LectorCSV, sin try/catch).
        istringstream conversor(entrada);
        size_t numero = 0;
        // Solo se aceptan los numeros de la pagina que se esta mostrando.
        if (conversor >> numero && conversor.eof() && numero >= inicio + 1 && numero <= fin) {
            mostrarDetalle(plataforma, ids[numero - 1]);
        } else {
            cout << "Opcion invalida.\n";
        }
    }
}

void mostrarMisListas(const Plataforma& plataforma) {
    cout << "\n>> VER MAS TARDE\n";
    imprimirTitulos(plataforma, plataforma.obtenerVerMasTarde());
    cout << "\n>> TE GUSTARON\n";
    imprimirTitulos(plataforma, plataforma.obtenerLikes());
}

void mostrarMenu() {
    cout << "\n=========================================\n";
    cout << " Plataforma de Streaming - Buscador\n";
    cout << "=========================================\n";
    cout << "1. Buscar por texto(titulo o sinopsis)\n";
    cout << "2. Buscar por director\n";
    cout << "3. Buscar por actor\n";
    cout << "4. Buscar por genero\n";
    cout << "5. Mis listas (Ver mas tarde y Likes)\n";
    cout << "6. Salir\n";
    cout << "Elige una opcion: ";
}

int main(int argc, char** argv) {
#ifdef _WIN32
    // La consola de Windows no usa UTF-8 por defecto (usa CP850/437). El
    // CSV si esta en UTF-8: sin esto los titulos con tilde se ven como
    // simbolos raros (salida) y lo que el usuario escribe con tilde llega en
    // otra codificacion y no coincide con el indice (entrada).
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

#ifdef RUTA_PROYECTO
    string rutaPorDefecto = string(RUTA_PROYECTO) + "/dataset/wiki_movie_plots_deduped.csv";
#else
    string rutaPorDefecto = "dataset/wiki_movie_plots_deduped.csv";
#endif
    string ruta = (argc > 1) ? argv[1] : rutaPorDefecto;

    cout << "Cargando base de datos de peliculas..." << endl;
    Plataforma plataforma;
    size_t cantidad = plataforma.cargarPeliculas(ruta);
    cout << "Peliculas cargadas: " << cantidad << endl;
    if (cantidad == 0) {
        cout << "\nNo se encontro el dataset en: " << ruta << "\n"
             << "Descargalo y colocalo en dataset/ (ver docs/instalacion.md).\n";
        return 1;
    }

    cout << "Construyendo indice de busqueda (arbol de sufijos)..." << endl;
    plataforma.construirIndiceTexto();

    cout << "Construyendo indices por tag (director, actor, genero)..." << endl;
    plataforma.construirIndicesTags();

    cout << "Listo. Base de datos indexada.\n";

    int opcion = 0;
    do {
        mostrarMenu();
        cin >> opcion;

        if (cin.eof()) break; // se acabo la entrada
        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n'); // descarta lo que quedo mal escrito en el buffer
            cout << "Opcion invalida.\n";
            continue;
        }
        cin.ignore(10000, '\n'); // limpia el salto de linea pendiente antes del getline

        string consulta;
        switch (opcion) {
            case 1:
                cout << "Palabra, sub-palabra o frase a buscar: ";
                leerLinea(consulta);
                navegarResultados(plataforma, plataforma.buscarPorTexto(consulta));
                break;
            case 2:
                cout << "Nombre del director (completo o parcial): ";
                leerLinea(consulta);
                navegarResultados(plataforma, plataforma.buscarPorDirector(consulta));
                break;
            case 3:
                cout << "Nombre del actor (completo o parcial): ";
                leerLinea(consulta);
                navegarResultados(plataforma, plataforma.buscarPorActor(consulta));
                break;
            case 4:
                cout << "Genero(s), separados por coma: ";
                leerLinea(consulta);
                navegarResultados(plataforma, plataforma.buscarPorGeneros(consulta));
                break;
            case 5:
                mostrarMisListas(plataforma);
                break;
            case 6:
                cout << "Hasta luego.\n";
                break;
            default:
                cout << "Opcion invalida.\n";
        }
    } while (opcion != 6);

    return 0;
}
