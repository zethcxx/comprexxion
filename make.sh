#!/bin/bash
set -e

# Nombre del directorio de construcción
BUILD_DIR="build"

# Tipo de compilación: Debug o Release (por defecto: Release)
BUILD_TYPE=${1:-Release}

# Eliminar el directorio de construcción si existe para una compilación limpia
if [ -d "$BUILD_DIR" ]; then
    echo "Eliminando el directorio de construcción existente..."
    rm -rf "$BUILD_DIR"
fi

# Crear el directorio de construcción
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configurar el proyecto con CMake
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" ..

# Compilar el proyecto
cmake --build .
