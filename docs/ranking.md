# Algoritmo de importancia (ranking)

El enunciado pide mostrar primero las **cinco películas más importantes** de cada búsqueda y que el grupo implemente su propio algoritmo para decidirlo. Está implementado en `src/busqueda/Ranking.cpp`.

## Idea

Una película es más importante para una consulta cuanto **más términos** de la consulta contiene y cuanto **más visible** es la coincidencia: en el título pesa más que en la sinopsis, y una palabra completa pesa más que una sub-palabra.

## Puntaje

Para cada película encontrada:

| Condición | Puntos | Ejemplo |
|---|---|---|
| El título completo es igual a la consulta | +100 | "it" → *It* |
| Por cada término de la consulta que contiene (cobertura) | +10 | "barco fantasma": tener los dos términos suma 20 |
| …y ese término es una **palabra completa del título** | +30 | "ghost" en *The Ghost Ship* |
| …si no, es una **sub-palabra del título** | +15 | "bar" en *Crowbar* |
| …si no, es una **palabra completa de la sinopsis** | +5 | "ghost" en la sinopsis |
| …si no (sub-palabra dentro de la sinopsis) | +0 | solo cuenta la cobertura |

**Desempates:** primero la película más reciente; si también empatan, el orden del CSV.

### Por qué estos pesos

- **Título exacto (+100)** supera a cualquier combinación de los demás criterios. Quien escribe "it" busca la película *It*, no las 188 que tienen "it" en el título.
- **Cobertura (+10 por término)** respeta el "y/o" del enunciado: se muestran las películas con cualquiera de las palabras, pero las que tienen todas aparecen primero.
- **Título > sinopsis:** que la palabra esté en el título indica que la película *trata* de eso. En la sinopsis puede ser una mención de paso.
- **Palabra completa > sub-palabra:** "bar" como palabra ("Gay Bar") es más relevante que como fragmento ("Barcelona").
- **Más reciente en empates:** sin datos de popularidad en el dataset, la fecha es un criterio neutral y fácil de explicar.

## Resultados obtenidos

| Consulta | Top 3 |
|---|---|
| "it" | *It* (2017), *It* (1990), *It!* (1967) |
| "ghost ship" | *Ghost Ship* (2002), *Ghost Ship* (1952), *The Ghost Ship* (1943) |
| "the ghost" | *The Ghost* (2008), *Ghost in the Shell* (2017), *A Ghost Story* (2017) |
| "bar" | *Small Town Gay Bar* (2006), *Gaz Bar Blues* (2003), *Big and Little Wong Tin Bar* (1962) |

## Implementación

1. `IndiceBusqueda::buscarPorTermino` devuelve, **por cada término**, el `set` de películas que lo contienen. Así se sabe qué término encontró cada película.
2. `ordenarPorImportancia` une esos conjuntos y calcula el puntaje de cada candidata:
   - el título se tokeniza al vuelo, porque es corto;
   - la sinopsis **no se recorre**: `IndiceBusqueda::contienePalabra` consulta el índice invertido con **búsqueda binaria** (`std::binary_search`). Las listas ya están ordenadas por ID porque se llenan en ese orden.
3. `std::sort` con una **lambda** como criterio (puntaje, año, ID).

Las búsquedas por tag (director, actor, género) no tienen texto que comparar, así que `ordenarPorAnio` las ordena de la más reciente a la más antigua.

## Complejidad (S6)

Notación: R = películas candidatas, t = términos de la consulta, a = palabras del título, n = películas de la lista de un término.

| Paso | Complejidad |
|---|---|
| Puntuar una película | O(t · (a + log R + log n)) |
| Puntuar todas | O(R · t · (a + log R + log n)) |
| Ordenar | O(R log R) |

Medición empírica, búsqueda + ranking:

| Consulta | Candidatas | Tiempo |
|---|---|---|
| "barco" | 2 | 2 µs |
| "it" | 188 | 0,2 ms |
| "bar" | 7 635 | 9,7 ms |
| "ghost ship" | 9 141 | 12,6 ms |

## Temas del curso

| Tema | Semana | Uso |
|---|---|---|
| Lambdas y `<algorithm>` (`sort`, `find`) | S4 | Criterio de orden, listas del usuario |
| `set`, `vector`, `pair` | S5 | Candidatos, puntajes |
| Búsqueda binaria y Big O | S6 | `contienePalabra`, análisis de costo |

## Mejoras para la entrega 2

- **Top 5 con `priority_queue`** (6.6.2): obtener los 5 mejores cuesta O(R log 5) en lugar de ordenar todo en O(R log R).
- **Frecuencia del término (TF-IDF):** premiar las palabras que se repiten en la sinopsis y las que son poco comunes en el corpus.
- **Stemming solo para puntuar:** que "running" también sume para "run".
- **Likes del usuario:** subir las películas parecidas a las que le gustaron (se conecta con F14).
