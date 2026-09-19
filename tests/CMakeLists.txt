# ============================================================================
# Тесты ядра (GoogleTest). Тестируем только ядро и слой приложения —
# они не требуют Qt и запускаются headless (docs/architecture.md §1.1).
# ============================================================================

file(GLOB RAVEL_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp")

if(NOT RAVEL_TEST_SOURCES)
    message(STATUS "[ravel] tests: тестов пока нет — GoogleTest не подтягиваю")
    return()
endif()

# GoogleTest: ищем установленный, иначе — FetchContent (нужен интернет).
find_package(GTest 1.12 QUIET)

if(NOT GTest_FOUND)
    if(RAVEL_USE_FETCHCONTENT)
        message(STATUS "[ravel] GTest не найден локально — FetchContent (нужен интернет)")
        include(FetchContent)
        # /MD-рантайм для MSVC, иначе конфликт с Qt и нашими целями
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        v1.14.0
            GIT_SHALLOW    TRUE)
        FetchContent_MakeAvailable(googletest)
    else()
        message(FATAL_ERROR
            "GoogleTest не найден. Включите RAVEL_USE_FETCHCONTENT=ON "
            "или установите GTest (vcpkg install gtest).")
    endif()
endif()

add_executable(ravel_tests ${RAVEL_TEST_SOURCES})

target_link_libraries(ravel_tests PRIVATE
    ravel::app
    ravel::core
    ravel::storage
    GTest::gtest_main)

# Регистрация тестов в CTest: каждый TEST() — отдельный кейс.
include(GoogleTest)
gtest_discover_tests(ravel_tests
    DISCOVERY_TIMEOUT 60
    PROPERTIES LABELS "core")
