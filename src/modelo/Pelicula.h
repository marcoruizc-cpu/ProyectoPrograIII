#ifndef PELICULA_H
#define PELICULA_H

#include <string>
#include <vector>

struct Pelicula {
    int id = -1;
    int anioEstreno = 0;
    std::string titulo;
    std::string origen;
    std::string director;                    // texto original, para mostrar
    std::vector<std::string> directores;      // separado por coma, para indexar por tag
    std::vector<std::string> reparto;
    std::string genero;                      // texto original, para mostrar
    std::vector<std::string> generos;         // generos atomicos, para indexar por tag
    std::string wikiPagina;
    std::string sinopsis;

    // Los tokens de titulo/sinopsis ya NO se guardan aqui: eran 13.4 M de
    // std::string (~430 MB). IndiceBusqueda tokeniza al vuelo al indexar.

    // Like y Ver mas tarde NO se guardan aqui: son datos del usuario, no de
    // la pelicula. Viven en Plataforma (listas likes / verMasTarde).
};

#endif
