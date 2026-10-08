#include "app/Plataforma.h"
#include <algorithm> // std::find
#include "datos/LectorCSV.h"
#include "busqueda/Ranking.h"

size_t Plataforma::cargarPeliculas(const std::string& rutaCSV) {
    // leerCSV devuelve un temporal que se mueve a construirPeliculas y se
    // destruye al terminar esta linea: las filas crudas no quedan en memoria.
    peliculas = construirPeliculas(leerCSV(rutaCSV));
    return peliculas.size();
}

void Plataforma::construirIndiceTexto() {
    indiceTexto.indexar(peliculas);
}

void Plataforma::construirIndicesTags() {
    indiceTags.indexar(peliculas);
}

std::vector<int> Plataforma::buscarPorTexto(const std::string& consulta) const {
    std::vector<ResultadoTermino> resultados = indiceTexto.buscarPorTermino(consulta);
    return ordenarPorImportancia(consulta, resultados, peliculas, indiceTexto);
}

std::vector<int> Plataforma::buscarPorDirector(const std::string& nombre) const {
    return ordenarPorAnio(indiceTags.buscarPorDirector(nombre), peliculas);
}

std::vector<int> Plataforma::buscarPorActor(const std::string& nombre) const {
    return ordenarPorAnio(indiceTags.buscarPorActor(nombre), peliculas);
}

std::vector<int> Plataforma::buscarPorGeneros(const std::string& generos) const {
    return ordenarPorCoincidencias(indiceTags.buscarPorGeneros(generos), peliculas);
}

const Pelicula& Plataforma::obtenerPelicula(int id) const {
    return peliculas[id];
}

// Agrega 'id' a 'lista' si no estaba. Devuelve false si ya estaba.
static bool agregarSinRepetir(std::vector<int>& lista, int id) {
    if (std::find(lista.begin(), lista.end(), id) != lista.end()) return false;
    lista.push_back(id);
    return true;
}

bool Plataforma::darLike(int id) {
    return agregarSinRepetir(likes, id);
}

bool Plataforma::agregarAVerMasTarde(int id) {
    return agregarSinRepetir(verMasTarde, id);
}

// Elimina una pelicula de la lista. Devuelve false si no estaba.
static bool quitarSiExiste(std::vector<int>& lista, int id) {
    auto it = std::find(lista.begin(), lista.end(), id);
    if (it == lista.end()) return false;
    lista.erase(it);
    return true;
}

bool Plataforma::quitarLike(int id) {
    return quitarSiExiste(likes, id);
}

bool Plataforma::quitarDeVerMasTarde(int id) {
    return quitarSiExiste(verMasTarde, id);
}

bool Plataforma::tieneLike(int id) const {
    return std::find(likes.begin(), likes.end(), id) != likes.end();
}

bool Plataforma::estaEnVerMasTarde(int id) const {
    return std::find(verMasTarde.begin(), verMasTarde.end(), id) != verMasTarde.end();
}

const std::vector<int>& Plataforma::obtenerLikes() const {
    return likes;
}

const std::vector<int>& Plataforma::obtenerVerMasTarde() const {
    return verMasTarde;
}
