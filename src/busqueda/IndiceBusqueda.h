#ifndef INDICE_BUSQUEDA_H
#define INDICE_BUSQUEDA_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include "modelo/Pelicula.h"
#include "estructuras/SuffixTrie.h"

// Indice de texto (titulo + sinopsis) en dos niveles:
//
//   consulta --> SuffixTrie (vocabulario) --> IDs de palabras
//            --> peliculasPorPalabra     --> IDs de peliculas
//
// 1. 'arbol': Suffix Trie construido sobre las palabras DISTINTAS del
//    corpus. Resuelve palabra completa y sub-palabra ("bar" -> barco,
//    embarcar) en O(k).
// 2. 'peliculasPorPalabra': indice invertido. La posicion i guarda las
//    peliculas donde aparece la palabra con id i (ids densos 0..V-1, por
//    eso basta un vector en vez de otro map).
// 3. 'idPorPalabra': palabra -> id, para resolver consultas mas cortas que
//    SuffixTrie::LARGO_MINIMO como palabra exacta ("up", "it").
//
// Stopwords (conectores en ingles, idioma del dataset): se descartan de las
// sinopsis y no entran al arbol, pero SI se indexan en los titulos para que
// peliculas como "It" o "Up" se puedan encontrar (ver buscarPorTermino()).
// Resultado de buscar UN termino de la consulta.
struct ResultadoTermino {
    std::string termino;       // palabra ya normalizada ("barco")
    bool esExacto = false;     // true: se busco como palabra exacta (stopword o < 3 letras)
    std::set<int> peliculas;   // peliculas que contienen el termino
};

class IndiceBusqueda {
private:
    SuffixTrie arbol;
    std::map<std::string, int> idPorPalabra;
    std::vector<std::vector<int>> peliculasPorPalabra;

    // Registra que 'palabra' aparece en la pelicula idPelicula (crea la
    // palabra en el vocabulario si es nueva).
    void registrarPalabra(const std::string& palabra, int idPelicula);

    // Agrega los IDs de peliculas de la palabra idPalabra a 'resultado'.
    void agregarPeliculasDe(int idPalabra, std::set<int>& resultado) const;

    // Agrega las peliculas de 'palabra' solo si coincide EXACTAMENTE con una
    // palabra del vocabulario (sin buscar sub-palabras).
    void agregarPalabraExacta(const std::string& palabra, std::set<int>& resultado) const;

public:
    // Tokeniza al vuelo titulo y sinopsis de cada pelicula y construye los
    // tres niveles del indice.
    void indexar(const std::vector<Pelicula>& peliculas);

    // Busca una consulta de texto (palabra, sub-palabra o frase) y devuelve,
    // POR CADA TERMINO usado, las peliculas que lo contienen. El ranking
    // necesita saber que termino encontro cada pelicula (una pelicula que
    // tiene "barco" Y "fantasma" es mas importante que una con solo uno).
    // - Una palabra ("barco") o sub-palabra ("bar"): la resuelve el arbol.
    // - Varias palabras ("barco fantasma"): un resultado por palabra; la
    //   UNION es el resultado final, como pide el enunciado ("y/o").
    // - Stopwords: se ignoran si la consulta tiene otras palabras; si la
    //   consulta es SOLO stopwords ("it"), se buscan como palabra exacta.
    std::vector<ResultadoTermino> buscarPorTermino(const std::string& consulta) const;

    // true si 'palabra' aparece como palabra COMPLETA en el titulo o la
    // sinopsis de la pelicula. Las listas del indice invertido estan
    // ordenadas por id (se llenan en ese orden), asi que se usa busqueda
    // binaria (S6): O(log n).
    bool contienePalabra(const std::string& palabra, int idPelicula) const;
};

#endif
