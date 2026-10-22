# ============================================================================
# Fuzz target: libFuzzer-обёртка над JsonStorage::Load + Graph::FromSnapshot.
#
# Требует Clang: флаг -fsanitize=fuzzer отсутствует в MSVC. Санитайзеры
# address/undefined задаются сверху (как в job sanitizers: через
# -DCMAKE_CXX_FLAGS) — здесь добавляется только движок фуззинга.
# ============================================================================

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR
        "[ravel] RAVEL_BUILD_FUZZ требует Clang (clang++) — "
        "-fsanitize=fuzzer в MSVC отсутствует")
endif()

add_executable(ravel_storage_fuzz storage_fuzz.cpp)
target_link_libraries(ravel_storage_fuzz PRIVATE
    ravel::storage
    ravel::core)
target_compile_options(ravel_storage_fuzz PRIVATE -fsanitize=fuzzer)
target_link_options(ravel_storage_fuzz PRIVATE -fsanitize=fuzzer)
