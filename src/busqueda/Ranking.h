#ifndef RANKING_H
#define RANKING_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include "modelo/Pelicula.h"
#include "busqueda/IndiceBusqueda.h"

// Algoritmo de importancia (F10). Asigna un puntaje a cada pelicula
// encontrada y las ordena de mayor a menor. Detalle y justificacion de los
// pesos en docs/ranking.md.
//
//   +100  el titulo completo es igual a la consulta        ("It" para "it")
//   + 10  por cada termino de la consulta que contiene     (cobertura: "barco" Y "fantasma")
//   + 30  si ese termino es una palabra COMPLETA del titulo
//   + 15  si no, si es una SUB-PALABRA del titulo          ("bar" en "Crowbar")
//   +  5  si no, si es una palabra completa de la sinopsis
//
// Empates: primero la mas reciente; luego el orden del CSV.
std::vector<int> ordenarPorImportancia(const std::string& consulta,
                                       const std::vector<ResultadoTermino>& resultados,
                                       const std::vector<Pelicula>& peliculas,
                                       const IndiceBusqueda& indice);

// Orden para busquedas por director o actor, donde no hay texto que
// comparar: las mas recientes primero.
std::vector<int> ordenarPorAnio(const std::set<int>& ids, const std::vector<Pelicula>& peliculas);

// Orden para busqueda por generos: primero las que comparten MAS generos de
// los pedidos; si empatan, las mas recientes.
std::vector<int> ordenarPorCoincidencias(const std::map<int, int>& coincidencias,
                                         const std::vector<Pelicula>& peliculas);

struct CandidataTop {
    int id;
    double puntaje;

    // Sobrecarga de '>' para construir el Min-Heap en std::priority_queue
    bool operator>(const CandidataTop& otra) const {
        return puntaje > otra.puntaje;
    }
};

class Ranking {
public:
    // Procesa N candidatas y retorna solo los IDs del Top 5 en O(N log 5)
    static std::vector<int> obtenerTop5Ids(const std::vector<CandidataTop>& candidatas);
};

#endif
