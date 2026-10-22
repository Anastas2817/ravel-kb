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
      open_action_(new QAction(tr("Открыть…"), this)),
      save_action_(new QAction(tr("Сохранить"), this)),
      save_as_action_(new QAction(tr("Сохранить как…"), this)) {
  setCentralWidget(view_);
  view_->setScene(scene_);

  open_action_->setShortcut(QKeySequence::Open);  // Ctrl+O
  connect(open_action_, &QAction::triggered, this, &MainWindow::OnOpen);

  save_action_->setShortcut(QKeySequence::Save);  // Ctrl+S
  connect(save_action_, &QAction::triggered, this, &MainWindow::OnSave);

  save_as_action_->setShortcut(QKeySequence::SaveAs);  // Ctrl+Shift+S
  connect(save_as_action_, &QAction::triggered, this, &MainWindow::OnSaveAs);

  QMenu* file_menu = menuBar()->addMenu(tr("&Файл"));
  file_menu->addAction(open_action_);
  file_menu->addAction(save_action_);
  file_menu->addAction(save_as_action_);

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
  const ravel::app::ControllerResult result =
      controller_.Open(std::filesystem::path(path.toStdWString()));
  if (!result.has_value()) {
    QMessageBox::warning(this, tr("Ошибка открытия"), ErrorToString(result.error()));
    return;
  }
  statusBar()->showMessage(tr("Открыто: %1").arg(path));
}

void MainWindow::OnSave() {
  const ravel::app::ControllerResult result = controller_.Save();
  if (!result.has_value()) {
    // NoPath: Ctrl+S -> SaveAs.
    if (result.error() == ravel::app::ControllerError::NoPath) {
      OnSaveAs();
      return;
    }
    QMessageBox::warning(this, tr("Ошибка сохранения"), ErrorToString(result.error()));
    return;
  }
  statusBar()->showMessage(tr("Сохранено"));
}

void MainWindow::OnSaveAs() {
  const QString path = QFileDialog::getSaveFileName(this, tr("Сохранить файл"), QString(),
                                                    tr("Ravel (*.ravel.json)"));
  if (path.isEmpty()) {
    return;  // user close the dialog
  }
  const ravel::app::ControllerResult result =
      controller_.SaveAs(std::filesystem::path(path.toStdWString()));
  if (!result.has_value()) {
    QMessageBox::warning(this, tr("Ошибка сохранения"), ErrorToString(result.error()));
    return;
  }
  statusBar()->showMessage(tr("Сохранено: %1").arg(path));
}

}  // namespace ravel::ui
