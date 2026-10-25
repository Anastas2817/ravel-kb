#ifndef RAVEL_UI_INCLUDE_MAINWINDOW_HPP_
#define RAVEL_UI_INCLUDE_MAINWINDOW_HPP_

#include "canvas_view.hpp"
#include "controller.hpp"

#include <QAction>
#include <QGraphicsScene>
#include <QMainWindow>

namespace ravel::ui {

class NodeItem;

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
  void OnNodeDragged(std::uint64_t id, const QPointF& new_pos);
  void OnAddRelation(std::uint64_t from, std::uint64_t to, const QString& from_title,
                     const QString& to_title);
  void OnDeleteSelected();

 private:
  static QString ErrorToString(ravel::app::ControllerError err);
  NodeItem* FindNode(std::uint64_t id) const;

  QGraphicsScene* scene_;
  CanvasView* view_;
  ravel::app::Controller controller_;
  QAction* open_action_;
  QAction* save_action_;
  QAction* save_as_action_;
  QAction* undo_action_;
  QAction* redo_action_;
  QAction* delete_action_;
};

}  // namespace ravel::ui

#endif  // RAVEL_UI_INCLUDE_MAINWINDOW_HPP_
