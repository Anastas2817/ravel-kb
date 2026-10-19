#!/usr/bin/env bash
# Архитектурный тест: ui/ не включает graph.hpp напрямую.
#
# Инвариант 7 (implementation.md): все мутации — через команды, ui зависит
# только от фасада app (Controller). Прямое включение graph.hpp из ui —
# первый шаг к обходу фасада; этот скрипт — страховочная сетка, которая
# падает раньше, чем нарушение попадёт в main.
#
# Транзитивное включение (ui -> controller.hpp -> graph.hpp) разрешено:
# проверяется только ПРЯМОЕ включение — ровно то, что означает инвариант.
# Когда тест сработает на этапе 3 (UI захочет типы ядра для отрисовки),
# правильный ответ — read-API фасада, а не ослабление проверки.

set -euo pipefail

violations=$(grep -rn --include='*.hpp' --include='*.cpp' \
  '#include[[:space:]]*"graph\.hpp"' ui/ || true)

if [[ -n "$violations" ]]; then
  echo "АРХИТЕКТУРНЫЙ ТЕСТ ПРОВАЛЕН: ui/ включает graph.hpp напрямую (обход фасада app):"
  echo "$violations"
  echo "Правильный путь: расширить read-API Controller, не включать ядро из ui."
  exit 1
fi

echo "OK: ui/ не включает graph.hpp напрямую."
