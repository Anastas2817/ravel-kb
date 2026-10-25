# ============================================================================
# Ravel — корневой CMakeLists
#
# Сборка (Windows 11 + Visual Studio 2022):
#   cmake -B build -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH="C:/Qt/6.x/msvc2022_64"
#   cmake --build build --config Release
#   ctest --test-dir build -C Release --output-on-failure
#
# Подробности и требования — README.md, «Быстрый старт».
# ============================================================================

cmake_minimum_required(VERSION 3.20)

project(ravel
    VERSION 0.1.0
    DESCRIPTION "Инструмент для ручного проектирования персональных баз знаний"
    LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Глобальные настройки C++
# ---------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# std::expected (<expected>) — потребитель: слой команд (app/command.hpp).
# Поддержка тулчейнов матрицы: GCC libstdc++ 12+, MSVC STL 19.33+ (VS 2022
# 17.3+), libc++ 16+. Перед downgrade проверить эти версии.

# Qt собирается с динамическим рантаймом (/MD) — подгоняем под него,
# иначе будут конфликты линковки (LNK2038: mismatch RuntimeLibrary).
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Тип сборки" FORCE)
endif()

# Единые каталоги вывода (для VS-генератора конфигурация подставляется сама).
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# ---------------------------------------------------------------------------
# Опции
# ---------------------------------------------------------------------------

option(RAVEL_BUILD_UI "Собирать десктопное приложение (требуется Qt 6 Widgets)" ON)
option(RAVEL_USE_FETCHCONTENT "Подтягивать недостающие зависимости через FetchContent (нужен интернет)" ON)
option(RAVEL_BUILD_FUZZ "Собирать libFuzzer-таргет (требуется Clang, см. fuzz/CMakeLists.txt)" OFF)

# ---------------------------------------------------------------------------
# Флаги MSVC (Microsoft Visual C++, Visual Studio 2022)
# ---------------------------------------------------------------------------

if(MSVC)
    # /W4            — высокий уровень предупреждений
    # /permissive-   — строгое соответствие стандарту C++23
    # /utf-8         — исходники в UTF-8 (кириллица в строках/комментариях)
    # /Zc:__cplusplus — корректное значение __cplusplus (иначе 199711L)
    add_compile_options(/W4 /permissive- /utf-8 /Zc:__cplusplus)

    # Дебаг-информация в отдельном .pdb и для релиза (удобно анализировать краши)
    add_compile_options("$<$<CONFIG:Release>:/Zi>")
    add_link_options("$<$<CONFIG:Release>:/DEBUG>" "$<$<CONFIG:Release>:/OPT:REF>" "$<$<CONFIG:Release>:/OPT:ICF>")

    # Воспроизводимая сборка: __DATE__/__TIME__ не меняются между пересборками.
    # (/Zc:__DATE__/% /Zc:__TIME__/% — флаги clang-cl, MSVC их не знает и ругается D9002)
    add_compile_options(/Brepro)
endif()

# ---------------------------------------------------------------------------
# Цели
# ---------------------------------------------------------------------------

add_subdirectory(core)    # ядро: чистый C++23, без Qt (docs/architecture.md §1)
add_subdirectory(storage) # IStorage / JsonStorage (nlohmann/json)
add_subdirectory(app)     # команды (Command + undo/redo), контроллеры

if(RAVEL_BUILD_UI)
    add_subdirectory(ui)  # Qt 6 Widgets; если Qt не найден — цель пропускается с предупреждением
endif()

# Fuzz target (libFuzzer + ASan/UBSan): только Clang, опция OFF по умолчанию.
# tests/ не подходит — глоб собирает все *.cpp в ravel_tests, а фуззер имеет
# свой main (LLVMFuzzerTestOneInput). Отдельный каталог — чтобы жил отдельно.
if(RAVEL_BUILD_FUZZ)
    add_subdirectory(fuzz)
endif()

# Тесты: стандартный переключатель BUILD_TESTING (из CTest).
# add_subdirectory без guard EXISTS намеренно: пропажа каталога тестов должна
# ронять configure, а не молча отключать тесты (2026-10-16: перенос tests/ в
# docs/tests/ выпал из-за guard'а молча — CI стал бы зелёным без единого теста).
include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests) # GoogleTest; FetchContent — только если есть тесты
endif()

# Приложение — стартовый проект при открытии *.sln
if(TARGET ravel)
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT ravel)
endif()
