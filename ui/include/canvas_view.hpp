#ifndef RAVEL_UI_INCLUDE_CANVAS_VIEW_HPP_
#define RAVEL_UI_INCLUDE_CANVAS_VIEW_HPP_

#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>

namespace ravel::ui {
class CanvasView : public QGraphicsView {
  Q_OBJECT
 public:
  explicit CanvasView(QWidget* parent = nullptr);
  using QGraphicsView::QGraphicsView;
 signals:
  void DoubleClicked(QPointF scene_pos);

 protected:
  void wheelEvent(QWheelEvent* e) override;
  void mouseDoubleClickEvent(QMouseEvent* e) override {
    if (e->button() == Qt::LeftButton) {
      emit DoubleClicked(mapToScene(e->pos()));
    }
    QGraphicsView::mouseDoubleClickEvent(e);
  }
};

}  // namespace ravel::ui

#endif  //  RAVEL_UI_INCLUDE_CANVAS_VIEW_HPP_
