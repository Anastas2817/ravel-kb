# ============================================================================
# ravel_storage — сериализация: IStorage, JsonStorage (nlohmann/json).
# ============================================================================

# 1) Сначала ищем установленную библиотеку (vcpkg, Conan, системная).
# 2) Если не нашли — подтягиваем через FetchContent (нужен интернет).
find_package(nlohmann_json 3.2.0 QUIET)

if(NOT nlohmann_json_FOUND)
    if(RAVEL_USE_FETCHCONTENT)
        message(STATUS "[ravel] nlohmann_json не найдена локально — FetchContent (нужен интернет)")
        include(FetchContent)
        FetchContent_Declare(nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG        v3.11.3
            GIT_SHALLOW    TRUE)
        FetchContent_MakeAvailable(nlohmann_json)
    else()
        message(FATAL_ERROR
            "nlohmann/json не найдена. Включите RAVEL_USE_FETCHCONTENT=ON "
            "или установите библиотеку (vcpkg install nlohmann-json).")
    endif()
endif()

file(GLOB RAVEL_STORAGE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp")

file(GLOB RAVEL_STORAGE_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

if(RAVEL_STORAGE_SOURCES)
    add_library(ravel_storage STATIC ${RAVEL_STORAGE_SOURCES} ${RAVEL_STORAGE_HEADERS})
    target_include_directories(ravel_storage PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
else()
    add_library(ravel_storage INTERFACE)
    target_include_directories(ravel_storage INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    message(STATUS "[ravel] storage: исходники не найдены — создана INTERFACE-заглушка")
endif()

add_library(ravel::storage ALIAS ravel_storage)

# nlohmann/json — header-only; ravel::core нужен самим исходникам storage
# (storage.hpp включает graph.hpp), поэтому связь PUBLIC, а не INTERFACE:
# INTERFACE-зависимости не попадают в include-пути собственной компиляции цели.
target_link_libraries(ravel_storage PUBLIC ravel::core nlohmann_json::nlohmann_json)

# Include-пути nlohmann/json помечаются SYSTEM (/external:I у MSVC, -isystem у
# gcc/clang): предупреждения анализатора из чужих заголовков (C26495, C26819
# и др.) не должны засорять нашу сборку. Работает и для find_package, и для
# FetchContent (у imported target INTERFACE_INCLUDE_DIRECTORIES уже задан).
if(TARGET nlohmann_json)
    if(nlohmann_json_SOURCE_DIR)
        set(_nlohmann_inc "${nlohmann_json_SOURCE_DIR}/include")
    else()
        get_target_property(_nlohmann_inc nlohmann_json INTERFACE_INCLUDE_DIRECTORIES)
    endif()
    set_target_properties(nlohmann_json PROPERTIES
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_nlohmann_inc}")
endif()
