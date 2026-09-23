#ifndef INDICE_TAGS_H
#define INDICE_TAGS_H

#include <string>
#include <vector>
#include <map>
#include "modelo/Pelicula.h"

// Indices secundarios (contenedores asociativos, S5) para buscar peliculas
// por director, actor o genero -- no requieren el SuffixTrie porque aca
// se busca por el TAG COMPLETO, no por substring dentro de el.
// std::map (arbol balanceado, O(log n)): mantiene las claves ordenadas y su
// costo es predecible. std::unordered_map (S5B) tambien seria valido.
class IndiceTags {
private:
    std::map<std::string, std::vector<int>> porDirector;
    std::map<std::string, std::vector<int>> porActor;
    std::map<std::string, std::vector<int>> porGenero;

public:
    void indexar(const std::vector<Pelicula>& peliculas);

    std::vector<int> buscarPorDirector(const std::string& nombre) const;
    std::vector<int> buscarPorActor(const std::string& nombre) const;
    std::vector<int> buscarPorGenero(const std::string& genero) const;
};

#endif
