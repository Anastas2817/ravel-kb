#ifndef RAVEL_UI_INCLUDE_RELATION_ITEM_HPP_
#define RAVEL_UI_INCLUDE_RELATION_ITEM_HPP_

#include <QGraphicsLineItem>

namespace ravel::ui {

class NodeItem;

class RelationItem : public QGraphicsLineItem {
 public:
  RelationItem(NodeItem* from, NodeItem* to);

  void Update();

 private:
  NodeItem* from_;
  NodeItem* to_;
};

}  // namespace ravel::ui

#endif  // RAVEL_UI_INCLUDE_EDGE_ITEM_HPP_
