# Guardar listas del usuario en un archivo

La plataforma mantiene dos listas de IDs (`likes` y `verMasTarde`) en la clase `Plataforma`.
Ahora se persisten en `datos_usuario/listas.txt`, ubicado en la carpeta raiz del proyecto,
independientemente del directorio desde el que CLion ejecute el programa.

## Formato del archivo

Se crea automaticamente al guardar por primera vez:

```text
LISTAS_V1
LIKE 15
LIKE 42
VER_MAS_TARDE 42
```

La cabecera `LISTAS_V1` identifica el formato. Los enteros son IDs del catalogo cargado.
Los mismos IDs pueden aparecer en ambas listas; no se permite repetir el mismo ID dentro
de una sola lista.

## Flujo

1. `main()` carga el CSV con `cargarPeliculas()`.
2. `cargarListas(rutaListas)` lee y valida el archivo, si existe.
3. Al agregar un Like o guardar para Ver mas tarde, `mostrarDetalle()` llama a
   `guardarListas(rutaListas)` solo si se agrego un nuevo elemento.
4. El guardado escribe un archivo temporal y lo renombra al terminar.
5. Al reiniciar, `cargarListas()` recupera las listas anteriores.

## Consideraciones

- Primera ejecucion: si `listas.txt` no existe, ambas listas empiezan vacias.
- Archivo invalido o inaccesible: se detiene el arranque con un aviso, para evitar
  sustituir accidentalmente datos del usuario.
- Error al guardar: se muestra un aviso; el cambio permanece en memoria en la sesion actual,
  pero podria perderse al cerrar.
- `datos_usuario/` se excluye de Git mediante `.gitignore`.
- **Usa el mismo dataset y el mismo orden de filas** entre sesiones. Los IDs dependen de
  ese orden; si reemplazas/reordenas el CSV, una lista antigua podria apuntar a otra pelicula.
- El archivo es local a este proyecto/equipo; no se sincroniza entre usuarios.

## Prueba manual en CLion

1. Compila nuevamente con CMake.
2. Ejecuta, busca una pelicula y dale Like; agrega otra a Ver mas tarde.
3. Comprueba que se creo `datos_usuario/listas.txt` en la raiz del proyecto.
4. Cierra el programa completamente y vuelvelo a abrir.
5. Selecciona **5. Mis listas**; deben mostrarse los mismos titulos.

No es necesario editar los archivos manualmente.


## Quitar peliculas de las listas

1. En el menu principal entra a **5. Mis listas**.
2. Elige **1. Likes** o **2. Ver mas tarde**.
3. Se muestran de cinco en cinco. Usa **V** para avanzar y **A** para retroceder.
4. Escribe el numero mostrado al lado de la pelicula que quieres quitar.
5. Confirma con **S**. **N** cancela; **X** regresa a Mis listas.
6. Al quitarla, se actualiza `datos_usuario/listas.txt` inmediatamente.

La pelicula solo se elimina de la lista seleccionada. Si esta en las dos,
quitarla de Likes **no** la elimina de Ver mas tarde, ni viceversa.
La escritura en Windows reemplaza el archivo existente usando `MoveFileExW`.
