cmake_minimum_required(VERSION 3.20)
project(ProyectoPrograIII)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ProyectoPrograIII
        src/main.cpp
        src/app/Plataforma.h
        src/app/Plataforma.cpp
        src/app/PersistenciaListas.cpp
        src/busqueda/IndiceBusqueda.h
        src/busqueda/IndiceBusqueda.cpp
        src/busqueda/IndiceTags.h
        src/busqueda/IndiceTags.cpp
        src/busqueda/Ranking.h
        src/busqueda/Ranking.cpp
        src/estructuras/SuffixTrie.h
        src/datos/LectorCSV.h
        src/datos/LectorCSV.cpp
        src/datos/Texto.h
        src/datos/Texto.cpp
        src/modelo/Pelicula.h)

# Los #include se escriben relativos a src/ (p. ej. "modelo/Pelicula.h").
target_include_directories(ProyectoPrograIII PRIVATE src)
target_compile_definitions(ProyectoPrograIII PRIVATE RUTA_PROYECTO="${CMAKE_SOURCE_DIR}")
