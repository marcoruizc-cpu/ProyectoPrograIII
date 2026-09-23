#ifndef SUFFIX_TRIE_H
#define SUFFIX_TRIE_H

#include <string>
#include <map>
#include <vector>

// Trie de sufijos a nivel de PALABRA (no del documento completo).
// Justificacion (ver conversacion previa / documentar en el repo):
// - Un Suffix Trie sobre el texto completo tiene O(L^2) nodos en el peor
//   caso; con decenas de miles de sinopsis eso no entra en memoria.
// - Insertando los sufijos de cada palabra por separado, el costo es
//   O(m^2) por palabra (m = longitud de la palabra, tipicamente < 20),
//   y el arbol comparte ramas entre palabras con sufijos comunes.
// - Trade-off aceptado: encuentra coincidencias DENTRO de una palabra,
//   no substrings que cruzan el limite entre dos palabras. El enunciado
//   no pide ese caso (sus ejemplos son "bar" en "barco", o "barco" +
//   "fantasma" como palabras separadas que se unen).
//
// Optimizacion de memoria (medida sobre el dataset completo):
// - El arbol se construye sobre el VOCABULARIO (palabras distintas, ~138 mil)
//   y cada nodo guarda IDs de PALABRA, no de pelicula. La relacion
//   palabra -> peliculas vive en IndiceBusqueda (indice invertido). Antes
//   cada nodo guardaba IDs de pelicula: 68 M de enteros; ahora ~2.8 M.
// - LARGO_MINIMO: los nodos de profundidad 1 y 2 no guardan IDs (serian las
//   listas mas grandes y un patron de 1-2 letras no es una busqueda util).
//   Los patrones cortos se resuelven como palabra exacta en IndiceBusqueda.
class SuffixTrie {
public:
    static const size_t LARGO_MINIMO = 3;

private:
private:
    struct Nodo {
        std::map<char, Nodo*> hijos; // arbol balanceado (S6), no tabla hash
        // vector en vez de unordered_set: la garantia de que jamas se
        // inserta un id duplicado la da 'ultimoIdInsertado' (ver abajo),
        // asi que no hace falta pagar el costo de hashing de un set solo
        // para deduplicar. push_back es O(1) amortizado.
        std::vector<int> idsPalabras;
        // Ultimo idPalabra insertado en este nodo. Cada palabra se inserta
        // una sola vez y todos sus sufijos seguidos, asi que el PRIMER toque
        // de un nodo por una palabra es el unico que necesita insertar; los
        // toques siguientes de la misma palabra (sufijos que comparten
        // ramas, p. ej. "ana" dentro de "banana") se detectan con una
        // simple comparacion de enteros.
        int ultimoIdInsertado = -1;
    };

    Nodo* raiz;

    // Inserta el sufijo de 'palabra' que empieza en 'inicio', SIN copiarlo
    // a un string nuevo (evita substr()). Recorrer por indices en vez de
    // por substrings es lo que hace viable indexar ~35,000 sinopsis: con
    // substr(), cada uno de los m sufijos de una palabra reserva memoria
    // para una copia de hasta m caracteres -- multiplicado por millones
    // de palabras, esa asignacion domina el tiempo de ejecucion.
    void insertarSufijoDesde(const std::string& palabra, size_t inicio, int idPalabra) {
        Nodo* actual = raiz;
        for (size_t j = inicio; j < palabra.size(); j++) {
            char c = palabra[j];
            auto it = actual->hijos.find(c);
            if (it == actual->hijos.end()) {
                Nodo* nuevo = new Nodo();
                actual->hijos[c] = nuevo;
                actual = nuevo;
            } else {
                actual = it->second;
            }
            size_t profundidad = j - inicio + 1;
            if (profundidad >= LARGO_MINIMO && actual->ultimoIdInsertado != idPalabra) {
                actual->idsPalabras.push_back(idPalabra);
                actual->ultimoIdInsertado = idPalabra;
            }
        }
    }

    void liberarNodo(Nodo* nodo) {
        if (nodo == nullptr) return;
        for (auto& par : nodo->hijos) {
            liberarNodo(par.second);
        }
        delete nodo;
    }

    Nodo* copiarNodo(const Nodo* origen) {
        if (origen == nullptr) return nullptr;
        Nodo* nuevo = new Nodo();
        nuevo->idsPalabras = origen->idsPalabras;
        nuevo->ultimoIdInsertado = origen->ultimoIdInsertado;
        for (const auto& par : origen->hijos) {
            nuevo->hijos[par.first] = copiarNodo(par.second);
        }
        return nuevo;
    }

public:
    SuffixTrie() : raiz(new Nodo()) {}

    ~SuffixTrie() {
        liberarNodo(raiz);
    }

    // Rule of Five: los nodos son memoria dinamica manejada a mano (new/delete),
    // igual que en Tensor++, asi que copia y movimiento deben definirse explicitamente.
    SuffixTrie(const SuffixTrie& otro) {
        raiz = copiarNodo(otro.raiz);
    }

    SuffixTrie& operator=(const SuffixTrie& otro) {
        if (this != &otro) {
            liberarNodo(raiz);
            raiz = copiarNodo(otro.raiz);
        }
        return *this;
    }

    SuffixTrie(SuffixTrie&& otro) noexcept : raiz(otro.raiz) {
        otro.raiz = nullptr;
    }

    SuffixTrie& operator=(SuffixTrie&& otro) noexcept {
        if (this != &otro) {
            liberarNodo(raiz);
            raiz = otro.raiz;
            otro.raiz = nullptr;
        }
        return *this;
    }

    // Inserta todos los sufijos de 'palabra' asociandolos a idPalabra.
    // Complejidad: O(m^2), m = palabra.size().
    void insertarPalabra(const std::string& palabra, int idPalabra) {
        for (size_t i = 0; i < palabra.size(); i++) {
            insertarSufijoDesde(palabra, i, idPalabra);
        }
    }

    // Busca un patron (palabra completa o substring) y devuelve los IDs de
    // las PALABRAS que lo contienen. Complejidad O(k), k = patron.size(),
    // independiente del tamano del corpus indexado.
    // Patrones mas cortos que LARGO_MINIMO devuelven vacio (no se indexan).
    std::vector<int> buscar(const std::string& patron) const {
        if (patron.size() < LARGO_MINIMO) return {};
        Nodo* actual = raiz;
        for (char c : patron) {
            auto it = actual->hijos.find(c);
            if (it == actual->hijos.end()) {
                return {};
            }
            actual = it->second;
        }
        return actual->idsPalabras;
    }
};

#endif
