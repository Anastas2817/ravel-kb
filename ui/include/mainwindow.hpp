#ifndef RAVEL_UI_INCLUDE_MAINWINDOW_HPP_
#define RAVEL_UI_INCLUDE_MAINWINDOW_HPP_

#include "controller.hpp"

#include <QAction>
#include <QComboBox>
#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLineEdit>
#include <QMainWindow>
#include <QMouseEvent>

namespace ravel::ui {

class CanvasView : public QGraphicsView {
  Q_OBJECT
 public:
  using QGraphicsView::QGraphicsView;
 signals:
  void DoubleClicked(QPointF scene_pos);

 protected:
  void mouseDoubleClickEvent(QMouseEvent* e) override {
    if (e->button() == Qt::LeftButton) {
      emit DoubleClicked(mapToScene(e->pos()));
    }
    QGraphicsView::mouseDoubleClickEvent(e);
  }
};

class AddNodeDialog : public QDialog {
  Q_OBJECT
 public:
  AddNodeDialog(const QStringList& types, QWidget* parent = nullptr);
  [[nodiscard]] QString SelectedType() const;
  [[nodiscard]] QString NodeTitle() const;

 private:
  QComboBox* type_;
  QLineEdit* title_;
};

class MainWindow : public QMainWindow {
  Q_OBJECT
 public:
  explicit MainWindow(QWidget* parent = nullptr);

 private slots:
  void OnOpen();
  void OnSave();
  void OnSaveAs();
  void OnSceneDoubleClick(QPointF pos);
  void ResyncScene();
  void OnUndo();
  void OnRedo();
  void OnNodeMoved(std::uint64_t id, QPointF to);

 private:
  static QString ErrorToString(ravel::app::ControllerError err);

  QGraphicsScene* scene_;
  CanvasView* view_;
  ravel::app::Controller controller_;
  QAction* open_action_;
  QAction* save_action_;
  QAction* save_as_action_;
  QAction* undo_action_;
  QAction* redo_action_;
};

}  // namespace ravel::ui

#endif  // RAVEL_UI_INCLUDE_MAINWINDOW_HPP_
