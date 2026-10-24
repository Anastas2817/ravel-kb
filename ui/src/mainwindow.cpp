#include "mainwindow.hpp"

#include "node_item.hpp"
#include "relation_item.hpp"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QHash>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QStringList>
#include <QVBoxLayout>

namespace ravel::ui {

namespace {
// Border-to-border line between two nodes.
QLineF RelationLine(const NodeItem* from, const NodeItem* to) {
  return QLineF(from->AnchorTowards(to->pos()), to->AnchorTowards(from->pos()));
}
}  // namespace

AddNodeDialog::AddNodeDialog(const QStringList& types, QWidget* parent) : QDialog(parent) {
  auto* layout = new QVBoxLayout(this);
  type_ = new QComboBox(this);
  type_->addItems(types);
  title_ = new QLineEdit(this);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  layout->addWidget(type_);
  layout->addWidget(title_);
  layout->addWidget(buttons);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString AddNodeDialog::SelectedType() const {
  return type_->currentText();
}

QString AddNodeDialog::NodeTitle() const {
  return title_->text();
}

AddRelationDialog::AddRelationDialog(const QStringList& types, QWidget* parent) : QDialog(parent) {
  auto* layout = new QVBoxLayout(this);
  type_ = new QComboBox(this);
  type_->addItems(types);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  layout->addWidget(type_);
  layout->addWidget(buttons);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString AddRelationDialog::SelectedType() const {
  return type_->currentText();
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      scene_(new QGraphicsScene(this)),
      view_(new CanvasView(this)),
      open_action_(new QAction(tr("Открыть…"), this)),
      save_action_(new QAction(tr("Сохранить"), this)),
      save_as_action_(new QAction(tr("Сохранить как…"), this)),
      undo_action_(new QAction(tr("Отменить"), this)),
      redo_action_(new QAction(tr("Восстановить"), this)) {
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

  connect(view_, &CanvasView::DoubleClicked, this, &MainWindow::OnSceneDoubleClick);

  QMenu* file_menu = menuBar()->addMenu(tr("&Файл"));
  file_menu->addAction(open_action_);
  file_menu->addAction(save_action_);
  file_menu->addAction(save_as_action_);
  QMenu* edit = menuBar()->addMenu(tr("&Правка"));
  edit->addAction(undo_action_);
  edit->addAction(redo_action_);

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

  // NodeItem rect is center-origin: setPos() places the model position at the
  // visual center of the node.
  constexpr qreal kWidth = 120.0;
  constexpr qreal kHeight = 60.0;

  for (const ravel::app::NodeView& view : nodes) {
    NodeItem* item = items.take(static_cast<quint64>(view.id));
    if (item == nullptr) {
      const QRectF rect(-kWidth / 2, -kHeight / 2, kWidth, kHeight);
      item = new NodeItem(
          view.id, QString::fromStdString(view.title), QString::fromStdString(view.type), rect,
          QString::fromStdString(view.color), view.shape, QString::fromStdString(view.frame_color));
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
  AddNodeDialog dlg(ToQStringList(controller_.NodeTypeNames()), this);
  if (dlg.exec() != QDialog::Accepted) {
    return;
  }
  const ravel::app::CommandResult result =
      controller_.AddNode(dlg.NodeTitle().toStdString(), dlg.SelectedType().toStdString(), "", 1,
                          ravel::Position{pos.x(), pos.y()});
  if (!result.has_value()) {
    QMessageBox::warning(this, tr("Ошибка создания"),
                         tr("Не удалось создать узел: неизвестный тип (онтология пуста?)."));
    return;
  }
  ResyncScene();
}

void MainWindow::OnAddRelation(std::uint64_t from, std::uint64_t to) {
  AddRelationDialog dlg(ToQStringList(controller_.RelationTypeNames()), this);
  if (dlg.exec() != QDialog::Accepted) {
    return;
  }
  const ravel::app::CommandResult result =
      controller_.AddRelation(from, to, dlg.SelectedType().toStdString(), "");
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

}  // namespace ravel::ui
