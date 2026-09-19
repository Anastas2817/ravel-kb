# ============================================================================
# ravel — десктопное приложение (Qt 6 Widgets, QGraphicsView/QGraphicsScene).
#
# Если Qt 6 не установлена, цель пропускается с предупреждением:
# сборка ядра/тестов при этом остаётся рабочей (docs/architecture.md §1.1).
# ============================================================================

find_package(Qt6 COMPONENTS Widgets QUIET)

if(NOT Qt6_FOUND)
    message(WARNING
        "[ravel] Qt 6 Widgets не найдена — цель 'ravel' (UI) пропущена.\n"
        "Установите Qt через официальный установщик (кит MSVC 2022 64-bit) и укажите:\n"
        "  cmake -B build -DCMAKE_PREFIX_PATH=\"C:/Qt/6.x.y/msvc2022_64\"\n"
        "или соберите без UI: cmake -B build -DRAVEL_BUILD_UI=OFF")
    return()
endif()

message(STATUS "[ravel] Qt ${Qt6_VERSION} найдена — цель 'ravel' (UI) будет собрана")

set(CMAKE_AUTOMOC ON)  # moc для классов с Q_OBJECT
set(CMAKE_AUTOUIC ON)  # формы .ui, если появятся
set(CMAKE_AUTORCC ON)  # ресурсы .qrc, если появятся

file(GLOB RAVEL_UI_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.qrc")

if(NOT RAVEL_UI_SOURCES)
    message(WARNING "[ravel] ui: исходников пока нет — исполняемый файл 'ravel' не создан")
    return()
endif()

# WIN32 — GUI-подсистема Windows (без консольного окна).
add_executable(ravel WIN32 ${RAVEL_UI_SOURCES})

target_link_libraries(ravel PRIVATE
    Qt6::Widgets
    ravel::app
    ravel::core
    ravel::storage)

# ---------------------------------------------------------------------------
# windeployqt: после сборки подкладываем Qt-DLL рядом с ravel.exe,
# чтобы он запускался вне IDE.
# ---------------------------------------------------------------------------
find_program(RAVEL_WINDEPLOYQT
    NAMES windeployqt
    HINTS "${Qt6_DIR}/../../../bin" "$ENV{QTDIR}/bin")

if(RAVEL_WINDEPLOYQT)
    add_custom_command(TARGET ravel POST_BUILD
        COMMAND "${RAVEL_WINDEPLOYQT}"
                --no-translations
                --compiler-runtime
                "$<TARGET_FILE:ravel>"
        COMMENT "windeployqt: копирую Qt-DLL рядом с ravel.exe")
else()
    message(STATUS "[ravel] windeployqt не найден — запуск .exe вне IDE потребует ручного windeployqt")
endif()

# Установка: exe + рядом лежащие Qt-DLL (после windeployqt).
install(TARGETS ravel RUNTIME DESTINATION .)
if(RAVEL_WINDEPLOYQT)
    install(DIRECTORY "$<TARGET_FILE_DIR:ravel>/" DESTINATION .)
endif()
