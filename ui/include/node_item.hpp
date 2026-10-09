#ifndef RAVEL_UI_INCLUDE_NODE_ITEM_HPP_
#define RAVEL_UI_INCLUDE_NODE_ITEM_HPP_

#include <cstdint>

#include <QGraphicsObject>

namespace ravel::ui {

class NodeItem : public QGraphicsObject {
  Q_OBJECT

 public:
  NodeItem(std::uint64_t id, const QRectF& rect, QGraphicsItem* parent = nullptr);

  std::uint64_t Id() const {
    return id_;
  }
  QRectF boundingRect() const override;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
             QWidget* widget) override;

 signals:
  void Moved(std::uint64_t id, const QPointF& new_pos);

 protected:
  QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

 private:
  std::uint64_t id_;
  QRectF rect_;
};

}  // namespace ravel::ui

#endif  // RAVEL_UI_INCLUDE_NODE_ITEM_HPP_
