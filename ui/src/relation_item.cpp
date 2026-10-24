#ifdef _MSC_VER
// Qt: шум code analysis (PREfast, C26xxx) из чужих заголовков — см. main.cpp.
#pragma warning(disable : 26495 26813)
#endif

#include "relation_item.hpp"

#include "node_item.hpp"

#include <QObject>  // connect

namespace ravel::ui {

RelationItem::RelationItem(std::uint64_t id, std::uint64_t from, std::uint64_t to)
    : id_(id), from_(from), to_(to) {
}

}  // namespace ravel::ui
