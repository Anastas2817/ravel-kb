#ifdef _MSC_VER
// Qt: шум code analysis (PREfast, C26xxx) из чужих заголовков — см. main.cpp.
#pragma warning(disable : 26495 26813)
#endif

#include "node_item.hpp"

#include <QPainter>

namespace ravel::ui {

NodeItem::NodeItem(std::uint64_t id, QString title, const QRectF& rect, QGraphicsItem* parent)
    : QGraphicsObject(parent), id_(id), title_(title), rect_(rect) {
  this->setFlag(QGraphicsItem::ItemIsMovable);
  this->setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  this->setAcceptHoverEvents(true);
}

QRectF NodeItem::boundingRect() const {
  return rect_;
}

void NodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/,
                     QWidget* /*widget*/) {
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setBrush(Qt::blue);
  painter->setPen(Qt::SolidLine);
  painter->drawRoundedRect(rect_, 10, 10);
  painter->drawText(rect_, Qt::AlignCenter, title_);
}

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant& value) {
  if (change == ItemPositionChange) {
    emit Moved(id_, value.toPointF());
  }
  return QGraphicsObject::itemChange(change, value);
}

}  // namespace ravel::ui
