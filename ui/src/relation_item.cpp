#include <QObject>  // connect

#include "relation_item.hpp"
#include "node_item.hpp"

namespace ravel::ui {

RelationItem::RelationItem(NodeItem* from, NodeItem* to) : from_(from), to_(to) {
  QObject::connect(from, &NodeItem::Moved, [this]() { Update(); });
  QObject::connect(to, &NodeItem::Moved, [this]() { Update(); });
  Update();
}

void RelationItem::Update() {
  const QPointF src = from_->pos() + from_->boundingRect().center();
  const QPointF dest = to_->pos() + to_->boundingRect().center();
  setLine(QLineF(src, dest));
}

}  // namespace ravel::ui
