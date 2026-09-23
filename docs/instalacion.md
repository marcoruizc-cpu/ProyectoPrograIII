# Instalación y ejecución

## 1. Descargar el dataset

El dataset (81 MB) **no está en el repositorio**.

1. Descargar `wiki_movie_plots_deduped.csv` desde el [enlace del curso](https://drive.google.com/file/d/1UJkRuCF8UD92W_DT7S8dXCYzaR_9wqB_/view?usp=sharing).
2. Colocarlo en la carpeta `dataset/` del proyecto, sin cambiarle el nombre:

```
ProyectoPrograIII/
└── dataset/
    └── wiki_movie_plots_deduped.csv
```

Si el archivo no está ahí, el programa lo indica y termina.

## 2. Compilar

Requisitos: CMake 3.20 o superior y un compilador con C++17 (MinGW / MSVC en Windows, g++ o clang en Linux/macOS).

**CLion:** abrir la carpeta del proyecto, crear el perfil **Release** en *Settings → Build, Execution, Deployment → CMake* y compilar. En Debug la indexación es varias veces más lenta.

**Terminal:**

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## 3. Ejecutar

```
./build/ProyectoPrograIII            # usa dataset/wiki_movie_plots_deduped.csv
./build/ProyectoPrograIII otra/ruta.csv
```

La carga e indexación tarda entre 10 y 30 s según el equipo; luego aparece el menú. En Windows, el programa configura la consola en UTF-8 para mostrar y aceptar tildes.

## 4. Verificar

Ejecutar los casos de [pruebas.md](pruebas.md).
