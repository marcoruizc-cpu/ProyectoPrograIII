#include "datos/LectorCSV.h"
#include "datos/Texto.h" // separarLista, quitarMarcasDeCita
#include <fstream>
#include <sstream>
#include <iostream>
#include <utility> // std::move

std::vector<std::vector<std::string>> leerCSV(const std::string& rutaArchivo) {
    std::ifstream archivo(rutaArchivo, std::ios::binary);
    if (!archivo.is_open()) {
        std::cerr << "No se pudo abrir el archivo: " << rutaArchivo << std::endl;
        return {};
    }

    std::vector<std::vector<std::string>> filas;
    std::vector<std::string> filaActual;
    std::string campoActual;
    bool dentroDeComillas = false;
    char c;

    while (archivo.get(c)) {
        if (dentroDeComillas) {
            if (c == '"') {
                if (archivo.peek() == '"') {
                    campoActual += '"';
                    archivo.get(c); // consume la segunda comilla (escape "")
                } else {
                    dentroDeComillas = false; // cierre de campo entre comillas
                }
            } else {
                campoActual += c; // incluye comas y saltos de linea literales
            }
        } else {
            if (c == '"') {
                dentroDeComillas = true;
            } else if (c == ',') {
                filaActual.push_back(campoActual);
                campoActual.clear();
            } else if (c == '\n') {
                if (!campoActual.empty() && campoActual.back() == '\r') {
                    campoActual.pop_back();
                }
                filaActual.push_back(campoActual);
                campoActual.clear();
                filas.push_back(filaActual);
                filaActual.clear();
            } else {
                campoActual += c;
            }
        }
    }

    // Ultima fila si el archivo no termina con salto de linea
    if (!campoActual.empty() || !filaActual.empty()) {
        filaActual.push_back(campoActual);
        filas.push_back(filaActual);
    }

    return filas;
}

std::vector<Pelicula> construirPeliculas(std::vector<std::vector<std::string>> filas) {
    std::vector<Pelicula> peliculas;
    if (filas.empty()) return peliculas;
    peliculas.reserve(filas.size());

    const size_t NUM_COLUMNAS = 8; // Release Year, Title, Origin/Ethnicity, Director, Cast, Genre, Wiki Page, Plot
    int filasDescartadas = 0;

    for (size_t i = 1; i < filas.size(); i++) { // i = 1: saltar encabezado
        auto& fila = filas[i];
        if (fila.size() != NUM_COLUMNAS) {
            filasDescartadas++;
            continue;
        }

        Pelicula p;
        p.id = static_cast<int>(peliculas.size());
        // stoi lanza una excepcion si el campo no es un numero valido; en vez
        // de try/catch, extraigo con stringstream y reviso el bit de fallo,
        // que es el mismo mecanismo que ya usan cin/getline.
        std::istringstream conversorAnio(fila[0]);
        if (!(conversorAnio >> p.anioEstreno)) {
            p.anioEstreno = 0;
        }
        // separarLista se calcula ANTES de mover fila[3], fila[4] y fila[5]
        p.directores = separarLista(fila[3]);
        p.reparto = separarLista(fila[4]);
        p.generos = separarLista(fila[5]);
        // Semantica de movimiento (S1): se transfiere el buffer del string
        // en vez de copiarlo; fila[k] queda en un estado valido pero vacio.
        p.titulo = std::move(fila[1]);
        p.origen = std::move(fila[2]);
        p.director = std::move(fila[3]);
        p.genero = std::move(fila[5]);
        p.wikiPagina = std::move(fila[6]);
        p.sinopsis = quitarMarcasDeCita(fila[7]);

        peliculas.push_back(std::move(p));
    }

    if (filasDescartadas > 0) {
        std::cerr << "Aviso: se descartaron " << filasDescartadas
                  << " filas malformadas durante el preprocesamiento." << std::endl;
    }

    return peliculas;
}
