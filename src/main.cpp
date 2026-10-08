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
#include "busqueda/Ranking.h"
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

string unirConComas(const vector<string>& elementos) {
    string resultado;
    for (size_t i = 0; i < elementos.size(); i++) {
        if (i > 0) resultado += ", ";
        resultado += elementos[i];
    }
    return resultado;
}

// F11: detalle de una pelicula con las opciones Like y Ver mas tarde (F12).
void mostrarDetalle(Plataforma& plataforma, int id, const string& rutaListas) {
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
            bool agregado = plataforma.darLike(id);
            cout << (agregado ? "Le diste Like a " : "Ya le habias dado Like a ")
                 << p.titulo << ".\n";
            if (agregado && !plataforma.guardarListas(rutaListas)) {
                cerr << "AVISO: No se pudo guardar el Like en disco. "
                     << "El cambio solo estara en memoria durante esta sesion.\n";
            }
        } else if (entrada == "2") {
            bool agregado = plataforma.agregarAVerMasTarde(id);
            cout << (agregado ? "Agregada a Ver mas tarde: " : "Ya estaba en Ver mas tarde: ")
                 << p.titulo << ".\n";
            if (agregado && !plataforma.guardarListas(rutaListas)) {
                cerr << "AVISO: No se pudo guardar Ver mas tarde en disco. "
                     << "El cambio solo estara en memoria durante esta sesion.\n";
            }
        } else {
            cout << "Opcion invalida.\n";
        }
    }
}

// F9: muestra los resultados (ya ordenados por importancia) de 5 en 5, con
// la opcion de ver los 5 siguientes o elegir una pelicula por su numero.
void navegarResultados(Plataforma& plataforma, const vector<int>& ids, const string& rutaListas) {
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
            mostrarDetalle(plataforma, ids[numero - 1], rutaListas);
        } else {
            cout << "Opcion invalida.\n";
        }
    }
}

void mostrarTop5Destacadas(Plataforma& plataforma, size_t totalPeliculas, const string& rutaListas) {
    cout << "\nCalculando Top 5 con Min-Heap (O(N log 5))...\n";

    vector<CandidataTop> candidatas;
    candidatas.reserve(totalPeliculas);

    for (size_t id = 0; id < totalPeliculas; ++id) {
        int idInt = static_cast<int>(id);
        const Pelicula& p = plataforma.obtenerPelicula(idInt);

        double puntaje = static_cast<double>(p.anioEstreno);
        if (plataforma.tieneLike(idInt)) {
            puntaje += 500.0; // Bonificación extra si tiene Like
        }

        candidatas.push_back({idInt, puntaje});
    }

    vector<int> top5Ids = Ranking::obtenerTop5Ids(candidatas);

    cout << "\n=========================================\n";
    cout << " TOP 5 PELICULAS MEJOR EVALUADAS\n";
    cout << "=========================================\n";
    navegarResultados(plataforma, top5Ids, rutaListas);
}

// Permite seleccionar por numero y quitar una pelicula de la lista elegida.
// Paginamos para que las listas extensas no saturen la consola.
void administrarLista(Plataforma& plataforma, bool editarLikes, const string& rutaListas) {
    size_t inicio = 0;
    string entrada;
    while (true) {
        const vector<int>& ids = editarLikes ? plataforma.obtenerLikes()
                                              : plataforma.obtenerVerMasTarde();
        cout << "\n>> " << (editarLikes ? "MIS LIKES" : "VER MAS TARDE") << " (" << ids.size() << ")\n";
        if (ids.empty()) {
            cout << "  (lista vacia)\n";
            return;
        }

        // Despues de quitar el ultimo elemento de una pagina, volver a la anterior.
        if (inicio >= ids.size()) inicio = ((ids.size() - 1) / TAMANO_PAGINA) * TAMANO_PAGINA;
        const size_t fin = min(inicio + TAMANO_PAGINA, ids.size());
        for (size_t i = inicio; i < fin; ++i) {
            const Pelicula& p = plataforma.obtenerPelicula(ids[i]);
            cout << "  [" << i + 1 << "] " << p.titulo << " (" << p.anioEstreno << ")\n";
        }
        cout << "Numero = quitar de la lista";
        if (fin < ids.size()) cout << " | V = siguientes";
        if (inicio > 0) cout << " | A = anteriores";
        cout << " | X = volver\nOpcion: ";
        if (!leerLinea(entrada) || entrada == "x" || entrada == "X") return;
        if (entrada == "v" || entrada == "V") {
            if (fin < ids.size()) inicio = fin;
            else cout << "No hay mas elementos.\n";
            continue;
        }
        if (entrada == "a" || entrada == "A") {
            if (inicio > 0) inicio -= TAMANO_PAGINA;
            else cout << "Estas en la primera pagina.\n";
            continue;
        }

        istringstream conversor(entrada);
        size_t numero = 0;
        if (!(conversor >> numero && conversor.eof() && numero >= inicio + 1 && numero <= fin)) {
            cout << "Opcion invalida.\n";
            continue;
        }

        const int id = ids[numero - 1];
        const string titulo = plataforma.obtenerPelicula(id).titulo;
        cout << "Quitar \"" << titulo << "\" de "
             << (editarLikes ? "Likes" : "Ver mas tarde") << "? (S/N): ";
        if (!leerLinea(entrada)) return;
        if (entrada != "s" && entrada != "S") {
            cout << "Cancelado.\n";
            continue;
        }

        const bool quitado = editarLikes ? plataforma.quitarLike(id)
                                        : plataforma.quitarDeVerMasTarde(id);
        if (!quitado) {
            cout << "La pelicula ya no estaba en esta lista.\n";
            continue;
        }
        cout << "Eliminada de la lista: " << titulo << ".\n";
        if (!plataforma.guardarListas(rutaListas)) {
            cerr << "AVISO: No se pudo actualizar el archivo. "
                 << "El cambio solo estara en memoria durante esta sesion.\n";
        }
    }
}

void mostrarMisListas(Plataforma& plataforma, const string& rutaListas) {
    string entrada;
    while (true) {
        cout << "\n=========== MIS LISTAS ===========\n"
             << "[1] Likes (" << plataforma.obtenerLikes().size() << ")\n"
             << "[2] Ver mas tarde (" << plataforma.obtenerVerMasTarde().size() << ")\n"
             << "[0] Volver al menu\nOpcion: ";
        if (!leerLinea(entrada) || entrada == "0") return;
        if (entrada == "1") administrarLista(plataforma, true, rutaListas);
        else if (entrada == "2") administrarLista(plataforma, false, rutaListas);
        else cout << "Opcion invalida.\n";
    }
}

void mostrarMenu() {
    cout << "\n=========================================\n";
    cout << " Plataforma de Streaming - Buscador\n";
    cout << "=========================================\n";
    cout << "1. Buscar por texto(titulo o sinopsis)\n";
    cout << "2. Buscar por director\n";
    cout << "3. Buscar por actor\n";
    cout << "4. Buscar por genero\n";
    cout << "5. Ver Top 5 destacadas (Min-Heap)\n";
    cout << "6. Mis listas (Ver mas tarde y Likes)\n";
    cout << "7. Salir\n";
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
    const string rutaListas = string(RUTA_PROYECTO) + "/datos_usuario/listas.txt";
#else
    string rutaPorDefecto = "dataset/wiki_movie_plots_deduped.csv";
    const string rutaListas = "datos_usuario/listas.txt";
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

    if (!plataforma.cargarListas(rutaListas)) {
        cerr << "No se pudieron cargar las listas en: " << rutaListas << "\n"
             << "Comprueba el archivo antes de continuar para evitar perder tus listas.\n";
        return 1;
    }
    cout << "Listas cargadas: " << plataforma.obtenerLikes().size()
         << " Likes y " << plataforma.obtenerVerMasTarde().size()
         << " en Ver mas tarde.\n";

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
                navegarResultados(plataforma, plataforma.buscarPorTexto(consulta), rutaListas);
                break;
            case 2:
                cout << "Nombre del director (completo o parcial): ";
                leerLinea(consulta);
                navegarResultados(plataforma, plataforma.buscarPorDirector(consulta), rutaListas);
                break;
            case 3:
                cout << "Nombre del actor (completo o parcial): ";
                leerLinea(consulta);
                navegarResultados(plataforma, plataforma.buscarPorActor(consulta), rutaListas);
                break;
            case 4:
                cout << "Genero(s), separados por coma: ";
                leerLinea(consulta);
                navegarResultados(plataforma, plataforma.buscarPorGeneros(consulta), rutaListas);
                break;
            case 5:
                mostrarTop5Destacadas(plataforma, cantidad, rutaListas);
                break;
            case 6:
                mostrarMisListas(plataforma, rutaListas);
                break;
            case 7:
                cout << "Hasta luego.\n";
                break;
            default:
                cout << "Opcion invalida.\n";
        }
    } while (opcion != 7);

    return 0;
}
