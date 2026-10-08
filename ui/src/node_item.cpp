#include <QPainter>

#include "node_item.hpp"

namespace ravel::ui {

NodeItem::NodeItem(ravel::NodeId id, const QRectF& rect, QGraphicsItem* parent)
    : QGraphicsObject(parent), id_(id), rect_(rect) {
  this->setFlag(QGraphicsItem::ItemIsMovable);
  this->setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  this->setAcceptHoverEvents(true);
}

QRectF NodeItem::boundingRect() const {
  return rect_;
}

void NodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/,
                     QWidget* /*widget*/) {
  painter->setBrush(Qt::blue);
  painter->setPen(Qt::SolidLine);
  painter->drawRect(rect_);
}

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant& value) {
  if (change == ItemPositionChange) {
    emit Moved(id_, value.toPointF());
  }
  return QGraphicsObject::itemChange(change, value);
}

}  // namespace ravel::ui
