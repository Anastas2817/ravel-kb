#ifdef _MSC_VER
// Qt: шум code analysis (PREfast, C26xxx) из чужих заголовков — см. main.cpp.
#pragma warning(disable : 26495 26813)
#endif

#include "node_item.hpp"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

namespace ravel::ui {

NodeItem::NodeItem(std::uint64_t id, QString title, QString type, const QRectF& rect,
                   QGraphicsItem* parent)
    : QGraphicsObject(parent), id_(id), title_(title), type_(type), rect_(rect) {
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

void NodeItem::mousePressEvent(QGraphicsSceneMouseEvent* e) {
  if (e->button() == Qt::LeftButton) {
    drag_start_ = this->pos();
  }
  QGraphicsObject::mousePressEvent(e);
}

void NodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e) {
  if (e->button() == Qt::LeftButton) {
    QPointF cur = this->pos();
    if (cur != drag_start_) {
      emit MoveFinished(id_, cur);
    }
  }
  QGraphicsObject::mouseReleaseEvent(e);
}

}  // namespace ravel::ui
