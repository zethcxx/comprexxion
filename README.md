# COMPREXXION
Un programa que copia una estructura de directorios y archivos definido en `comprexxion.txt` o cualquier otro archivo indicado con `-c <config.txt>` para luego comprimir todo en un archivo.


# Compilación

```sh
$ cmake -B build -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release
$ cmake --build build
```

# Ejecución

```sh
$ ./bin/comprexxion
```
