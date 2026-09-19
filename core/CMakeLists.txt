# ============================================================================
# ravel_core — ядро: модель данных, алгоритмы раскладки, собственные сигналы.
# Чистый C++17, БЕЗ Qt и сторонних зависимостей (docs/architecture.md §1.1).
# ============================================================================

file(GLOB RAVEL_CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp")

file(GLOB RAVEL_CORE_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.hpp")

if(RAVEL_CORE_SOURCES)
    add_library(ravel_core STATIC ${RAVEL_CORE_SOURCES} ${RAVEL_CORE_HEADERS})
    target_include_directories(ravel_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
else()
    # Исходников пока нет — интерфейсная цель, чтобы сборка и зависимости
    # настраивались уже сейчас. Появление .cpp-файлов в src/ автоматически
    # превратит её в статическую библиотеку (CONFIGURE_DEPENDS).
    add_library(ravel_core INTERFACE)
    target_include_directories(ravel_core INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    message(STATUS "[ravel] core: исходники не найдены — создана INTERFACE-заглушка")
endif()

add_library(ravel::core ALIAS ravel_core)

target_compile_features(ravel_core INTERFACE cxx_std_17)
