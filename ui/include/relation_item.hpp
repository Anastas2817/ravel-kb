#ifndef RAVEL_UI_INCLUDE_RELATION_ITEM_HPP_
#define RAVEL_UI_INCLUDE_RELATION_ITEM_HPP_

#include <QGraphicsItem>
#include <QLineF>
#include <QPainter>
#include <QString>
#include <cstdint>
#include <string>

namespace ravel::ui {

class RelationItem : public QGraphicsItem {
 public:
  RelationItem(std::uint64_t id, std::uint64_t from, std::uint64_t to, QString color,
               std::string arrow, bool symmetric);

  std::uint64_t id() const {
    return id_;
  }
  std::uint64_t from() const {
    return from_;
  }
  std::uint64_t to() const {
    return to_;
  }

  [[nodiscard]] QRectF boundingRect() const override;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

  void setLine(const QLineF& line) {
    if (line_ == line) {
      return;
    }
    // boundingRect() depends on line_: the scene must repaint the old region
    // and reindex the item. Without this the old pixels stay (trail) and the
    // new position is not repainted (line lags behind the dragged node).
    prepareGeometryChange();
    line_ = line;
    update();
  }

 private:
  QLineF line_;

  std::uint64_t id_;
  std::uint64_t from_;
  std::uint64_t to_;
  QString description_;

  QString type_;
  QString color_;
  std::string arrow_;
  bool symmetric_;
};

}  // namespace ravel::ui

#endif  // RAVEL_UI_INCLUDE_RELATION_ITEM_HPP_
