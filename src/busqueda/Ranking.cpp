#include "busqueda/Ranking.h"
#include <algorithm> // std::sort
#include <set>
#include <utility>   // std::pair
#include "datos/Texto.h"   // normalizarTexto, tokenizar

static const int PUNTOS_TITULO_EXACTO = 100;
static const int PUNTOS_POR_TERMINO = 10;
static const int PUNTOS_PALABRA_EN_TITULO = 30;
static const int PUNTOS_SUBPALABRA_EN_TITULO = 15;
static const int PUNTOS_PALABRA_EN_SINOPSIS = 5;

// Une palabras con un espacio: {"to", "be"} -> "to be".
static std::string unir(const std::vector<std::string>& palabras) {
    std::string resultado;
    for (size_t i = 0; i < palabras.size(); i++) {
        if (i > 0) resultado += ' ';
        resultado += palabras[i];
    }
    return resultado;
}

// Puntaje de UNA pelicula para la consulta. El titulo se tokeniza al vuelo
// (es corto); la sinopsis NO se recorre: se consulta el indice invertido con
// busqueda binaria.
static int puntuar(const Pelicula& p, const std::string& consultaNormalizada,
                   const std::vector<ResultadoTermino>& resultados, const IndiceBusqueda& indice) {
    std::vector<std::string> palabrasTitulo = tokenizar(normalizarTexto(p.titulo));

    int puntaje = 0;
    if (unir(palabrasTitulo) == consultaNormalizada) {
        puntaje += PUNTOS_TITULO_EXACTO;
    }

    for (const ResultadoTermino& r : resultados) {
        if (r.peliculas.count(p.id) == 0) continue; // esta pelicula no tiene este termino
        puntaje += PUNTOS_POR_TERMINO;

        bool palabraEnTitulo = false;
        bool subpalabraEnTitulo = false;
        for (const std::string& palabra : palabrasTitulo) {
            if (palabra == r.termino) {
                palabraEnTitulo = true;
            } else if (!r.esExacto && palabra.find(r.termino) != std::string::npos) {
                subpalabraEnTitulo = true;
            }
        }

        if (palabraEnTitulo) {
            puntaje += PUNTOS_PALABRA_EN_TITULO;
        } else if (subpalabraEnTitulo) {
            puntaje += PUNTOS_SUBPALABRA_EN_TITULO;
        } else if (indice.contienePalabra(r.termino, p.id)) {
            // No esta completa en el titulo, asi que esta completa en la sinopsis.
            puntaje += PUNTOS_PALABRA_EN_SINOPSIS;
        }
    }
    return puntaje;
}

std::vector<int> ordenarPorImportancia(const std::string& consulta,
                                       const std::vector<ResultadoTermino>& resultados,
                                       const std::vector<Pelicula>& peliculas,
                                       const IndiceBusqueda& indice) {
    // Union de los resultados de cada termino ("barco" y/o "fantasma").
    std::set<int> candidatos;
    for (const ResultadoTermino& r : resultados) {
        candidatos.insert(r.peliculas.begin(), r.peliculas.end());
    }

    std::string consultaNormalizada = unir(tokenizar(normalizarTexto(consulta)));

    std::vector<std::pair<int, int>> puntuados; // {puntaje, idPelicula}
    puntuados.reserve(candidatos.size());
    for (int id : candidatos) {
        puntuados.push_back({puntuar(peliculas[id], consultaNormalizada, resultados, indice), id});
    }

    // Lambda como criterio de orden (S4): mayor puntaje primero; si empatan,
    // la mas reciente; si tambien empatan, el orden original del CSV.
    std::sort(puntuados.begin(), puntuados.end(),
              [&peliculas](const std::pair<int, int>& a, const std::pair<int, int>& b) {
                  if (a.first != b.first) return a.first > b.first;
                  int anioA = peliculas[a.second].anioEstreno;
                  int anioB = peliculas[b.second].anioEstreno;
                  if (anioA != anioB) return anioA > anioB;
                  return a.second < b.second;
              });

    std::vector<int> ordenados;
    ordenados.reserve(puntuados.size());
    for (const auto& par : puntuados) {
        ordenados.push_back(par.second);
    }
    return ordenados;
}

std::vector<int> ordenarPorAnio(const std::vector<int>& ids, const std::vector<Pelicula>& peliculas) {
    std::vector<int> ordenados = ids;
    std::sort(ordenados.begin(), ordenados.end(), [&peliculas](int a, int b) {
        if (peliculas[a].anioEstreno != peliculas[b].anioEstreno) {
            return peliculas[a].anioEstreno > peliculas[b].anioEstreno;
        }
        return a < b;
    });
    return ordenados;
}
