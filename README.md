# COMPREXXION
Un programa que copia una estructura de directorios y archivos definido en `comprexxion.txt` o cualquier otro archivo indicado con `-c <config.txt>` para luego comprimir todo en un archivo.

---

> [!IMPORTANT]
> No terminado

---

## IMPORTANT UPDATE
Cambie el estándar de c++23 a c++20 para que un amigo pueda ejecutarlo correctamente &nbsp; : )

> [!NOTE]
> Los comentarios en los .hpp son para un mejor reconocimiento de mi parte, asi que ignóralos &nbsp;:b.

## CONFIGURATION FILE AND RULES

Estructura de la configuración:

```yaml
project_name  : <string>
project_root  : <string>
compress_type : <string>
compress_level: <int32>

structure:
<indent><+|-><d|f><string>
```

`+` = include<br>
`-` = exclude<br>
`d` = directory<br>
`f` = file

Las cadenas no pueden estar vacías.
Puedes anidar directorio y archivos ejemplo:

```yaml
structure:
  +d "src/"
      +f "main.cpp"
```

Pero en el siguiente caso se ignora `"src/"` ya que la inclusión tiene prioridad sobre la exclusión:

```yaml
structure:
  +d "src/"
      +f "main.cpp"

  -d "src/"
```

Por defecto los directorios en vez de copiarse se crean, asi que para añadir todo el contenido explícitamente se utiliza el asterisco:<br>
- `+d "<path>" *`.

Pero lo siguiente genera redundancia:

```yaml
structure:
  +d "src/" *
      +f "main.cpp"
```

Si se usa `-d "<path>"` no hace falta añadir el asterisco, ya que por defecto se excluirá la carpeta y todo su contenido.

Por ende el siguiente ejemplo genera un error:

```yaml
structure:
  -d "build/"
      -f "file1" # ERROR
      +f "file2" # ERROR
```


El primer bloque de identación ya sea con espacios o tabulaciones, se utilizara para determinar cada nivel de indentación.

Hay un caso especial en lo siguiente:

```yaml
structure:
 # este es un comentario
  +d "src/"
      +f "main.cpp"

  -d "src/"
```

El primer bloque de indentación (sea espacios o tabulaciones) dentro de `structure` se usará como unidad base de indentación. Incluso si la primera línea es un comentario no se ignora su indentación y se toma como referencia para todo el bloque.

Todas las líneas siguientes deben estar alineadas con múltiplos exactos de esa indentación base. Si una línea rompe este patrón, se considera error de indentación.

## COMPILATION

```sh
$ cmake -B build -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release
$ cmake --build build
```

## EXECUTION

```sh
$ ./bin/comprexxion -c <config.txt>
```
