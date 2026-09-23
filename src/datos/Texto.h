#ifndef TEXTO_H
#define TEXTO_H

#include <string>
#include <vector>

// Utilidades de procesamiento de texto compartidas por los indices.
// Se separan de LectorCSV porque no dependen del formato CSV: las usan
// IndiceBusqueda (titulo/sinopsis) e IndiceTags (director/actor/genero).

// Normaliza texto para indexar y buscar:
// - minusculas;
// - pliega tildes de letras latinas (UTF-8) a ASCII: "Amélie" -> "amelie";
// - puntuacion ASCII y Unicode (apostrofo curvo, rayas, comillas) -> espacio;
// - caracteres de otros alfabetos (malayalam, japones) se conservan intactos.
std::string normalizarTexto(const std::string& texto);

// Separa un texto ya normalizado en palabras.
std::vector<std::string> tokenizar(const std::string& textoNormalizado);

// Normaliza un tag para usarlo como clave: recorta espacios y pasa a
// minusculas y plegando tildes, conservando la puntuacion ("S. Fleming").
std::string normalizarTag(const std::string& texto);

// Elimina las marcas de cita de Wikipedia de una sinopsis: "[12]", "[a]",
// "[Note 1]", "[citation needed]". Conserva corchetes con palabras ("[her]").
std::string quitarMarcasDeCita(const std::string& texto);

// true si la palabra (ya normalizada) es un conector que no se indexa.
bool esStopword(const std::string& palabra);

#endif
