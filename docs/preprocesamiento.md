# Pre-procesamiento de datos

Todo el pre-procesamiento está implementado en C++ (sin scripts externos), como exige el enunciado. Se aplica **una sola vez al cargar** el CSV y la **misma normalización** se aplica a cada consulta del usuario, para que índice y consulta hablen el mismo "idioma".

Dataset: `dataset/wiki_movie_plots_deduped.csv` — 34 886 películas, 81 MB, 8 columnas (`Release Year, Title, Origin/Ethnicity, Director, Cast, Genre, Wiki Page, Plot`), 13,4 millones de palabras y 139 427 palabras distintas.

## Resumen del pipeline

| # | Etapa | Dónde | Problema que resuelve (medido en el dataset) |
|---|---|---|---|
| 1 | Parseo CSV con comillas | `LectorCSV::leerCSV` | Sinopsis con comas, comillas `""` y saltos de línea dentro del campo |
| 2 | Validación de filas | `construirPeliculas` | Filas con ≠ 8 columnas se descartan y se reporta cuántas |
| 3 | Separación de listas | `separarLista` (LectorCSV) | Director/Cast/Género unidos con `, / ; & – —` y " and " |
| 4 | Descarte de "unknown" | `separarLista` | 6 083 géneros y 1 124 directores "unknown" |
| 5 | Limpieza de marcas de cita | `quitarMarcasDeCita` (Texto) | 4 125 sinopsis con `[12]`, `[a]`, `[Note 1]`, `[citation needed]` |
| 6 | Minúsculas + plegado de tildes | `normalizarTexto` (Texto) | 10 535 filas (30 %) con caracteres no ASCII |
| 7 | Puntuación Unicode → separador | `normalizarTexto` | `’` (9 550), `—` (4 515), `–` (2 943), comillas `“ ”` |
| 8 | Tokenización | `tokenizar` (Texto) | Separar el texto en palabras |
| 9 | Stopwords | `esStopword` (Texto) + `IndiceBusqueda` | Las 50 palabras más frecuentes son el 41,5 % del texto |
| 10 | Normalización de tags | `normalizarTag` (Texto) | "François Truffaut" = "francois truffaut" |

## 1–2. Lectura del CSV

**Problema:** la columna `Plot` contiene comas, comillas escapadas (`""`) y saltos de línea. Leer línea por línea con `getline(..., ',')` corta las sinopsis en pedazos.

**Solución:** `leerCSV` es una máquina de dos estados (`dentroDeComillas`) que lee carácter por carácter:

- dentro de comillas, la coma y el salto de línea son texto literal;
- `""` dentro de comillas es una comilla escapada;
- fuera de comillas, `,` cierra el campo y `\n` cierra la fila (se quita el `\r` de Windows).

Las filas con un número de columnas distinto de 8 se descartan y se informa cuántas. El año se convierte con `istringstream` revisando el estado de fallo (sin `try/catch`, que no está en el temario).

**Memoria (semántica de movimiento, S1):** `construirPeliculas` recibe las filas **por valor** y se llama con un temporal: `construirPeliculas(leerCSV(ruta))`. Cada campo se transfiere con `std::move` a la `Pelicula`, así el texto crudo no queda duplicado y las filas se liberan al terminar la carga.

## 3–4. Normalización de campos (tags)

**Problema:**

- Los géneros vienen combinados: "comedy, drama", "romantic comedy/drama", "comedy–drama", "drama; comedy". Había **2 221 géneros distintos** y una búsqueda de "comedy" no encontraba "comedy, drama".
- Los directores y actores vienen unidos con `,`, `&` o " and " ("Herbert Brenon and Carl Laemmle").
- "unknown" es un dato faltante, no un género: 6 083 películas (17 %) tenían género "unknown" y 1 124 director "Unknown".

**Solución:** `separarLista` unifica los separadores `–`, `—`, " and ", `;`, `/` y `&` a `,`, corta, recorta espacios y descarta los valores vacíos y "unknown". Se aplica a Director, Cast y Genre. El texto original de `genero` y `director` se conserva para mostrarlo.

**Decisión:** el guion `-` **no** separa, porque rompería "sci-fi", "neo-noir" y nombres como "Jean-Luc Godard".

**Resultado:** 2 221 → **1 014 géneros atómicos**; "comedy" pasó de 4 398 a 5 802 películas y "sci-fi" de 221 a 361.

## 5. Limpieza de marcas de cita

**Problema:** las sinopsis vienen de Wikipedia y traen marcas de referencia que se indexaban como números o palabras falsas: `[12]` (6 415 veces), `[citation needed]` (84), `[clarification needed]` (69), `[Note 1]`, `[N 2]`, `[a]`.

**Solución:** `quitarMarcasDeCita` elimina un corchete si su contenido (máximo 30 caracteres) es un número, una sola letra, una nota (`Note n`, `note n`, `N n`) o termina en `needed`.

**Decisión:** los corchetes con palabras reales se conservan, porque son parte del texto: `[her]`, `[his]`, `[1936 film]`. La limpieza se aplica al cargar, así que la sinopsis sale limpia tanto en el índice como en pantalla.

## 6–7. Minúsculas, plegado de tildes y puntuación Unicode

**Problema:** el CSV está en UTF-8, donde los caracteres fuera de ASCII ocupan de 2 a 4 bytes. La versión anterior trataba cada byte por separado y `isalnum` los reemplazaba por espacios: "Amélie" se indexaba como `am` + `lie`, y "Pokémon" como `pok` + `mon`.

**Solución:** `normalizarTexto` recorre el texto por **caracteres UTF-8 completos** (`largoUTF8` lee el primer byte para saber cuántos bytes tiene el carácter):

| Tipo de carácter | Tratamiento | Ejemplo |
|---|---|---|
| Letra o dígito ASCII | minúscula | `A` → `a` |
| Puntuación ASCII | espacio | `.` `,` `!` |
| Letra latina con tilde (U+00C0–U+017F) | letra ASCII, según la tabla `PLEGADO_LATINO` | `é`→`e`, `ō`→`o`, `ł`→`l`, `ß`→`ss` |
| Puntuación y símbolos Unicode (U+0080–U+00BF, U+2000–U+2FFF) | espacio | `’ — – “ ” … £ ₹` |
| Otros alfabetos (malayalam, japonés…) | se conservan intactos | `東京` |

La tabla `PLEGADO_LATINO` (192 entradas) se generó con la base de datos Unicode (descomposición NFKD sin diacríticos), para no depender de una transcripción manual. Se indexa con `(primerByte − 0xC3) · 64 + (segundoByte − 0x80)`, en O(1).

**Resultado:** "amelie" pasó de 0 a 2 resultados y "pokemon" de 0 a 20.

## 8. Tokenización

`tokenizar` separa el texto ya normalizado por espacios con `istringstream`. Como la normalización convirtió toda la puntuación en espacios, cada token es una palabra limpia.

**Memoria:** los tokens **no** se guardan en `Pelicula`. Se generan al vuelo durante la indexación y se descartan película por película. Guardarlos significaba 13,4 M de `std::string` (~430 MB).

## 9. Stopwords

**Problema:** por la ley de Zipf, las 50 palabras más frecuentes ("the", "to", "and", "a", "of"…) son el **41,5 %** de todo el texto. Aparecen en casi todas las películas: la consulta "the ghost" devolvía 34 511 de 34 886 películas.

**Solución:** un `std::set<std::string>` con 56 conectores y pronombres en inglés (el idioma del dataset), consultado en O(log n) (S5, igual que el ejemplo "contador de palabras sin conectores").

| | Sinopsis | Título | Árbol de sufijos |
|---|---|---|---|
| ¿Se indexan las stopwords? | No | **Sí** | No |

**Por qué los títulos sí:** 28 películas tienen un título formado solo por stopwords y serían imposibles de encontrar: *It* (1927, 1990, 2017), *Up* (2009), *Her* (2013), *She*, *Them!*, *To Be or Not to Be* (1942), *To Have and Have Not* (1944), *The One* (2001).

**Reglas de consulta:**

1. Si la consulta tiene al menos una palabra significativa ("the ghost"), las stopwords se ignoran: 880 resultados en vez de 34 511.
2. Si la consulta tiene **solo** stopwords ("it", "up", "to be or not to be"), se buscan como **palabra exacta** en el vocabulario. Como solo están indexadas en títulos, "it" encuentra *It* y no todo lo que contiene "city". "It" pasó de 0 a 188 resultados, "Up" de 0 a 144.

## 10. Normalización de tags

`normalizarTag` recorta espacios, pasa a minúsculas y pliega tildes, pero **conserva la puntuación ASCII**: la búsqueda por tag es por coincidencia exacta del nombre completo, y nombres como "S. Fleming" necesitan sus puntos. Así, "francois truffaut" encuentra a "François Truffaut".

## Decisiones descartadas

| Técnica | Decisión | Motivo |
|---|---|---|
| Quitar vocales | Descartada | "bar", "bear", "bore" y "boar" quedarían todas como "br": resultados incorrectos |
| Stemming (Porter) | Entrega 2, solo para el ranking | Choca con la búsqueda por sub-palabra: si se indexa "run" en vez de "running", la búsqueda "runni" falla. Además hay sobre-stemming (university = universe) |
| Eliminar duplicados título + año | Descartada | Los 285 casos son en su mayoría versiones de distintos países (el campo `Origin` es distinto), no errores |
| Separar géneros por `-` | Descartada | Rompe "sci-fi", "neo-noir" y "Jean-Luc" |

## Limitaciones conocidas

- Solo se pliegan las letras latinas (U+00C0–U+017F). Otros alfabetos se conservan, pero no se transliteran.
- La búsqueda por tag es por nombre completo: "spielberg" solo no encuentra a "Steven Spielberg".
- Algunos títulos del CSV empiezan con un espacio (" Babel"); no se recortan.
- En Windows, `main.cpp` configura la consola en UTF-8 (`SetConsoleOutputCP` / `SetConsoleCP`) para mostrar y leer tildes. Buscar sin tildes ("amelie") funciona siempre.

## Temas del curso utilizados

| Tema | Semana | Uso |
|---|---|---|
| Semántica de movimiento | S1 | `construirPeliculas` por valor + `std::move` |
| `string`, `vector`, iteradores | S5 | Parseo, tokenización, listas de tags |
| `map`, `set` | S5 | Índice invertido, stopwords, índices de tags |
| Big O | S6 | Plegado O(1) por carácter; normalización O(L) por texto de L bytes |
