# Plataforma de Streaming

## Descripción

Plataforma de streaming en C++ que busca películas por **palabra, frase, sub-palabra o tag** (director, actor, género) sobre un dataset de 34 886 películas. Los resultados se ordenan por importancia y se muestran de 5 en 5. Al seleccionar una película se ve su sinopsis, con las opciones **Like** y **Ver más tarde**.

La búsqueda de texto usa un **Suffix Trie por palabra** construido sobre el vocabulario, junto con un índice invertido: encuentra sub-palabras ("bar" en "embarcar") con un recorrido O(k), usando unos 340 MB de memoria.

## Cómo ejecutarlo

1. Descargar el dataset y colocarlo en `dataset/` (no está en el repositorio).
2. Compilar con CMake en modo **Release**.
3. Ejecutar `ProyectoPrograIII`.

Detalle paso a paso en [instalacion.md](instalacion.md).

## Documentación

| Documento | Contenido |
|---|---|
| [Enunciado](#enunciado) | Requerimientos del proyecto (al final de este documento) |
| [Instalación](instalacion.md) | Dataset, compilación y ejecución |
| [Pre-procesamiento](preprocesamiento.md) | Limpieza y normalización de los datos, con mediciones |
| [Árbol](arbol.md) | Justificación del Suffix Trie, complejidad y memoria |
| [Ranking](ranking.md) | Algoritmo de importancia |
| [Pruebas](pruebas.md) | 29 casos de prueba con su salida esperada |

## Estructura

El código está organizado por capas; cada capa solo depende de las de abajo.

```
src/
├── main.cpp                      Presentación: menú, paginación y detalle (consola)
├── app/
│   └── Plataforma.h/.cpp         Coordinación: catálogo, índices y listas del usuario
├── busqueda/
│   ├── IndiceBusqueda.h/.cpp     Búsqueda por texto (árbol + índice invertido)
│   ├── IndiceTags.h/.cpp         Búsqueda por director, actor y género
│   └── Ranking.h/.cpp            Algoritmo de importancia
├── estructuras/
│   └── SuffixTrie.h              Árbol de sufijos
├── datos/
│   ├── LectorCSV.h/.cpp          Lectura del CSV
│   └── Texto.h/.cpp              Normalización, tokenización y stopwords
└── modelo/
    └── Pelicula.h                Modelo de datos
```

```
main → Plataforma → LectorCSV ─────────→ Pelicula
                  → IndiceBusqueda → SuffixTrie
                  │                → Texto
                  → IndiceTags ─────→ Texto
                  → Ranking ────────→ IndiceBusqueda, Texto
```

## Estado — Entrega 1 (semana 8)

| Funcionalidad | Estado |
|---|---|
| Lectura y pre-procesamiento del CSV en C++ | Hecho |
| Árbol de búsqueda (Suffix Trie por palabra) | Hecho |
| Búsqueda por palabra, frase y sub-palabra | Hecho |
| Búsqueda por tag (director, actor, género) | Hecho |
| Top 5 por importancia y "ver siguientes 5" | Hecho |
| Sinopsis, Like y Ver más tarde | Hecho (en memoria) |
| Mostrar "Ver más tarde" al iniciar (guardado en archivo) | Entrega 2 |
| Recomendaciones según los Likes | Entrega 2 |

## Enunciado

El objetivo del proyecto final es implementar una plataforma de **streaming**. Un programa que administre la **búsqueda y visualización** de la sinopsis de películas. Para ello se debe implementar las siguientes operaciones:

* El programa debe leer la base de datos en forma **.csv**. La base de datos puede ser descargada desde el siguiente [link] (https://drive.google.com/file/d/1UJkRuCF8UD92W_DT7S8dXCYzaR_9wqB_/view?usp=sharing). El grupo es responsable del **pre-procesamiento de los datos**.
* El programa debe cargar el contenido corregido del archivo en un **Árbol** que permita la búsqueda rápida de una película. Los caracteres (letras y números) deben ser los valores que se almacenen en los nodos del Árbol. Puede utilizar como referencia estructuras como los **Tries**, **Suffix Trees**, etc." La elección del tipo de Árbol queda a criterio del grupo y debe ser justificada y documentada en el repositorio.
* Para buscar una película se debe utilizar una **palabra, frase o sub-palabra**. Ejemplo:
    - Si se busca la palabra "barco", el programa debería encontrar todas las películas en las cuales la palabra "barco" este en el título o sinópsis.
    - Si se busca la frase "barco fantasma", el programa debería encontrar todas las películas en las cuales las palabras "barco" y/o "fantasma" este en el título o sinópsis.
    - Si se busca el string "bar", el programa debería encontrar todas las películas en las cuales el string "bar" este en el título o sinópsis (El string "bar" podría ser parte de una palabra).
* También se debe poder buscar películas por un **Tag**: director, casting, generero, etc.
* Al buscar películas deben de aparecer la cinco más **importantes** y una opción para visualizar las siguientes cinco coincidencias. El grupo **debe implementar un algoritmo** para determinar que pélicula tiene más importancia en una búsqueda.
* Al seleccionar una película, se debe visualizar la sinopsis y las opciones **Like** y **Ver más tarde** .
* Al iniciar el programa la plataforma debería mostrar las películas que fueron añadidas en **Ver más tarde**. Además, se debe visualizar las películas similares a las que el usuario les dio **Like** (implemente su propio algoritmo).

### Requisitos
* Grupos de cuatro personas como máximo y de tres como mínimo. No se aceptarán grupos de dos o una persona.
* Subir el programa a un repositorio en Github. **En el repositorio debe de estar toda la documentación sobre el proyecto**.
* La exposición del proyecto es **Presencial**. En la presentación de la semana 7/8, las exposiciones son con respecto a los avances que hayan conseguido.
* Todo el programa, desde la lectura hasta la búsqueda de palabras, debe estar en C++.
* Cumplir con la rúbrica del proyecto.
* Fecha de presentación: La semana 8 (avance) y la semana 16 (final).
