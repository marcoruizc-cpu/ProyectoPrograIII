#ifndef LECTOR_CSV_H
#define LECTOR_CSV_H

#include <string>
#include <vector>
#include "modelo/Pelicula.h"

// Lee un archivo CSV respetando comillas (campos con comas, comillas
// escapadas "" y saltos de linea dentro del campo, como el caso de "Plot").
// Devuelve cada fila como un vector de campos crudos (sin normalizar).
std::vector<std::vector<std::string>> leerCSV(const std::string& rutaArchivo);

// Convierte las filas crudas del CSV (sin el encabezado) en objetos Pelicula.
// Descarta filas malformadas.
// Recibe 'filas' POR VALOR: llamada con un temporal
// (construirPeliculas(leerCSV(ruta))) el vector se MUEVE sin copiarse, y cada
// campo se mueve (std::move) hacia la Pelicula. Asi el texto crudo no queda
// duplicado en memoria (filas + peliculas) y se libera al terminar.
std::vector<Pelicula> construirPeliculas(std::vector<std::vector<std::string>> filas);

#endif
