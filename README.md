# COMPREXXION
Un programa que copia una estructura de directorios y archivos definido en `comprexxion.txt` o cualquier otro archivo indicado con `-c <config.txt>` para luego comprimirlo en un archivo.

# Por qué?
No lo sé. Simplemente estoy aburrido y esta es una de las maneras que tengo para distraerme.

# Compilación

```sh
$ cmake -B build -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release
$ cmake --build build
```
o
```
chmod +x make.sh
./make.sh Release
```

# Ejecución

```sh
$ ./bin/comprexxion -c <config.txt>
```
