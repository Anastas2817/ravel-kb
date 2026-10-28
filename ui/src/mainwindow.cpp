#include "mainwindow.hpp"

#include "add_node_dialog.hpp"
#include "add_relation_dialog.hpp"
#include "node_item.hpp"
#include "open_node_dialog.hpp"
#include "open_relation_dialog.hpp"
#include "relation_item.hpp"

#include <QFileDialog>
#include <QHash>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QStringList>
#include <optional>

namespace ravel::ui {

namespace {
// Border-to-border line between two nodes.
QLineF RelationLine(const NodeItem* from, const NodeItem* to) {
  return QLineF(from->AnchorTowards(to->pos()), to->AnchorTowards(from->pos()));
}
}  // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      scene_(new QGraphicsScene(this)),
      view_(new CanvasView(this)),
      open_action_(new QAction(tr("Открыть…"), this)),
      save_action_(new QAction(tr("Сохранить"), this)),
      save_as_action_(new QAction(tr("Сохранить как…"), this)),
      undo_action_(new QAction(tr("Отменить"), this)),
      redo_action_(new QAction(tr("Восстановить"), this)),
      delete_action_(new QAction(tr("Удалить"), this)) {
  setCentralWidget(view_);
  view_->setScene(scene_);
  // Full redraw instead of the default MinimalViewportUpdate: with region
  // updates the scene left smear trails behind dragged relations (the root
  // cause sits in Qt's deferred dirty-item processing; not worth chasing for
  // this scale). At 10^2-10^3 items a full viewport repaint is unnoticeable.
  view_->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

  open_action_->setShortcut(QKeySequence::Open);  // Ctrl+O
  connect(open_action_, &QAction::triggered, this, &MainWindow::OnOpen);

  save_action_->setShortcut(QKeySequence::Save);  // Ctrl+S
  connect(save_action_, &QAction::triggered, this, &MainWindow::OnSave);

  save_as_action_->setShortcut(QKeySequence::SaveAs);  // Ctrl+Shift+S
  connect(save_as_action_, &QAction::triggered, this, &MainWindow::OnSaveAs);

  undo_action_->setShortcut(QKeySequence::Undo);  // Ctrl+Z
  connect(undo_action_, &QAction::triggered, this, &MainWindow::OnUndo);

  redo_action_->setShortcut(QKeySequence::Redo);  // Ctrl+Y on Windows, Ctrl+Shift+Z on macOS/Linux
  connect(redo_action_, &QAction::triggered, this, &MainWindow::OnRedo);

  delete_action_->setShortcuts({QKeySequence("Ctrl+D"), QKeySequence::Delete});
  connect(delete_action_, &QAction::triggered, this, &MainWindow::OnDeleteSelected);

  connect(view_, &CanvasView::DoubleClicked, this, &MainWindow::OnSceneDoubleClick);

  QMenu* file_menu = menuBar()->addMenu(tr("&Файл"));
  file_menu->addAction(open_action_);
  file_menu->addAction(save_action_);
  file_menu->addAction(save_as_action_);
  QMenu* edit = menuBar()->addMenu(tr("&Правка"));
  edit->addAction(undo_action_);
  edit->addAction(redo_action_);
  edit->addAction(delete_action_);

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
  ResyncScene();
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

void MainWindow::ResyncScene() {
  const std::vector<ravel::app::NodeView> nodes = controller_.Nodes();
  const std::vector<ravel::app::RelationView> relations = controller_.Relations();

  // Index current scene items by model id.
  QHash<quint64, NodeItem*> items;
  const QList<QGraphicsItem*> scene_items = scene_->items();
  for (QGraphicsItem* item : scene_items) {
    if (auto* node = dynamic_cast<NodeItem*>(item)) {
      items.insert(node->id(), node);
    }
  }

  for (const ravel::app::NodeView& view : nodes) {
    // NodeItem rect is center-origin: setPos() places the model position at the
    // visual center of the node.
    const qreal kWidth = 140.0 * view.size;
    const qreal kHeight = 60.0 * view.size;
    const QRectF rect(-kWidth / 2, -kHeight / 2, kWidth, kHeight);
    NodeItem* item = items.take(static_cast<quint64>(view.id));
    if (item == nullptr) {
      item = new NodeItem(view.id, QString::fromStdString(view.title),
                          QString::fromStdString(view.type), view.size, rect,
                          QString::fromStdString(view.color), view.shape,
                          QString::fromStdString(view.frame_color));
      scene_->addItem(item);
      connect(item, &NodeItem::MoveFinished, this, &MainWindow::OnNodeMoved);
      connect(item, &NodeItem::RelationFinished, this, &MainWindow::OnAddRelation);
      // Per-step geometry broadcast: RelationItems follow the dragged node.
      connect(item, &NodeItem::Moved, this, &MainWindow::OnNodeDragged);
    }
    if (view.position.has_value()) {
      const QPointF target(view.position->x_, view.position->y_);
      if (item->pos() != target) {
        item->setPos(target);
      }
    }
    if (item->boundingRect() != rect) {
      item->SetRect(rect, view.size);
    }
    if (item->title() != view.title || item->GetType() != view.type) {
      item->Change(QString::fromStdString(view.title), QString::fromStdString(view.type),
                   QString::fromStdString(view.color), view.shape,
                   QString::fromStdString(view.frame_color));
    }
  }

  // Whatever is left in `items` no longer exists in the model. removeItem()
  // passes ownership to the caller, so we delete explicitly (no QObject
  // parent-child for scene items).
  for (NodeItem* item : items) {
    scene_->removeItem(item);
    delete item;
  }

  // Reconcile relation items by model id (same pattern as nodes). Geometry is
  // refreshed on every resync; live following during drag is driven by
  // NodeItem::Moved (OnNodeDragged).
  QHash<quint64, RelationItem*> rel_items;
  for (QGraphicsItem* item : scene_->items()) {
    if (auto* rel = dynamic_cast<RelationItem*>(item)) {
      rel_items.insert(rel->id(), rel);
    }
  }
  for (const ravel::app::RelationView& view : relations) {
    RelationItem* rel = rel_items.take(view.id);
    if (rel == nullptr) {
      rel = new RelationItem(view.id, view.from, view.to, QString::fromStdString(view.color),
                             view.arrow, view.symmetric);
      scene_->addItem(rel);
    }
    const NodeItem* from_item = FindNode(view.from);
    const NodeItem* to_item = FindNode(view.to);
    if (from_item != nullptr && to_item != nullptr) {
      rel->setLine(RelationLine(from_item, to_item));
    }
    if (rel->GetType() != view.type) {
      rel->Change(QString::fromStdString(view.type),
                   QString::fromStdString(view.color), view.arrow,
                   view.symmetric);
    }
  }
  for (RelationItem* rel : rel_items) {
    scene_->removeItem(rel);
    delete rel;
  }
}

QStringList ToQStringList(const std::vector<std::string>& v) {
  QStringList out;
  out.reserve(static_cast<qsizetype>(v.size()));
  for (const auto& s : v) {
    out << QString::fromStdString(s);
  }
  return out;
}

void MainWindow::OnSceneDoubleClick(QPointF pos) {
  RelationItem* rel = nullptr;
  for (QGraphicsItem* item : scene_->items(pos)) {
    if (auto* node = dynamic_cast<NodeItem*>(item)) {
      std::optional<ravel::app::NodeView> cur = controller_.FindNode(node->id());
      if (!cur) {
        return;
      }
      OpenNodeDialog dlg(cur.value(), ToQStringList(controller_.NodeTypeNames()), this);
      dlg.setWindowOpacity(0.9);
      dlg.setWindowTitle(tr("Редактирование понятия"));
      if (dlg.exec() != QDialog::Accepted) {
        return;
      }
      const ravel::app::CommandResult result =
          controller_.ChangeNode(node->id(), dlg.title().toStdString(), dlg.type().toStdString(),
                                 dlg.description().toStdString(), dlg.size());
      if (!result.has_value()) {
        QMessageBox::warning(this, tr("Ошибка редактирования"),
                             tr("Не удалось изменить узел: неизвестный тип (онтология пуста?)."));
        return;
      }
      ResyncScene();
      return;
    } else if (auto* cur = dynamic_cast<RelationItem*>(item)) {
      rel = cur;
    }
  }

  if (rel != nullptr) {
    std::optional<ravel::app::RelationView> cur = controller_.FindRelation(rel->id());
    if (!cur) {
      return;
    }
    std::optional<ravel::app::NodeView> from = controller_.FindNode(cur.value().from);
    std::optional<ravel::app::NodeView> to = controller_.FindNode(cur.value().to);
    if (!from || !to) {
      return;
    }
    OpenRelationDialog dlg(cur.value(), ToQStringList(controller_.RelationTypeNames()),
                           QString::fromStdString(from.value().title),
                           QString::fromStdString(to.value().title), this);
    dlg.setWindowOpacity(0.9);
    dlg.setWindowTitle(tr("Редактирование связи"));
    if (dlg.exec() != QDialog::Accepted) {
      return;
    }
    const ravel::app::CommandResult result = controller_.ChangeRelation(
        rel->id(), dlg.type().toStdString(), dlg.description().toStdString());
    if (!result.has_value()) {
      QMessageBox::warning(this, tr("Ошибка редактирования"),
                           tr("Не удалось изменить связь: неизвестный тип (онтология пуста?)."));
      return;
    }
  } else {
    AddNodeDialog dlg(ToQStringList(controller_.NodeTypeNames()), this);
    dlg.setWindowOpacity(0.9);
    dlg.setWindowTitle(tr("Новое понятие"));
    if (dlg.exec() != QDialog::Accepted) {
      return;
    }
    const ravel::app::CommandResult result = controller_.AddNode(
        dlg.title().toStdString(), dlg.type().toStdString(), dlg.description().toStdString(),
        dlg.size(), ravel::Position{pos.x(), pos.y()});
    if (!result.has_value()) {
      QMessageBox::warning(this, tr("Ошибка создания"),
                           tr("Не удалось создать узел: неизвестный тип (онтология пуста?)."));
      return;
    }
  }

  ResyncScene();
}

void MainWindow::OnAddRelation(std::uint64_t from, std::uint64_t to, const QString& from_title,
                               const QString& to_title) {
  AddRelationDialog dlg(ToQStringList(controller_.RelationTypeNames()), from_title, to_title, this);
  dlg.setWindowOpacity(0.9);
  dlg.setWindowTitle(tr("Новая связь"));
  if (dlg.exec() != QDialog::Accepted) {
    return;
  }
  const ravel::app::CommandResult result =
      controller_.AddRelation(from, to, dlg.type().toStdString(), dlg.description().toStdString());
  if (!result.has_value()) {
    QMessageBox::warning(this, tr("Ошибка создания"),
                         tr("Не удалось создать отношение: неизвестный тип (онтология пуста?)."));
    return;
  }
  ResyncScene();
}

void MainWindow::OnUndo() {
  const ravel::app::CommandResult result = controller_.Undo();
  if (!result.has_value()) {
    QMessageBox::warning(this, tr("Ошибка отмены"), tr("нечего отменять?"));
    return;
  }
  ResyncScene();
}

void MainWindow::OnRedo() {
  const ravel::app::CommandResult result = controller_.Redo();
  if (!result.has_value()) {
    QMessageBox::warning(this, tr("Ошибка отмены"), tr("нечего восстанавливать?"));
    return;
  }
  ResyncScene();
}

void MainWindow::OnNodeMoved(std::uint64_t id, QPointF to) {
  const ravel::app::CommandResult result =
      controller_.MoveNode(id, ravel::Position{to.x(), to.y()});
  if (!result.has_value()) {
    QMessageBox::warning(this, tr("Ошибка перемещения"), tr("не отменить операцию"));
    return;
  }
  ResyncScene();
}

NodeItem* MainWindow::FindNode(std::uint64_t id) const {
  for (QGraphicsItem* item : scene_->items()) {
    if (auto* node = dynamic_cast<NodeItem*>(item)) {
      if (node->id() == id) {
        return node;
      }
    }
  }
  return nullptr;
}

void MainWindow::OnNodeDragged(std::uint64_t id, const QPointF& /*new_pos*/) {
  // new_pos equals the node's pos(): Moved is emitted from ItemPositionHasChanged,
  // after the position is committed.
  for (QGraphicsItem* item : scene_->items()) {
    auto* rel = dynamic_cast<RelationItem*>(item);
    if (rel == nullptr || (rel->from() != id && rel->to() != id)) {
      continue;
    }
    const NodeItem* from_item = FindNode(rel->from());
    const NodeItem* to_item = FindNode(rel->to());
    if (from_item == nullptr || to_item == nullptr) {
      continue;
    }
    rel->setLine(RelationLine(from_item, to_item));
  }
}

void MainWindow::OnDeleteSelected() {
  std::vector<ravel::NodeId> node_ids;
  std::vector<ravel::RelationId> rel_ids;
  for (QGraphicsItem* item : scene_->selectedItems()) {
    if (auto* node = dynamic_cast<NodeItem*>(item)) {
      node_ids.push_back(node->id());
    } else if (auto* rel = dynamic_cast<RelationItem*>(item)) {
      rel_ids.push_back(rel->id());
    }
  }
  if (node_ids.empty() && rel_ids.empty()) {
    return;
  }

  const ravel::app::CommandResult r = controller_.Delete(node_ids, rel_ids);
  if (!r.has_value()) {
    QMessageBox::warning(this, tr("Ошибка удаления"), tr("не удалось удалить"));
    return;
  }

  ResyncScene();
}

}  // namespace ravel::ui
