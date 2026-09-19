# ============================================================================
# ravel_app — прикладной слой: команды (Command + undo/redo), контроллеры,
# история. Зависит от ядра и хранения; от Qt не зависит.
# ============================================================================

file(GLOB RAVEL_APP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp")

file(GLOB RAVEL_APP_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

if(RAVEL_APP_SOURCES)
    add_library(ravel_app STATIC ${RAVEL_APP_SOURCES} ${RAVEL_APP_HEADERS})
    target_include_directories(ravel_app PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
else()
    add_library(ravel_app INTERFACE)
    target_include_directories(ravel_app INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    message(STATUS "[ravel] app: исходники не найдены — создана INTERFACE-заглушка")
endif()

add_library(ravel::app ALIAS ravel_app)

target_link_libraries(ravel_app INTERFACE ravel::core ravel::storage)
