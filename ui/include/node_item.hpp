#ifndef RAVEL_UI_INCLUDE_NODE_ITEM_HPP_
#define RAVEL_UI_INCLUDE_NODE_ITEM_HPP_

#include <QGraphicsObject>
#include <QString>
#include <cstdint>

namespace ravel::ui {

class NodeItem : public QGraphicsObject {
  Q_OBJECT

 public:
  NodeItem(std::uint64_t id, QString title, const QRectF& rect, QGraphicsItem* parent = nullptr);

  std::uint64_t id() const {
    return id_;
  }
  QRectF boundingRect() const override;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

 signals:
  void Moved(std::uint64_t id, const QPointF& new_pos);

 protected:
  QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

 private:
  std::uint64_t id_;
  QString title_ = "wtf";
  QString name_ = "model";
  QString color_ = "#b3dcfd";
  QString shape_ = "rect";
  QString frame_color_ = "#000000";
  QRectF rect_;
};

}  // namespace ravel::ui

#endif  // RAVEL_UI_INCLUDE_NODE_ITEM_HPP_
