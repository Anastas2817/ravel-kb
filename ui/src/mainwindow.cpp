#include "mainwindow.hpp"

#include <QFileDialog>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>

namespace ravel::ui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      scene_(new QGraphicsScene(this)),
      view_(new QGraphicsView(this)),
      open_action_(new QAction(tr("Открыть…"), this)) {
  setCentralWidget(view_);
  view_->setScene(scene_);

  open_action_->setShortcut(QKeySequence::Open);  // Ctrl+O
  connect(open_action_, &QAction::triggered, this, &MainWindow::OnOpen);

  QMenu* file_menu = menuBar()->addMenu(tr("&Файл"));
  file_menu->addAction(open_action_);

  statusBar()->showMessage(tr("Готово"));
}

QString MainWindow::ErrorToString(ravel::app::ControllerError err) {
  using ravel::app::ControllerError;
  switch (err) {
    case ControllerError::NoPath:
      return tr("Файл не найден (или путь не задан).");
    case ControllerError::IoError:
      return tr("Ошибка чтения/записи файла (битый JSON или не та версия формата).");
    case ControllerError::NothingToUndo:
      return tr("Нечего отменять.");
    case ControllerError::NothingToRedo:
      return tr("Нечего возвращать.");
    default:
      return tr("Неизвестная ошибка.");
  }
}

void MainWindow::OnOpen() {
  const QString path =
      QFileDialog::getOpenFileName(this, tr("Открыть файл"), QString(), tr("Ravel (*.ravel.json)"));
  if (path.isEmpty()) {
    return;  // user close the dialog
  }
  const auto result = controller_.Open(std::filesystem::path(path.toStdWString()));
  if (!result.has_value()) {
    QMessageBox::warning(this, tr("Ошибка открытия"), ErrorToString(result.error()));
    return;
  }
  statusBar()->showMessage(tr("Открыто: %1").arg(path));
}

}  // namespace ravel::ui
