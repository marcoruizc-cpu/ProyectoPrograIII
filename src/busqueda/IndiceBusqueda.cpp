#include "busqueda/IndiceBusqueda.h"
#include <algorithm> // std::binary_search
#include "datos/Texto.h" // normalizarTexto, tokenizar, esStopword

void IndiceBusqueda::registrarPalabra(const std::string& palabra, int idPelicula) {
    int idPalabra;
    auto it = idPorPalabra.find(palabra);
    if (it == idPorPalabra.end()) {
        idPalabra = static_cast<int>(peliculasPorPalabra.size());
        idPorPalabra[palabra] = idPalabra;
        peliculasPorPalabra.push_back({});
    } else {
        idPalabra = it->second;
    }

    // Las peliculas se procesan en orden de id, asi que basta mirar el
    // ultimo elemento para no repetir la misma pelicula.
    std::vector<int>& lista = peliculasPorPalabra[idPalabra];
    if (lista.empty() || lista.back() != idPelicula) {
        lista.push_back(idPelicula);
    }
}

void IndiceBusqueda::indexar(const std::vector<Pelicula>& peliculas) {
    // Paso 1: vocabulario + indice invertido, recorriendo cada pelicula una
    // sola vez. Los tokens son temporales: se liberan al pasar a la siguiente.
    for (const auto& p : peliculas) {
        // Titulo: se indexan TODAS sus palabras, incluidas las stopwords.
        // Sin esto, 28 peliculas cuyo titulo es solo stopwords ("It", "Up",
        // "Her", "To Be or Not to Be") serian imposibles de encontrar.
        for (const auto& palabra : tokenizar(normalizarTexto(p.titulo))) {
            registrarPalabra(palabra, p.id);
        }
        // Sinopsis: las stopwords se descartan (son el grueso del volumen y
        // solo agregan ruido).
        for (const auto& palabra : tokenizar(normalizarTexto(p.sinopsis))) {
            if (!esStopword(palabra)) {
                registrarPalabra(palabra, p.id);
            }
        }
    }

    // Paso 2: cada palabra DISTINTA entra una sola vez al arbol de sufijos.
    // Las stopwords no entran: solo se buscan como palabra exacta (ver buscar).
    for (const auto& par : idPorPalabra) {
        if (!esStopword(par.first)) {
            arbol.insertarPalabra(par.first, par.second);
        }
    }
}

void IndiceBusqueda::agregarPeliculasDe(int idPalabra, std::set<int>& resultado) const {
    const std::vector<int>& lista = peliculasPorPalabra[idPalabra];
    resultado.insert(lista.begin(), lista.end());
}

void IndiceBusqueda::agregarPalabraExacta(const std::string& palabra, std::set<int>& resultado) const {
    auto it = idPorPalabra.find(palabra);
    if (it != idPorPalabra.end()) {
        agregarPeliculasDe(it->second, resultado);
    }
}

std::vector<ResultadoTermino> IndiceBusqueda::buscarPorTermino(const std::string& consulta) const {
    std::vector<std::string> palabras = tokenizar(normalizarTexto(consulta));

    // Se separan las palabras significativas de las stopwords.
    std::vector<std::string> significativas;
    for (const auto& palabra : palabras) {
        if (!esStopword(palabra)) significativas.push_back(palabra);
    }

    // Consulta formada SOLO por stopwords ("it", "up", "to be or not to be"):
    // se buscan como palabra exacta. Solo estan indexadas en los titulos, asi
    // que "it" encuentra la pelicula "It" y no todo lo que contiene "city".
    // Con al menos una palabra significativa ("the ghost"), las stopwords se
    // ignoran para no inundar el resultado.
    bool soloStopwords = significativas.empty();
    const std::vector<std::string>& terminos = soloStopwords ? palabras : significativas;

    std::vector<ResultadoTermino> resultados;
    std::set<std::string> yaBuscados; // "to be or not to be": "to" y "be" una sola vez
    for (const auto& termino : terminos) {
        if (yaBuscados.count(termino)) continue;
        yaBuscados.insert(termino);

        ResultadoTermino r;
        r.termino = termino;
        r.esExacto = soloStopwords || termino.size() < SuffixTrie::LARGO_MINIMO;
        if (r.esExacto) {
            agregarPalabraExacta(termino, r.peliculas);
        } else {
            for (int idPalabra : arbol.buscar(termino)) {
                agregarPeliculasDe(idPalabra, r.peliculas);
            }
        }
        resultados.push_back(r);
    }
    return resultados;
}

bool IndiceBusqueda::contienePalabra(const std::string& palabra, int idPelicula) const {
    auto it = idPorPalabra.find(palabra);
    if (it == idPorPalabra.end()) return false;
    const std::vector<int>& lista = peliculasPorPalabra[it->second];
    return std::binary_search(lista.begin(), lista.end(), idPelicula);
}
