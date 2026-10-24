#ifdef _MSC_VER
// Qt: шум code analysis (PREfast, C26xxx) из чужих заголовков — см. main.cpp.
#pragma warning(disable : 26495 26813)
#endif

#include "relation_item.hpp"

#include <QColor>
#include <QPen>
#include <QPolygonF>

namespace ravel::ui {

namespace {
// Arrowhead geometry: length along the line and half-width at the base. Must
// fit into kBoundingMargin, or the head gets clipped by boundingRect().
constexpr qreal kHeadLength = 12.0;
constexpr qreal kHeadHalfWidth = 6.0;
constexpr qreal kBoundingMargin = 16.0;
}  // namespace

RelationItem::RelationItem(std::uint64_t id, std::uint64_t from, std::uint64_t to, QString color,
                           std::string arrow, bool symmetric)
    : id_(id), from_(from), to_(to), color_(color), arrow_(arrow), symmetric_(symmetric) {
  setZValue(-1);
}

QRectF RelationItem::boundingRect() const {
  // The margin must cover the arrowhead, which sticks out beyond the line end.
  // QLineF has no boundingRect(); QRectF from endpoints is normalized.
  return QRectF(line_.p1(), line_.p2())
      .adjusted(-kBoundingMargin, -kBoundingMargin, kBoundingMargin, kBoundingMargin);
}

void RelationItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/,
                         QWidget* /*widget*/) {
  painter->setRenderHint(QPainter::Antialiasing);
  QPen pen{QColor(color_)};  // braces: parentheses would parse as a function declaration
  if (arrow_ == "solid") {
    pen.setStyle(Qt::SolidLine);
  } else if (arrow_ == "dashed") {
    pen.setStyle(Qt::DashLine);
  } else if (arrow_ == "dotted") {
    // Qt::DotLine draws dots nearly touching; a manual pattern with a round
    // cap gives readable separated dots.
    pen.setCapStyle(Qt::RoundCap);
    pen.setDashPattern({1.0, 4.0});
  }
  painter->setPen(pen);
  painter->drawLine(line_);

  // Arrowhead only for directed relations: a symmetric one carries no direction.
  if (!symmetric_ && line_.length() > 0) {
    const QPointF dir = line_.unitVector().p2() - line_.unitVector().p1();
    const QPointF normal(-dir.y(), dir.x());
    const QPointF base = line_.p2() - dir * kHeadLength;
    const QPolygonF head{line_.p2(), base + normal * kHeadHalfWidth,
                         base - normal * kHeadHalfWidth};
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(color_));
    painter->drawPolygon(head);
  }
}

}  // namespace ravel::ui
