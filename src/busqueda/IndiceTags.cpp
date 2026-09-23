#include "busqueda/IndiceTags.h"
#include "datos/Texto.h" // normalizarTag, normalizarTexto, tokenizar, separarLista

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

std::set<int> IndiceTags::buscarParcial(const std::map<std::string, std::vector<int>>& indice,
                                        const std::vector<std::string>& palabras) {
    std::set<int> resultado;
    if (palabras.empty()) return resultado;

    for (const auto& par : indice) {
        const std::string& nombre = par.first; // ya normalizado: minusculas y sin tildes
        bool contieneTodas = true;
        for (const std::string& palabra : palabras) {
            if (nombre.find(palabra) == std::string::npos) {
                contieneTodas = false;
                break;
            }
        }
        if (contieneTodas) {
            resultado.insert(par.second.begin(), par.second.end());
        }
    }
    return resultado;
}

// La consulta se normaliza igual que el texto ("Spielberg" -> "spielberg",
// "François" -> "francois") y se separa en palabras.
std::set<int> IndiceTags::buscarPorDirector(const std::string& nombre) const {
    return buscarParcial(porDirector, tokenizar(normalizarTexto(nombre)));
}

std::set<int> IndiceTags::buscarPorActor(const std::string& nombre) const {
    return buscarParcial(porActor, tokenizar(normalizarTexto(nombre)));
}

std::map<int, int> IndiceTags::buscarPorGeneros(const std::string& generos) const {
    std::map<int, int> coincidencias; // idPelicula -> cuantos generos pedidos comparte
    std::set<std::string> yaBuscados; // "drama, drama" cuenta una sola vez

    for (const std::string& genero : separarLista(generos)) {
        std::vector<std::string> palabras = tokenizar(normalizarTexto(genero));
        std::string clave;
        for (const std::string& palabra : palabras) clave += palabra + " ";
        if (palabras.empty() || yaBuscados.count(clave)) continue;
        yaBuscados.insert(clave);

        for (int id : buscarParcial(porGenero, palabras)) {
            coincidencias[id]++;
        }
    }
    return coincidencias;
}
