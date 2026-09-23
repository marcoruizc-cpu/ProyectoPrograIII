#include "busqueda/IndiceTags.h"
#include "datos/Texto.h" // normalizarTag

// Agrega idPelicula a la lista de 'clave' sin repetirlo: las peliculas se
// procesan en orden de id, asi que basta mirar el ultimo elemento (p. ej.
// un genero repetido en el mismo campo: "comedy, comedy-drama, comedy").
static void agregarSinRepetir(std::map<std::string, std::vector<int>>& indice,
                              const std::string& clave, int idPelicula) {
    std::vector<int>& lista = indice[clave];
    if (lista.empty() || lista.back() != idPelicula) {
        lista.push_back(idPelicula);
    }
}

void IndiceTags::indexar(const std::vector<Pelicula>& peliculas) {
    for (const auto& p : peliculas) {
        for (const auto& director : p.directores) {
            agregarSinRepetir(porDirector, normalizarTag(director), p.id);
        }
        for (const auto& actor : p.reparto) {
            agregarSinRepetir(porActor, normalizarTag(actor), p.id);
        }
        for (const auto& genero : p.generos) {
            agregarSinRepetir(porGenero, normalizarTag(genero), p.id);
        }
    }
}

std::vector<int> IndiceTags::buscarPorDirector(const std::string& nombre) const {
    auto it = porDirector.find(normalizarTag(nombre));
    return (it != porDirector.end()) ? it->second : std::vector<int>{};
}

std::vector<int> IndiceTags::buscarPorActor(const std::string& nombre) const {
    auto it = porActor.find(normalizarTag(nombre));
    return (it != porActor.end()) ? it->second : std::vector<int>{};
}

std::vector<int> IndiceTags::buscarPorGenero(const std::string& genero) const {
    auto it = porGenero.find(normalizarTag(genero));
    return (it != porGenero.end()) ? it->second : std::vector<int>{};
}
