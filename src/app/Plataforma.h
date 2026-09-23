#ifndef PLATAFORMA_H
#define PLATAFORMA_H

#include <string>
#include <vector>
#include "modelo/Pelicula.h"
#include "busqueda/IndiceBusqueda.h"
#include "busqueda/IndiceTags.h"

// Nucleo de la plataforma de streaming: es duena del catalogo de peliculas,
// de los indices y de las listas del usuario, y expone las operaciones del
// enunciado. NO hace entrada/salida por consola: eso queda en main.cpp
// (interfaz), de modo que la logica se puede probar y ampliar sin tocar el menu.
class Plataforma {
private:
    std::vector<Pelicula> peliculas;
    IndiceBusqueda indiceTexto;
    IndiceTags indiceTags;

    // Listas del usuario (F12). vector y no set: se muestran en el orden en
    // que el usuario las agrego. Son listas cortas, asi que buscar un id con
    // std::find (O(n)) es suficiente.
    std::vector<int> likes;
    std::vector<int> verMasTarde;

public:
    // Lee el CSV y construye el catalogo. Devuelve la cantidad de peliculas.
    size_t cargarPeliculas(const std::string& rutaCSV);

    // Se separan de cargarPeliculas para que la interfaz pueda informar el
    // avance de cada etapa.
    void construirIndiceTexto();
    void construirIndicesTags();

    // Busquedas. Devuelven los IDs YA ORDENADOS (F10); la interfaz los
    // muestra de 5 en 5 (F9).
    // - Texto: por importancia (ver Ranking).
    // - Director / actor: nombre completo o parcial; las mas recientes primero.
    // - Generos: uno o varios ("drama, comedy"); primero las que comparten
    //   mas generos, luego las mas recientes.
    std::vector<int> buscarPorTexto(const std::string& consulta) const;
    std::vector<int> buscarPorDirector(const std::string& nombre) const;
    std::vector<int> buscarPorActor(const std::string& nombre) const;
    std::vector<int> buscarPorGeneros(const std::string& generos) const;

    const Pelicula& obtenerPelicula(int id) const;

    // Like y Ver mas tarde (F12). Devuelven false si la pelicula ya estaba.
    bool darLike(int id);
    bool agregarAVerMasTarde(int id);
    bool tieneLike(int id) const;
    bool estaEnVerMasTarde(int id) const;
    const std::vector<int>& obtenerLikes() const;
    const std::vector<int>& obtenerVerMasTarde() const;
};

#endif
