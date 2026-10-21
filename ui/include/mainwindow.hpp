#ifndef RAVEL_UI_INCLUDE_MAINWINDOW_HPP_
#define RAVEL_UI_INCLUDE_MAINWINDOW_HPP_

#include "controller.hpp"

#include <QAction>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMainWindow>

namespace ravel::ui {

class MainWindow : public QMainWindow {
  Q_OBJECT
 public:
  explicit MainWindow(QWidget* parent = nullptr);

 private slots:
  void OnOpen();

 private:
  static QString ErrorToString(ravel::app::ControllerError err);

  QGraphicsScene* scene_;
  QGraphicsView* view_;
  ravel::app::Controller controller_;
  QAction* open_action_;
};

}  // namespace ravel::ui

#endif  // RAVEL_UI_INCLUDE_MAINWINDOW_HPP_
