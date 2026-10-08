#ifndef RAVEL_UI_INCLUDE_NODE_ITEM_HPP_
#define RAVEL_UI_INCLUDE_NODE_ITEM_HPP_

#include <QGraphicsObject>

#include "graph.hpp"  // NodeId

namespace ravel::ui {

class NodeItem : public QGraphicsObject {
  Q_OBJECT

 public:
  NodeItem(ravel::NodeId id, const QRectF& rect, QGraphicsItem* parent = nullptr);

  ravel::NodeId Id() const { return id_; }
  QRectF boundingRect() const override;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
             QWidget* widget) override;

 signals:
  void Moved(ravel::NodeId id, const QPointF& new_pos);

 protected:
  QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

 private:
  ravel::NodeId id_;
  QRectF rect_;
};

}  // namespace ravel::ui

#endif  // RAVEL_UI_INCLUDE_NODE_ITEM_HPP_
