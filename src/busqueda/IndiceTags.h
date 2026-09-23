#ifndef INDICE_TAGS_H
#define INDICE_TAGS_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include "modelo/Pelicula.h"

// Indices secundarios (contenedores asociativos, S5) para buscar peliculas
// por director, actor o genero. Cada indice relaciona un nombre (o genero)
// normalizado con las peliculas donde aparece.
//
// La busqueda acepta el nombre COMPLETO o PARCIAL: una palabra ("spielberg")
// o un fragmento ("spiel"). Un nombre coincide si contiene TODAS las palabras
// escritas ("steven spiel" -> "steven spielberg"). Se devuelven las peliculas
// de todos los nombres que coinciden.
//
// std::map (arbol balanceado): mantiene las claves ordenadas y su costo es
// predecible. std::unordered_map (S5B) tambien seria valido.
class IndiceTags {
private:
    std::map<std::string, std::vector<int>> porDirector;
    std::map<std::string, std::vector<int>> porActor;
    std::map<std::string, std::vector<int>> porGenero;

    // Peliculas de todos los nombres del indice que contienen cada una de
    // 'palabras' (como palabra o fragmento). Recorre las claves: O(K * L),
    // K = nombres distintos del indice, L = largo del nombre.
    static std::set<int> buscarParcial(const std::map<std::string, std::vector<int>>& indice,
                                       const std::vector<std::string>& palabras);

public:
    void indexar(const std::vector<Pelicula>& peliculas);

    // Nombre completo o parcial.
    std::set<int> buscarPorDirector(const std::string& nombre) const;
    std::set<int> buscarPorActor(const std::string& nombre) const;

    // Uno o varios generos separados por coma ("drama, comedy"), cada uno
    // completo o parcial. Devuelve, por cada pelicula que tiene AL MENOS
    // uno, cuantos de los generos pedidos comparte.
    std::map<int, int> buscarPorGeneros(const std::string& generos) const;
};

#endif
