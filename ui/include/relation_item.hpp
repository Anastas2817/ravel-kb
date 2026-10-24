#ifndef RAVEL_UI_INCLUDE_RELATION_ITEM_HPP_
#define RAVEL_UI_INCLUDE_RELATION_ITEM_HPP_

#include <QGraphicsLineItem>
#include <QString>
#include <cstdint>

namespace ravel::ui {

class NodeItem;

class RelationItem : public QGraphicsLineItem {
 public:
  RelationItem(std::uint64_t id, std::uint64_t from, std::uint64_t to);

  std::uint64_t id() const {
    return id_;
  }
  std::uint64_t from() const {
    return from_;
  }
  std::uint64_t to() const {
    return to_;
  }

 private:
  std::uint64_t id_;
  std::uint64_t from_;
  std::uint64_t to_;
  QString description_;

  QString type_;
  QString color_;
  QString arrow_;
  bool symmetric_;
};

}  // namespace ravel::ui

#endif  // RAVEL_UI_INCLUDE_RELATION_ITEM_HPP_
