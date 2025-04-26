cmake_minimum_required ( VERSION 3.20 )

project ( COMPREXXION
    VERSION     0.21
    DESCRIPTION "Copia ciertos elementos de un proyecto y luego lo comprime."
    LANGUAGES   CXX
)

set(CMAKE_CXX_STANDARD_REQUIRED   ON)
set(CMAKE_CXX_EXTENSIONS         OFF)
set(CMAKE_CXX_STANDARD            20)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_NAME comprexxion)

# --- For Extra Resources
include(FetchContent)

# --- External Dependencie: fmt
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        11.1.4
)
FetchContent_MakeAvailable(fmt)


include_directories(
    ${CMAKE_SOURCE_DIR}/include/
)


file( GLOB_RECURSE EXTRA_RESOURCES
    ${CMAKE_SOURCE_DIR}/include/*.cpp
)


add_executable( ${EXECUTABLE_NAME}
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${EXTRA_RESOURCES}
)


target_compile_definitions( ${EXECUTABLE_NAME}
    PRIVATE
        $<$<CONFIG:Debug>:
            DEBUG
        >
)


target_compile_options( ${EXECUTABLE_NAME}
    PRIVATE
        $<$<CONFIG:Debug>:
            -g3
            -O0
            -fsanitize=address
            -fsanitize=leak
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wshadow
            -Wdouble-promotion
            -Wnull-dereference
            -Wformat=2
            -Wstrict-aliasing
            -Wstrict-overflow=5
            -Werror
            -fno-inline
            -fno-gnu-keywords
            -fno-optimize-sibling-calls
            -fdiagnostics-show-option
            -fno-asynchronous-unwind-tables
            -fdiagnostics-color=always
        >

        $<$<CONFIG:Release>:
            -Os
            -g0
            -Wl,--icf=sage,-strip-all,--gc-sections
            -fomit-frame-pointer
            -march=native
            -flto-compression-level=19
            -s
            -ffunction-sections
            -fvisibility=hidden
            -fvisibility-inlines-hidden
            # -fno-asynchronous-unwind-tables # <- not working in TDM
        >
)


target_link_options( ${EXECUTABLE_NAME}
    PRIVATE
        $<$<CONFIG:Debug>:
            -fsanitize=address
        >
)

target_link_libraries( ${EXECUTABLE_NAME}
    PRIVATE
        fmt::fmt
)
