# Justificación del árbol: Suffix Trie por palabra

## Requerimientos que debe cumplir

El enunciado pide un **árbol** cuyos nodos almacenen caracteres y que permita buscar por:

| Tipo de búsqueda | Ejemplo del enunciado | Qué exige al árbol |
|---|---|---|
| Palabra | "barco" | Encontrar la palabra completa |
| Frase | "barco fantasma" | Unir los resultados de cada palabra ("y/o") |
| **Sub-palabra** | "bar" | Encontrar el patrón **en cualquier posición** de una palabra ("embarcar", "crowbar") |

El tercer caso es el que decide la estructura.

## Alternativas evaluadas

| Estructura | ¿"bar" encuentra "embarcar"? | Memoria | Veredicto |
|---|---|---|---|
| Búsqueda lineal (`find` en cada sinopsis) | Sí | Ninguna extra | Cada consulta recorre los 81 MB: O(N·L) por consulta |
| Trie de prefijos | **No**: solo palabras que *empiezan* con "bar" | O(Σ m) | No cumple el requerimiento |
| Suffix Trie del texto completo | Sí | O(L²) nodos por sinopsis: con L ≈ 2 300 bytes de promedio, millones de nodos por película | Imposible en memoria |
| Suffix Tree comprimido (Ukkonen) | Sí | O(L) | Algoritmo complejo, fuera del temario; además encuentra coincidencias entre palabras, que no se piden |
| **Suffix Trie por palabra, sobre el vocabulario** | **Sí** | O(V · m²) acotado (V palabras distintas, m ≈ 5–15) | **Elegido** |

## Idea

Un **Trie** guarda cadenas carácter por carácter: cada nodo es un carácter y cada camino desde la raíz es un prefijo. Si además de cada palabra se insertan **todos sus sufijos**, cualquier sub-palabra es el prefijo de algún sufijo, y basta un recorrido desde la raíz para encontrarla.

Sufijos de "barco": `barco`, `arco`, `rco`, `co`, `o`. La búsqueda "arc" recorre `a → r → c` y llega a un nodo que sabe que "barco" contiene "arc".

Se insertan los sufijos **de cada palabra por separado**, no del texto completo:

- El costo es O(m²) por palabra, con m pequeño, en vez de O(L²) por sinopsis.
- Trade-off aceptado: no se encuentran sub-cadenas que crucen dos palabras ("co fan" dentro de "barco fantasma"). El enunciado no lo pide: sus ejemplos son una palabra, una sub-palabra o palabras separadas.

## Diseño en dos niveles

```
consulta "bar"
   │
   ▼
SuffixTrie (vocabulario: 139 427 palabras distintas)
   │   recorrido b → a → r : O(k)
   ▼
IDs de PALABRAS que contienen "bar"  {barco, embarcar, crowbar, …}
   │
   ▼
Índice invertido  peliculasPorPalabra[idPalabra]
   │
   ▼
IDs de PELÍCULAS (unión en un std::set)
```

| Pieza | Tipo | Archivo | Tema |
|---|---|---|---|
| Árbol de sufijos | `SuffixTrie` (nodos en el heap) | `SuffixTrie.h` | S1 (memoria dinámica, Regla de 5), S5 (`map`) |
| Vocabulario | `std::map<std::string, int>` palabra → id | `IndiceBusqueda` | S5 |
| Índice invertido | `std::vector<std::vector<int>>` id → películas | `IndiceBusqueda` | S5 |

**Por qué dos niveles:** en la primera versión cada nodo guardaba IDs de *película*. Los nodos cercanos a la raíz ("e", "a", "t") terminaban con casi las 35 000 películas cada uno: 68 millones de enteros. Guardando IDs de *palabra*, cada palabra distinta entra una sola vez al árbol, y la relación palabra → películas se guarda una sola vez en el índice invertido.

### Nodo

```cpp
struct Nodo {
    std::map<char, Nodo*> hijos;   // hijos ordenados: búsqueda O(log σ)
    std::vector<int> idsPalabras;  // palabras que contienen el camino raíz→nodo
    int ultimoIdInsertado = -1;    // evita repetir una palabra en el mismo nodo
};
```

- `ultimoIdInsertado`: los sufijos de una misma palabra comparten ramas ("ana" dentro de "banana"). Como cada palabra se inserta completa antes de pasar a la siguiente, basta comparar con el último ID para no repetirlo. Es O(1) y no necesita un `set` por nodo.
- **Regla de 5:** los nodos se crean con `new`, así que `SuffixTrie` define destructor, constructor de copia, asignación por copia, constructor de movimiento y asignación por movimiento (copia profunda recursiva; el movimiento transfiere la raíz).

## Complejidad (S6)

Notación: k = largo del patrón, m = largo de una palabra, V = palabras distintas, T = palabras totales del corpus, σ = hijos posibles por nodo (≤ 256), W = palabras que contienen el patrón, R = IDs de películas acumulados.

| Operación | Complejidad | Comentario |
|---|---|---|
| Insertar una palabra | O(m² · log σ) | m sufijos de hasta m caracteres |
| Construir el índice | O(T · log V + V · m² · log σ) | Registrar cada token en el vocabulario + insertar cada palabra distinta |
| Recorrer el patrón | O(k · log σ) | **No depende del tamaño del corpus** |
| Buscar (total) | O(k · log σ + R · log R) | Recorrido + unión de resultados en el `set` |
| Palabra exacta o corta (< 3 letras) | O(k · log V) | Búsqueda directa en el vocabulario |

## Optimización de memoria (medida)

Todas las configuraciones se midieron sobre el dataset completo (34 886 películas, 13,4 M de palabras).

| Configuración | Nodos | IDs en el árbol | IDs en el índice invertido | Memoria del proceso |
|---|---|---|---|---|
| Versión inicial (IDs de película en cada nodo y tokens guardados en `Pelicula`) | — | — | — | **1,2 GB** |
| IDs de película en cada nodo, sin tokens guardados | 944 349 | 68 121 518 | — | 665 MB |
| Árbol sobre el vocabulario + índice invertido | 944 349 | 4 674 918 | 6 529 401 | 344 MB |
| + stopwords fuera de las sinopsis | 944 349 | 4 674 530 | 5 452 540 | 339 MB |
| **+ largo mínimo 3 (final)** | 944 349 | **2 930 223** | 5 452 540 | **~332–340 MB** |

- **Largo mínimo 3 (`SuffixTrie::LARGO_MINIMO`):** los nodos de profundidad 1 y 2 no guardan IDs, porque serían las listas más grandes y un patrón de 1–2 letras no es una búsqueda útil. Las consultas cortas ("up", "ed") se resuelven como palabra exacta en el vocabulario.
- Los 944 349 nodos (cada uno con su `map`) son el piso de memoria restante, unos 120 MB estimados.

## Medición empírica (S7)

Medido con `std::chrono` en la máquina de pruebas; los valores varían según el equipo.

| Etapa | Tiempo |
|---|---|
| Lectura y pre-procesamiento del CSV | 4,7 s |
| Construcción del índice de texto | 14,5 s |
| Índices por tag | 0,1 s |

| Consulta | Películas encontradas | Tiempo por consulta |
|---|---|---|
| "barco" | 2 | < 1 µs |
| "it" (solo stopword, palabra exacta) | 188 | 9 µs |
| "bar" (sub-palabra) | 7 635 | 1,5 ms |
| "ghost ship" (frase) | 9 141 | 1,5 ms |

La búsqueda es rápida incluso para patrones muy frecuentes: el costo lo domina reunir los resultados (R), no el recorrido del árbol.

## Limitaciones y trabajo futuro (entrega 2)

| Mejora | Beneficio | Costo |
|---|---|---|
| Hijos como `vector<pair<char, Nodo*>>` ordenado + búsqueda binaria | Menos memoria por nodo que `map` (~48 B por hijo) | Inserción O(σ) en el vector |
| IDs solo en el nodo donde termina cada sufijo | IDs en el árbol: de 2,9 M a ~1 M | Buscar pasa a O(k + tamaño del subárbol) |
| Compresión de caminos (radix tree) | Menos nodos | Mayor complejidad de implementación |
