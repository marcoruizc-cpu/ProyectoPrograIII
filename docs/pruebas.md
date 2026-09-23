# Pruebas de aceptación — Entrega 1

Salidas obtenidas con el dataset completo (`dataset/wiki_movie_plots_deduped.csv`). Compilar en modo **Release**. Validado en Linux (g++ 13, -O2) y en Windows (CLion, Release): **29/29 pruebas aprobadas**.

Notación: `menú → entrada` (cada entrada va seguida de Enter). `x` vuelve atrás.

## Carga

| ID | Entrada | Salida esperada | Resultado |
|---|---|---|---|
| T01 | Iniciar el programa | `Peliculas cargadas: 34886` y `Listo. Base de datos indexada.` Sin mensajes de "Aviso". Tiempo de referencia: 10–30 s | OK |

## Búsqueda por texto (opción 1)

| ID | Entrada | Salida esperada | Qué valida | Resultado |
|---|---|---|---|---|
| T02 | `barco` | `Resultados 1-2 de 2` | Palabra (F5) | OK |
| T03 | `bar` | `de 7635`; [1] Small Town Gay Bar (2006), [2] Gaz Bar Blues (2003) | Sub-palabra (F7) | OK |
| T04 | `embarc` | `de 2`: The Lineup (1958), It Came from Beneath the Sea (1955) | Sub-palabra en medio de una palabra (F7) | OK |
| T05 | `ghost ship` | `de 9141`; [1] Ghost Ship (2002), [2] Ghost Ship (1952), [3] The Ghost Ship (1943) | Frase + ranking (F6, F10) | OK |
| T06 | `the ghost` | `de 880`; [1] The Ghost (2008) | Stopwords ignoradas en frase | OK |
| T07 | `it` | `de 188`; [1] It (2017), [2] It (1990), [3] It! (1967), [4] It (1927) | Solo stopwords + título exacto (F5, F10) | OK |
| T08 | en T07: `v` | `Resultados 6-10 de 188`; [7] It's Only the End of the World… (2016) | Ver siguientes 5 (F9) | OK |
| T09 | en la página 6-10: `3`, luego `7` | `3` → `Opcion invalida.`; `7` → abre *It's Only the End of the World* | Solo números de la página visible | OK |
| T10 | `Up` | `de 144`; [1] Up (2009) | Título de 2 letras que es stopword | OK |
| T11 | `pokemon` | `de 20`; [1] Pokémon the Movie 20: I Choose You! (2017) | Plegado de tildes (F2) | OK |
| T12 | `amelie` y luego `Amélie` | Ambas: `de 2`, Le Divorce (2003), We're No Angels (1955) | Con y sin tilde dan lo mismo | OK |
| T13 | `xyzqw` | `(sin coincidencias)` | Sin resultados | OK |
| T14 | `barco`, luego `v` | `No hay mas resultados.` | Límite de la paginación | OK |

## Detalle, Like y Ver más tarde

| ID | Entrada | Salida esperada | Qué valida | Resultado |
|---|---|---|---|---|
| T15 | opción 1 → `there's nothing out there` → `1` | [1] There's Nothing Out There (1991); la sinopsis termina en "…fight against the monster." **sin** `[1][2]` | Detalle (F11) + limpieza de citas (F2) | OK |
| T16 | en el detalle: `1`, `2`, `1`, `2`, `0` | `Le diste Like a…` → `Agregada a Ver mas tarde:…` → `Ya le habias dado Like a…` → `Ya estaba en Ver mas tarde:…` → vuelve a la lista | Like / Ver más tarde sin duplicados (F12) | OK |
| T17 | opción 5 | Ambas listas muestran `There's Nothing Out There (1991)` | Mis listas | OK |

## Búsqueda por tag (opciones 2, 3 y 4)

| ID | Entrada | Salida esperada | Qué valida | Resultado |
|---|---|---|---|---|
| T18 | 2 → `steven spielberg` | `de 30`; [1] The Post (2017) | Director, ordenado por año (F8) | OK |
| T19 | 2 → `francois truffaut` | `de 1`: Fahrenheit 451 (1966) | Tildes en tags (François) | OK |
| T20 | 2 → `wallace mccutcheon` | `de 4`; [1] Daniel Boone (1907) | Directores unidos con " and " | OK |
| T21 | 3 → `tom hanks` | `de 43`; [1] The Circle (2017) | Actor | OK |
| T22 | 4 → `comedy` | `de 5802` | Géneros combinados separados | OK |
| T23 | 4 → `sci-fi` | `de 361`; [1] Monster Trucks (2017) | El guion no separa | OK |
| T24 | 4 → `unknown` | `(sin coincidencias)` | "unknown" descartado | OK |

## Robustez

| ID | Entrada | Salida esperada | Resultado |
|---|---|---|---|
| T25 | menú: `abc`, luego `9` | `Opcion invalida.` en ambos casos, y el menú vuelve a aparecer | OK |
| T26 | en una lista de resultados: `999`, `hola`, `0` | `Opcion invalida.` en los tres casos | OK |
| T27 | menú: `6` | `Hasta luego.` y el programa termina | OK |

## Solo en Windows

| ID | Entrada | Salida esperada | Resultado |
|---|---|---|---|
| T28 | T11 (`pokemon`) | Se ve `Pokémon`, no `PokÃ©mon` | OK |
| T29 | T12 escribiendo `Amélie` con tilde | Mismo resultado que `amelie` | OK |
