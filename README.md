# COMPREXXION
Un programa que copia una estructura de directorios y archivos definido en `comprexxion.txt` o cualquier otro archivo indicado con `-c <config.txt>` para luego comprimir todo en un archivo.

### IMPORTANT UPDATE
Cambie el estándar de c++23 a c++20 para que un amigo pueda ejecutarlo correctamente :)

### COMPILATION

```sh
$ cmake -B build -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release
$ cmake --build build
```

### EXECUTION

```sh
$ ./bin/comprexxion -c <config.txt>
```
